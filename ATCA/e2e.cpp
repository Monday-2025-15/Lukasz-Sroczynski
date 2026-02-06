#include <sodium.h>
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

string generateKey() {
    unsigned char key[crypto_secretbox_KEYBYTES];
    randombytes_buf(key, sizeof key);

    char hex[crypto_secretbox_KEYBYTES * 2 + 1];
    sodium_bin2hex(hex, sizeof hex, key, sizeof key);

    return string(hex);
}

vector<unsigned char> hexToBin(const string& hex) {
    vector<unsigned char> bin(hex.size() / 2);
    sodium_hex2bin(
        bin.data(),
        bin.size(),
        hex.c_str(),
        hex.size(),
        nullptr,
        nullptr,
        nullptr
    );

    return bin;
}

void encryptAppendLine(const string& time, const string& plaintext, const string& keyHex) {
    string filename = "msgs.txt";
    auto key = hexToBin(keyHex);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    vector<unsigned char> cipher(crypto_secretbox_MACBYTES + plaintext.size());

    crypto_secretbox_easy(
        cipher.data(),
        (const unsigned char*)plaintext.data(),
        plaintext.size(),
        nonce,
        key.data()
    );

    char nonceHex[crypto_secretbox_NONCEBYTES * 2 + 1];
    char cipherHex[cipher.size() * 2 + 1];

    sodium_bin2hex(nonceHex, sizeof nonceHex, nonce, sizeof nonce);
    sodium_bin2hex(cipherHex, sizeof cipherHex, cipher.data(), cipher.size());

    ofstream out(filename, ios::app);
    out << time << nonceHex << ":" << cipherHex << "\n";
    out.close();
}

string decryptFile(const string& date, const string& time, const string& keyHex) {
    string search = date + " " + time;

    string filename = "msgs.txt";
    ifstream in(filename);
    string line, delimiter, result;

    int i = 0;
    while (true) {
        if (!getline(in, line))
            break;

        if (line.find(search) != string::npos) {
            result = line.substr(19);
            break;
        } else {
            i++;
        }
    }

    auto sep = result.find(':');

    auto nonce = hexToBin(result.substr(0, sep));
    auto cipher = hexToBin(result.substr(sep + 1));

    auto key = hexToBin(keyHex);

    vector<unsigned char> plain(cipher.size() - crypto_secretbox_MACBYTES);

    if (crypto_secretbox_open_easy(
            plain.data(),
            cipher.data(),
            cipher.size(),
            nonce.data(),
            key.data()) != 0)

    in.close();

    return string(plain.begin(), plain.end());
}

string toBase64(const string& bin) {
    size_t b64_len =
    sodium_base64_ENCODED_LEN(
        bin.size(),
        sodium_base64_VARIANT_ORIGINAL
    );

    string out(b64_len, '\0');

    sodium_bin2base64(
        out.data(),
        out.size(),
        (const unsigned char*)bin.data(),
        bin.size(),
        sodium_base64_VARIANT_ORIGINAL
    );

    out.pop_back();
    return out;
}

string fromBase64(const string& base64) {
    vector<unsigned char> buffer(base64.size() * 3 / 4 + 1);
    size_t bin_len = 0;

    sodium_base642bin(
        buffer.data(),
        buffer.size(),
        base64.c_str(),
        base64.size(),
        nullptr, 
        &bin_len,
        nullptr,
        sodium_base64_VARIANT_ORIGINAL
    );

    return string((char*)buffer.data(), bin_len);
}

string encodeMessage(const unsigned char* my_sk, const unsigned char* peer_pk, const string& message) {
    unsigned char shared[crypto_scalarmult_BYTES];
    if (crypto_scalarmult(shared, my_sk, peer_pk) != 0) {
        return "";
    }

    unsigned char key[crypto_aead_chacha20poly1305_IETF_KEYBYTES];
    crypto_generichash(key, sizeof key, shared, sizeof shared, nullptr, 0);

    unsigned char nonce[crypto_aead_chacha20poly1305_IETF_NPUBBYTES];
    randombytes_buf(nonce, sizeof nonce);

    vector<unsigned char> ciphertext(message.size() + crypto_aead_chacha20poly1305_IETF_ABYTES);
    unsigned long long cipher_len;

    crypto_aead_chacha20poly1305_ietf_encrypt(
        ciphertext.data(),
        &cipher_len,
        (const unsigned char*)message.data(),
        message.size(),
        nullptr,
        0,
        nullptr,
        nonce,
        key
    );

    string out;
    out.reserve(sizeof nonce + cipher_len);

    out.append((char*)nonce, sizeof nonce);
    out.append((char*)ciphertext.data(), cipher_len);

    return out;
}

string decodeMessage(const unsigned char* my_sk, const unsigned char* peer_pk, const string& packet) {
    if (packet.size() < crypto_aead_chacha20poly1305_IETF_NPUBBYTES) {
        return "";
    }

    const unsigned char* nonce = (const unsigned char*)packet.data();

    const unsigned char* ciphertext = (const unsigned char*)packet.data() + crypto_aead_chacha20poly1305_IETF_NPUBBYTES;

    size_t ciphertext_len = packet.size() - crypto_aead_chacha20poly1305_IETF_NPUBBYTES;

    unsigned char shared[crypto_scalarmult_BYTES];
    if (crypto_scalarmult(shared, my_sk, peer_pk) != 0) {
        return "";
    }


    unsigned char key[crypto_aead_chacha20poly1305_IETF_KEYBYTES];
    crypto_generichash(key, sizeof key, shared, sizeof shared, nullptr, 0);

    vector<unsigned char> decrypted(ciphertext_len);
    unsigned long long plain_len;

    if (crypto_aead_chacha20poly1305_ietf_decrypt(
            decrypted.data(),
            &plain_len,
            nullptr,
            ciphertext,
            ciphertext_len,
            nullptr,
            0,
            nonce,
            key
        ) != 0) {
        return "";
    }

    return string((char*)decrypted.data(), plain_len);
}

// int main() {
//     if (sodium_init() < 0) {
//         cout << "libsodium init failed\n";
//         return 1;
//     }

//     unsigned char alice_pk[crypto_box_PUBLICKEYBYTES];
//     unsigned char alice_sk[crypto_box_SECRETKEYBYTES];
//     unsigned char bob_pk[crypto_box_PUBLICKEYBYTES];
//     unsigned char bob_sk[crypto_box_SECRETKEYBYTES];

//     crypto_box_keypair(alice_pk, alice_sk);
//     crypto_box_keypair(bob_pk, bob_sk);

//     string packet = encodeMessage(alice_sk, bob_pk, "Hello Bob! To jest E2E.");

//     string msg = decodeMessage(bob_sk, alice_pk, packet);

//     cout << "Bob odebral: " << msg << endl;
//     cout << "Dlugosc: " << msg.size() << endl;

//     return 0;
// }
