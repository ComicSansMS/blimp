
#include <aws/core/Aws.h>
#include <aws/glacier/GlacierClient.h>

#include <sqlite3.h>

#include <gsl.h>

#include <rijndael.h>
#include <aes.h>

int main()
{
    CryptoPP::Rijndael rijn;
    CryptoPP::AESEncryption aes_encrypt;
    byte key[] = { 'C', 'O', 'O', 'L', 'K', 'E', 'Y', '!' };
    aes_encrypt.SetKey(key, sizeof(key));
    byte b[CryptoPP::AESEncryption::BLOCKSIZE] = "BLA";
    aes_encrypt.ProcessBlock(b);

    Aws::SDKOptions opt;
    Aws::InitAPI(opt);

    sqlite3_initialize();
}
