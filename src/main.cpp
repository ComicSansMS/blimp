
#include <aws/core/Aws.h>
#include <aws/glacier/GlacierClient.h>

#include <sqlite3.h>

#include <gsl.h>

#include <rijndael.h>
#include <aes.h>

#include <QApplication>

#include <ui/main_window.hpp>

int main(int argc, char* argv[])
{
    CryptoPP::AESEncryption aes_encrypt;
    byte key[] = { 'C', 'O', 'O', 'L', 'K', 'E', 'Y', '!' };
    aes_encrypt.SetKey(key, sizeof(key));
    byte b[CryptoPP::AESEncryption::BLOCKSIZE] = "BLA";
    aes_encrypt.ProcessBlock(b);

    Aws::SDKOptions opt;
    Aws::InitAPI(opt);

    sqlite3_initialize();

    QApplication the_app(argc, argv);

    //QMainWindow main_window;
    //main_window.show();
    //QTreeView view(&main_window);
    /*
    QTreeWidget widg;
    widg.show();

    auto fs_model = new QFileSystemModel();
    fs_model->setRootPath("D:");
    widg.setModel(fs_model);
    */

    MainWindow main_window;

    return the_app.exec();
}
