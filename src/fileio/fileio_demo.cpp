#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <chrono>
#include <iostream>
#include <vector>

int main()
{
    HANDLE hfin = CreateFile("moby.txt", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED, nullptr);
    if (hfin == INVALID_HANDLE_VALUE) {
        std::cout << "Error opening file: " << GetLastError() << "\n";
        return 1;
    }
    DWORD const BUFFER_SIZE = 1024 * 1024;
    std::vector<char> buffer(BUFFER_SIZE);
    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!hEvent) {
        std::cout << "Error creating event: " << GetLastError() << "\n";
        return 1;
    }
    auto const set_offset = [](std::uint64_t offset, OVERLAPPED& ov) {
        std::memcpy(&ov.Offset, &offset, sizeof(DWORD));
        std::memcpy(&ov.OffsetHigh, &reinterpret_cast<DWORD const*>(&offset)[1], sizeof(DWORD));
    };
    OVERLAPPED overlapped;
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = hEvent;
    set_offset(0, overlapped);
    auto const t0 = std::chrono::steady_clock::now();
    BOOL res = ReadFileEx(hfin, buffer.data(), BUFFER_SIZE, &overlapped, [](DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
            std::cout << "Complete " << dwErrorCode << ", read " << dwNumberOfBytesTransfered << " bytes.\n";
        });
    if (!res) {
        std::cout << "Error establishing file read: " << GetLastError() << "\n";
        return 1;
    }
    auto const t1 = std::chrono::steady_clock::now();
    std::cout << "Waiting...\n";
    auto const t2 = std::chrono::steady_clock::now();
    WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
    auto const t3 = std::chrono::steady_clock::now();
    std::cout << "Done.\n";
    std::cout << "Wait for async return: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "ms.\n";
    std::cout << "Wait for async complete: " << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t0).count() << "ms.\n";
}
