#include <aes.h>
#include <filters.h>
#include <hex.h>
#include <modes.h>
#include <osrng.h>
#include <pwdbased.h>
#include <sha.h>

#include <iostream>

char const secret[] = R"(I met a traveller from an antique land
Who said: "Two vast and trunkless legs of stone
Stand in the desert. Near them, on the sand,
Half sunk, a shattered visage lies, whose frown,
And wrinkled lip, and sneer of cold command,
Tell that its sculptor well those passions read
Which yet survive, stamped on these lifeless things,
The hand that mocked them and the heart that fed:
And on the pedestal these words appear:
'My name is Ozymandias, king of kings:
Look on my works, ye Mighty, and despair!'
Nothing beside remains. Round the decay
Of that colossal wreck, boundless and bare
The lone and level sands stretch far away." )";

int main()
{
    CryptoPP::AutoSeededRandomPool pool;
    std::cout << "Random " << pool.GenerateWord32() << "\n";

    CryptoPP::PKCS12_PBKDF<CryptoPP::SHA256> pbkdf;
    auto const key_length = pbkdf.GetValidDerivedLength(CryptoPP::AES::MAX_KEYLENGTH);

    std::vector<CryptoPP::byte> key;
    key.resize(key_length);

    std::cout << pbkdf.AlgorithmName() << "\n";
    std::string password = "batteryhorsestaples";
    pbkdf.DeriveKey(key.data(), key.size(), reinterpret_cast<CryptoPP::byte const*>(password.data()), password.length());

    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption cbc_enc;
    std::vector<CryptoPP::byte> enc_iv;
    enc_iv.resize(cbc_enc.IVSize());
    pool.GenerateBlock(enc_iv.data(), enc_iv.size());
    cbc_enc.SetKeyWithIV(key.data(), key.size(), enc_iv.data());

    std::vector<CryptoPP::byte> plaintext;
    std::size_t const plaintext_buffer_size = sizeof(secret) + CryptoPP::AES::BLOCKSIZE - (sizeof(secret) % CryptoPP::AES::BLOCKSIZE);
    plaintext.resize(plaintext_buffer_size);
    std::copy(std::begin(secret), std::end(secret), begin(plaintext));

    std::vector<CryptoPP::byte> ciphertext;
    ciphertext.resize(plaintext.size());
    cbc_enc.ProcessData(ciphertext.data(), plaintext.data(), plaintext.size());

    std::vector<CryptoPP::byte> decoded;
    decoded.resize(ciphertext.size());
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption cbc_dec;
    cbc_dec.SetKeyWithIV(key.data(), key.size(), enc_iv.data());
    cbc_dec.ProcessData(decoded.data(), ciphertext.data(), ciphertext.size());

    std::cout << "Generated key \n";
}

