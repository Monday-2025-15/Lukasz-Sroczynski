#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <sodium.h>

using namespace std;


string verification_code(int length) {
    const string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    string random_string;
    for (int i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

bool timer(int minutes) {
    int seconds = minutes * 60;

    while (seconds >= 0) {
        this_thread::sleep_for(chrono::seconds(1));
        seconds--;
    }
    
    return false;
}

string loadFileToBuffer(const string& path)
{
    ifstream file(path, ios::binary | ios::ate);
    if (!file)
        throw runtime_error("Cannot open file: " + path);

    streamsize size = file.tellg();
    if (size < 0)
        throw runtime_error("tellg() failed");

    file.seekg(0, ios::beg);

    string buffer(static_cast<size_t>(size), '\0');

    if (!file.read(buffer.data(), size))
        throw runtime_error("File read failed");

    return buffer;
}


void saveBufferToFile(const string& data, const string& path)
{
    vector<uint8_t> buffer(reinterpret_cast<const uint8_t*>(data.data()), reinterpret_cast<const uint8_t*>(data.data() + data.size()));  
    ofstream file(path, ios::binary);
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

bool sendFile(int sock, const string& path)
{
    string data = loadFileToBuffer(path);

    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(sock, data.data() + sent, data.size() - sent, 0);
        if (n <= 0)
            return false;
        sent += n;
    }

    return true;
}

bool receiveFile(int sock, const string& path, uint64_t size) {
    string buffer(size, '\0');

    size_t received = 0;
    while (received < size) {
        ssize_t n = recv(sock, buffer.data() + received, size - received,0);
        if (n <= 0) {
            return false;
        }

        received += n;
    }

    saveBufferToFile(buffer, path);
    return true;
}

// int main() {
//     //cout << verification_code(5) << endl;
//     string buff = loadFileToBuffer("input.jpg");
//     saveBufferToFile(buff, "output.jpg");

//     return 0;
// }