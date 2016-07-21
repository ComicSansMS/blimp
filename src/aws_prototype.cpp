#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/crypto/Sha256.h>
#include <aws/core/utils/base64/Base64.h>
#include <aws/glacier/GlacierClient.h>

#include <aws/glacier/model/DescribeVaultRequest.h>
#include <aws/glacier/model/UploadArchiveRequest.h>
#include <aws/glacier/model/UploadArchiveResult.h>

#include <hex.h>
#include <sha.h>


#include <array>
#include <fstream>
#include <iostream>

std::string calculateHash(unsigned char const* data, std::size_t data_size)
{
    CryptoPP::SHA256 hash_calc;
    hash_calc.Restart();
    hash_calc.Update(data, data_size);
    std::array<std::uint8_t, 32> digest;
    hash_calc.Final(reinterpret_cast<byte*>(digest.data()));
    
    CryptoPP::HexEncoder enc;
    enc.Put(reinterpret_cast<byte const*>(digest.data()), digest.size());
    std::array<char, 2*std::tuple_size<decltype(digest)>::value + 1> buffer;
    auto const res = enc.Get(reinterpret_cast<byte*>(buffer.data()), 2*digest.size());
    buffer.back() = '\0';
    return std::string(begin(buffer), end(buffer));
}

std::string calculateTreeHash(unsigned char const* data, std::size_t data_size)
{
    std::size_t CHUNK_SIZE = 1024*1024;
    std::vector<std::string> hashes;
    for(std::size_t data_offset = 0; data_offset < data_size; data_offset += CHUNK_SIZE) {
        hashes.push_back(calculateHash(data + data_offset, std::min(data_size - data_offset, CHUNK_SIZE)));
    }

    // collapse tree
    while(hashes.size() > 1) {
        std::vector<std::string> new_hashes;
        for(std::size_t i = 0; i < hashes.size() / 2; ++i) {
            std::array<std::uint8_t, 64> double_digest;
            CryptoPP::HexDecoder dec;
            dec.Put(reinterpret_cast<unsigned char const*>(hashes[i*2].data()), hashes[i*2].size());
            dec.Get(double_digest.data(), 32);
            dec.Put(reinterpret_cast<unsigned char const*>(hashes[i*2 + 1].data()), hashes[i*2 + 1].size());
            dec.Get(double_digest.data() + 32, 32);
            new_hashes.push_back(calculateHash(double_digest.data(), double_digest.size()));
            /*
            std::string concat = hashes[i*2] + hashes[i*2 + 1];
            new_hashes.push_back(calculateHash(reinterpret_cast<unsigned char const*>(concat.data()), concat.size()));
            */
        }
        hashes.swap(new_hashes);
    }

    return hashes.front();
}

int main()
{
    Aws::SDKOptions options;
    
    Aws::InitAPI(options);

    Aws::Client::ClientConfiguration client_cfg;
    client_cfg.region = Aws::Region::EU_CENTRAL_1;
    Aws::Auth::EnvironmentAWSCredentialsProvider cred_provider;
    Aws::Auth::AWSCredentials creds = cred_provider.GetAWSCredentials();
    Aws::Glacier::GlacierClient client(client_cfg);
    Aws::Glacier::Model::DescribeVaultRequest request;
    request.WithVaultName("TestVault").WithAccountId("-");
    //*
    Aws::Glacier::Model::DescribeVaultOutcome outcome = client.DescribeVault(request);
    if(!outcome.IsSuccess()) {
        std::cout << ":(" << std::endl;
        std::cout << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << ":)" << std::endl;
        auto const& res = outcome.GetResult();
        std::cout << "Vault is " << res.GetVaultARN() << " '" << res.GetVaultName() << "'\n"
                  << "    created at " << res.GetCreationDate() << "\n"
                  << "    last inventory at " << res.GetLastInventoryDate() << "\n"
                  << "    " << res.GetNumberOfArchives() << " archives with a total of "
                            << res.GetSizeInBytes() << " bytes."
                  << std::endl;

    }
    //*/


    std::vector<char> filedata;
    {
        std::ifstream fin("undertow.mp3", std::ios_base::binary);
        if(!fin) { abort(); }
        fin.seekg(0, std::ios_base::end);
        auto const fsize = fin.tellg();
        filedata.resize(fsize);
        fin.seekg(0, std::ios_base::beg);
        fin.read(filedata.data(), fsize);
    }

    std::cout << "Read " << filedata.size() << " bytes from file." << std::endl;
    std::cout << "Checksum is " << calculateHash(reinterpret_cast<unsigned char const*>(filedata.data()), filedata.size()) << std::endl;
    std::cout << "Tree Hash: " << calculateTreeHash(reinterpret_cast<unsigned char const*>(filedata.data()), filedata.size()) << std::endl;

    Aws::Glacier::Model::UploadArchiveRequest upload_request;
    upload_request.WithAccountId("-").WithArchiveDescription("In the undertow").WithVaultName("TestVault")
        .WithChecksum(calculateTreeHash(reinterpret_cast<unsigned char const*>(filedata.data()), filedata.size()))
        ;
    auto sstr = std::make_shared<Aws::StringStream>();
    sstr->write(filedata.data(), filedata.size());
    upload_request.SetBody(sstr);

    //*
    auto upload_outcome = client.UploadArchive(upload_request);
    if(!upload_outcome.IsSuccess()) {
        std::cout << ":(" << std::endl;
        std::cout << upload_outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << ":)" << std::endl;
        auto const& res = upload_outcome.GetResult();
        std::cout << "Archive id " << res.GetArchiveId() << "\n"
            << "    created at " << res.GetLocation() << "\n"
            << "    with checksum " << res.GetChecksum()
            << std::endl;

    }
    //*/

    Aws::ShutdownAPI(options);
}
