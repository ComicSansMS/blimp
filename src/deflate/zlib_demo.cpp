#include <zlib.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

enum class ZlibErrors {
    Ok              = Z_OK,
    StreamEnd       = Z_STREAM_END,
    NeedDictionary  = Z_NEED_DICT,
    Errno           = Z_ERRNO,
    StreamError     = Z_STREAM_ERROR,
    DataError       = Z_DATA_ERROR,
    MemoryError     = Z_MEM_ERROR,
    BufferError     = Z_BUF_ERROR,
    VersionError    = Z_VERSION_ERROR
};

void handleError_deflateInit(int res)
{
    std::cout << "Error in deflateInit(): ";
    if (res == Z_STREAM_ERROR) {
        std::cout << "Invalid compression level.";
    } else if (res == Z_MEM_ERROR) {
        std::cout << "Out of memory.";
    } else if (res == Z_VERSION_ERROR) {
        std::cout << "Version mismatch. Expected version " ZLIB_VERSION ".";
    }
}

void handleError_deflateEnd(int res)
{
    std::cout << "Error in deflateEnd(): ";
    if (res == Z_STREAM_ERROR) {
        std::cout << "Inconsistent stream state.";
    } else if (res == Z_DATA_ERROR) {
        std::cout << "Stream was freed prematurely.";
    }
}

std::vector<char> readInputData(char const* filename)
{
    std::fstream fin(filename);
    fin.seekg(0, std::ios_base::end);
    auto const filesize = fin.tellg();
    fin.seekg(0, std::ios_base::beg);
    std::vector<char> data;
    data.resize(filesize);
    fin.read(data.data(), data.size());
    return data;
}


int main()
{
    z_stream zs;
    zs.opaque = nullptr;
    zs.zalloc = nullptr;
    zs.zfree = nullptr;
    int res = deflateInit(&zs, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        handleError_deflateInit(res);
        return 1;
    }

    std::vector<char> data = readInputData("moby.txt");

    zs.next_in = reinterpret_cast<Bytef*>(data.data());
    zs.avail_in = static_cast<uInt>(data.size());
    if (static_cast<std::size_t>(zs.avail_in) != data.size()) { std::cout << "Input data too big " << std::endl; return 1; }

    std::unique_ptr<char[]> out_buff(new char[4096]);

    std::deque<char> compressed_data;
    while (zs.avail_in > 0) {
        zs.avail_out = 4096;
        zs.next_out = reinterpret_cast<Bytef*>(out_buff.get());
        res = deflate(&zs, Z_NO_FLUSH);
        if (res != Z_OK) {
            std::cout << "Error deflating: " << res << ".\n";
            return 1;
        }
        compressed_data.insert(compressed_data.end(), &out_buff[0], &out_buff[4096 - zs.avail_out]);
    }
    if (zs.total_in != data.size()) { std::cout << "Lost some input data...\n"; }
    while(true) {
        zs.avail_out = 4096;
        zs.next_out = reinterpret_cast<Bytef*>(out_buff.get());
        res = deflate(&zs, Z_FINISH);
        if (res == Z_OK || res == Z_STREAM_END) {
            compressed_data.insert(compressed_data.end(), &out_buff[0], &out_buff[4096 - zs.avail_out]);
            if (res == Z_STREAM_END) { std::cout << "Finished compressing.\n"; break; }
        } else {
            std::cout << "Error finishing deflate: " << res << ".\n";
        }
    }

    if (zs.total_out != compressed_data.size()) { std::cout << "Lost some output data...\n"; }
    std::cout << "Compression factor " << ((compressed_data.size() * 100) / data.size()) << "%.\n";

    res = deflateEnd(&zs);
    if (res != Z_OK) {
        handleError_deflateEnd(res);
    }
}
