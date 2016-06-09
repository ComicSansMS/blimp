
#include <aws/core/Aws.h>
#include <aws/glacier/GlacierClient.h>

#include <sqlite3.h>

#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <gsl.h>

#include <rijndael.h>
#include <aes.h>

#pragma warning(push)
#pragma warning(disable: 4242)
#include <QApplication>

#include <ui/main_window.hpp>
#pragma warning(pop)

int main(int argc, char* argv[])
{
    Ghulbus::Log::initializeLogging();
    Ghulbus::Log::setLogHandler(Ghulbus::Log::Handlers::logToWindowsDebugger);
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);
    CryptoPP::AESEncryption aes_encrypt;
    byte key[] = { 'C', 'O', 'O', 'L', 'K', 'E', 'Y', '!' };
    aes_encrypt.SetKey(key, sizeof(key));
    byte b[CryptoPP::AESEncryption::BLOCKSIZE] = "BLA";
    aes_encrypt.ProcessBlock(b);

    Aws::SDKOptions opt;
    Aws::InitAPI(opt);

    sqlite3_initialize();

    QApplication the_app(argc, argv);

    MainWindow main_window;

    return the_app.exec();
}
