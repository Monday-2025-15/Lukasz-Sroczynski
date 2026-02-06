#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <thread>
#include <vector>
#include <atomic>
#include <sys/ioctl.h>
#include <iomanip>
#include <sodium.h>
#include <array>

#include "functions.cpp"
#include "e2e.cpp"
#include "voip.cpp"

using namespace std;

#define PORT 1110
//#define SERVER_IP "192.168.50.80" //local IP
#define SERVER_IP "92.5.80.223" //server IP
//#define SERVER_IP "192.168.50.164" //local IP

bool isLoggedIn = false;
string username, msgKey;

string currentTime() {
    time_t timestamp = time(NULL);
    struct tm datetime = *localtime(&timestamp);

    char date[20];
    strftime(date, 20, "%d:%m:%Y %H:%M:%S", &datetime);

    return date;
}


struct Message {
    string date;
    string time;
    string author;
    string type;
    string content;
    bool unread;
};

struct Conversation {
    int count = 0;
    string user;
    vector<Message> messages;
};

struct Rooms {
    int count = 0;
    vector<Conversation> conversations;
};


class Connection {
    private:
        int port;
        const char* server_ip;
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        unsigned char secretKey[crypto_box_SECRETKEYBYTES];
        unsigned char publicKey[crypto_box_PUBLICKEYBYTES];
    public:
        Connection(int p, const char* i) : port(p), server_ip(i) {
            sockaddr_in serverAddress{};
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(port);

            inet_pton(AF_INET, server_ip, &serverAddress.sin_addr);
            if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
                perror("connect");
                exit(EXIT_FAILURE);
            }


            ifstream key("ATCA.key", ios::binary);
            if (key) {
                key.read((char*)secretKey, crypto_box_SECRETKEYBYTES);
                key.close();
            }
        }

        ~Connection() {
            if (clientSocket >= 0) {
                close(clientSocket);
            }
        }

        void disconnect() {
            close(clientSocket);
        }


        string getPublicKey(string receiver) {
            string data = currentTime() + "|MSG|" + receiver;

            send(clientSocket, data.c_str(), data.size(), 0);

            char buffer[65] = {0};
            recv(clientSocket, buffer, 65, 0);

            return string(buffer);
        }
        
        bool upload(string receiver, string message) {
            string type = "MSG";

            string key = getPublicKey(receiver);

            unsigned char receiverPublicKey[crypto_box_PUBLICKEYBYTES];
            sodium_base642bin(
                receiverPublicKey,
                crypto_box_PUBLICKEYBYTES,
                key.c_str(),
                key.size(),
                nullptr,
                nullptr,
                nullptr,
                sodium_base64_VARIANT_ORIGINAL
            );

            string encodedMessage = toBase64(encodeMessage(secretKey, receiverPublicKey, message));

            string time = currentTime();
            string data = time + "|" + type + "|" + username + "|" + receiver + "|" + encodedMessage;
            

            if (message.empty() == false) {
                const char* messageC = data.c_str();

                ssize_t sent = send(clientSocket, messageC, strlen(messageC), 0);
                if (sent <= 0) {
                    return false;
                }

                char buffer[4096] = {0};
                int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
                if (bytes <= 0 || string(buffer) != data) {
                    cout << string(buffer) << endl;
                    return false;
                }

                encryptAppendLine(time, message, msgKey);

                return true;
            }
            cout << "Wystąpił błąd podczas wysyłania wiadomości." << endl;
            return false;
        }

        bool createAccount(string email, string password, string user) {
            crypto_box_keypair(publicKey, secretKey);

            ofstream out("ATCA.key", ios::binary);
            out.write((char*)secretKey, crypto_box_SECRETKEYBYTES);
            out.close();

            char publicKey64[
                sodium_base64_ENCODED_LEN(
                    crypto_box_PUBLICKEYBYTES,
                    sodium_base64_VARIANT_ORIGINAL
                )
            ];

            sodium_bin2base64(
                publicKey64,
                sizeof publicKey64,
                publicKey,
                crypto_box_PUBLICKEYBYTES,
                sodium_base64_VARIANT_ORIGINAL
            );

            string privateKeyMSG = generateKey();
            string type = "VER";
            string data = currentTime() + "|" + type + "|" + email + "|" + password + "|" + user + "|" + publicKey64 + privateKeyMSG;
            string token;

            const char* message = data.c_str();

            ssize_t sent = send(clientSocket, message, data.length(), 0);
            if (sent <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
            if (bytes <= 0 || memcmp(buffer, message, bytes) != 0) {
                cout << string(buffer) << endl;
                return false;
            }

            cout << "Wprowadź kod weryfikacyjny wysłany na podany email: ";
            getline(cin, token);

            sent = send(clientSocket, token.c_str(), token.length(), 0);
            if (sent <= 0) {
                return false;
            }
            cout << "Oczekiwanie na weryfikację..." << endl;

            memset(buffer, 0, sizeof(buffer));
            bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
            if (bytes <= 0 || string(buffer) != token) {
                cout << string(buffer) << endl;
                return false;
            }

            return true;
        }

        bool login(string password) {
            string type = "LOG";
            string data = currentTime() + "|" + type + "|" + username + "|" + password;

            const char* message = data.c_str();

            ssize_t sent = send(clientSocket, message, strlen(message), 0);
            if (sent <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
            if (bytes <= 0 || string(buffer, strlen(message)) != data) {
                cout << "\033[2J\033[H";
                cout << string(buffer) << endl;
                return false;
            }

            string result = string(buffer);
            msgKey = result.substr(strlen(message), 64);

            ifstream in("ATCA.key", ios::binary);
            if (!in) {
                cout << "Nie mozna pobrać prywatnego klucza z pliku 'ATCA.key'" << endl;
                return false;
            }

            in.read((char*)secretKey, crypto_box_SECRETKEYBYTES);
            in.close();

            return true;
        }

        bool changeStatus() {
            string type = "STS";
            string data = currentTime() + "|" + type + "|" + username;

            const char* message = data.c_str();

            ssize_t sent = send(clientSocket, message, strlen(message), 0);
            if (sent <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
            if (bytes <= 0 || string(buffer) != data) {
                cout << string(buffer) << endl;
                return false;
            }
            
            return true;
        }

        bool synchronization(string mode, Rooms& rooms) {
            string type = "SYN";
            string data = currentTime() + "|" + type + "|" + mode + "|" + username;

            const char* message = data.c_str();

            ssize_t sent = send(clientSocket, message, strlen(message), 0);
            if (sent <= 0) {
                return false;
            }

            string result, date, time, receiver, sender, content, user, contentEncrypted, publicKey64, user2, size, filename;
            int bytes,length, delimiterA, delimiterB, delimiterC, port;
            bool convExists;

            Message msg;
            Conversation conv;

            while (true) {
                char buffer[4096] = {0};
                bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);

                if (bytes > 0) {
                    result = string(buffer);
                    length = strlen(buffer);
                    if (result == "|END|" || result == "") {
                        break;
                    } else if (result.find("|") == string::npos) {
                        break;
                    } else {
                        date = string(buffer, 10);
                        time = string(buffer + 10, 8);
                        data = string(buffer + 19, length - 19);

                        delimiterA = data.find("|");
                        delimiterB = data.find("|", delimiterA + 1);
                        delimiterC = data.find("|", delimiterB + 1);

                        sender = data.substr(0, delimiterA);
                        receiver = data.substr(delimiterA + 1, (delimiterB - delimiterA - 1));
                        contentEncrypted = data.substr(delimiterB + 1, (delimiterC - delimiterB - 1));
                        publicKey64 = data.substr(delimiterC  + 1, 65);


                        if (publicKey64.substr(0, 7) == "File::/") {
                            ifstream filename(contentEncrypted);
                            if (filename.good() == false) {
                                size = publicKey64.substr(7, publicKey64.length() - 7);
                                receiveFile(clientSocket, contentEncrypted, stoull(size));
                                content = contentEncrypted;
                            }
                        } else if (publicKey64.substr(0, 7) == "Call::/") {
                            port = stoi(publicKey64.substr(7, publicKey64.length() - 7));
                            if (sender == username) {
                                runCall(SERVER_IP, port);
                            } else {
                                cout << "Połączenie głosowe od " << sender << ". Czy chcesz je odebrać (naduś 'Y' lub 'y')" << endl;
                                getline(cin, content);
                                if (content == "Y" or content == "y") {
                                    runCall(SERVER_IP, port);
                                } else {
                                    cout << "Odrzucono połączenie głosowe od " << sender << endl;
                                    string type = "ECL";
                                    string time = currentTime();
                                    string data = time + "|" + type + "|" + username + "|" + receiver + "|" + to_string(port);
                                    
                                    const char* messageC = data.c_str();

                                    if (send(clientSocket, messageC, strlen(messageC), 0) <= 0) {
                                        return false;
                                    }

                                    char buffer[4096] = {0};
                                    int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
                                    if (bytes <= 0 || string(buffer) != data) {
                                        cout << string(buffer) << endl;
                                        return false;
                                    }
                                }
                            }
                            content = "Połączenie głosowe od " + sender;

                        } else {
                            unsigned char senderPublicKey[crypto_box_PUBLICKEYBYTES];
                            sodium_base642bin(
                                senderPublicKey,
                                crypto_box_PUBLICKEYBYTES,
                                publicKey64.c_str(),
                                publicKey64.size(),
                                nullptr,
                                nullptr,
                                nullptr,
                                sodium_base64_VARIANT_ORIGINAL
                            );    

                            if (sender == username) {
                                user = receiver;
                                content = decryptFile(date, time, msgKey);
                                
                            } else if (receiver == username) {
                                user = sender;
                                content = decodeMessage(secretKey, senderPublicKey, fromBase64(contentEncrypted));
                            }

                            msg.author = sender;
                            msg.date = date;
                            msg.time = time;
                            msg.type = type;
                            msg.content = content;
                            msg.unread = true;

                            convExists = false;
                            for (int i=0; i<rooms.count; i++) {
                                if (rooms.conversations[i].user == user) {
                                    rooms.conversations[i].messages.insert(rooms.conversations[i].messages.begin() + rooms.conversations[i].count, msg);
                                    rooms.conversations[i].count++;
                                    convExists = true;
                                    continue;
                                }
                            }

                            if (convExists == false) {
                                conv.user = user;
                                conv.messages.insert(conv.messages.begin(), msg);
                                conv.count = 1;

                                rooms.conversations.insert(rooms.conversations.begin() + rooms.count, conv);
                                rooms.count++;
                            }
                        }                        
                    }
                }
            }

            return false;
        }

        bool fileUpload(string receiver, string filename) {
            string type = "FIL";

            string fileData = loadFileToBuffer(filename);
            if (fileData.empty()) {
                return false;
            }

            uint64_t size = fileData.size();
            if (size > 25000000) {
                return false;
            }

            string time = currentTime();
            string data = time + "|" + type + "|" + username + "|" + receiver + "|" + filename + "|" + to_string(size);
            
            const char* messageC = data.c_str();

            if (send(clientSocket, messageC, strlen(messageC), 0) <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
            if (bytes <= 0 || string(buffer) != data) {
                cout << string(buffer) << endl;
                return false;
            }

            size_t sent = 0;
            while (sent < fileData.size()) {
                ssize_t n = send(clientSocket, fileData.data() + sent, fileData.size() - sent, 0);
                if (n <= 0)
                    return false;

                sent += n;
            }

            encryptAppendLine(time, filename, msgKey);

            return true;
        }

        bool isUserExistent(string user) {
            string type = "EXS";
            string data = currentTime() + "|" + type + "|" + user;

            const char* message = data.c_str();

            ssize_t sent = send(clientSocket, message, strlen(message), 0);
            if (sent <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);

            if (bytes > 0) {
                if (string(buffer) == data) {
                    return true;
                }
            }

            return false;
        }

        bool call(string receiver) {
            string type = "CAL";
            string time = currentTime();
            string data = time + "|" + type + "|" + username + "|" + receiver;
            
            const char* messageC = data.c_str();

            if (send(clientSocket, messageC, strlen(messageC), 0) <= 0) {
                return false;
            }

            char buffer[4096] = {0};
            int bytes = recv(clientSocket, buffer, (sizeof(buffer)-1), 0);
            if (bytes <= 0 || string(buffer) != data) {
                cout << string(buffer) << endl;
                return false;
            }

            return true;
        }

        bool createAccountHandler() {
            string email, password, user;

            cout << "\033[2J\033[H";
            cout << "Podaj email: ";
            getline(cin, email);
            if (email.empty()) {
                return false;
            } else if (email.find('@') == string::npos || email.find('.') == string::npos) {
                cout << "\033[2J\033[H";
                cout << "Podano niepoprawny email." << endl;
                cout << endl;
                return false;
            }

            cout << "Podaj hasło: ";
            getline(cin, password);
            if (password.empty()) {
                return false;
            } else if (password.length() < 5) {
                cout << "\033[2J\033[H";
                cout << "Hasło musi mieć co najmniej 8 znaków." << endl;
                cout << endl;
                return false;
            }

            cout << "Podaj nazwę użytkownika: ";
            getline(cin, user);
            if (user.empty()) {
                return false;
            } else if (user.length() < 3) {
                cout << "\033[2J\033[H";
                cout << "Nazwa użytkownika musi mieć co najmniej 3 znaki." << endl;
                cout << endl;
                return false;
            } else if (password == user) {
                cout << "\033[2J\033[H";
                cout << "Hasło nie może być takie samo jak nazwa użytkownika." << endl;
                cout << endl;
                return false;
            }

            if (password.find('|') != string::npos || user.find('|') != string::npos || email.find('|') != string::npos) {
                cout << "\033[2J\033[H";
                cout << "Nazwa użytkownika, email i hasło nie mogą zawierać znaków specjalnych." << endl;
                cout << endl;
                return false;
            }

            if (createAccount(email, password, user)) {
                cout << "\033[2J\033[H";
                cout << "Utworzono konto." << endl;
                cout << endl;
            } else {
                return false;
            }

            return true;
        }

        bool loginHandler() {
            string password;

            cout << "\033[2J\033[H";
            cout << "Podaj nazwę użytkownika: " << endl;
            getline(cin, username);

            if (username.empty()) {
                return false;
            }

            cout << "Podaj hasło: " << endl;
            getline(cin, password);
            if (password.empty()) {
                return false;
            }

            if (login(password)) {
                changeStatus();
                isLoggedIn = true;
                cout << "\033[2J\033[H";
                cout << "Zalogowano pomyślnie." << endl;
                cout << endl;
            } else {
                return false;
            }

            return true;
        }

        bool logoutHandler() {
            if (changeStatus()) {
                cout << "\033[2J\033[H";
                cout << "Wylogowano pomyślnie." << endl;
                cout << endl;
                isLoggedIn = false;
                username = "";
                msgKey = "";
                return true;
            } else {
                return false;
            }
        }
};


class APP {
    private:
        string receiver;
        bool isLoggedIn = false;

        atomic<bool> running{false};
        atomic<bool> inConv{false};
        thread syncThread;

    public:
        APP() {}

        ~APP() {}

        Rooms rooms;

        bool synchronization() {
            int startUp = 0;
            int conversations = 0;

            while (running) {
                Connection connection(PORT, SERVER_IP);
                if (startUp == 0) {
                    connection.synchronization("ALL", rooms);
                    startUp++;
                } else {
                    connection.synchronization("UPDATE", rooms);
                }
                
                if (conversations != rooms.count && inConv == false && running == true) {
                    conversations = rooms.count;
                    displayConversations();
                } else if (inConv == true && running == true) {
                    displayMessages(false);
                }

                if (running == true) {
                    timer(0.05); //0.017min*60s=1s
                }
            }

            return true;
        }

        void displayConversations() {
            cout << "\033[2J\033[H";
            cout << "Strona główna..." << endl;
            cout << endl;

            cout << "Twoje rozmowy - " << rooms.count << endl;
            for (int i=0; i<rooms.count; i++) {
                cout << rooms.conversations[i].user << endl;
            }

            cout << endl;
            cout << "Wybierz 0 aby się wylogować (trzeba je dwa razy podać), exit aby zakończyć działanie aplikacji lub nazwe rozmówcy" << endl;
        }


        bool displayMessages(bool fullLoad) {
            Conversation conv;
            Message msg;

            winsize w{};
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            int width = w.ws_col;

            int convID = -1;

            for (int i=0; i<rooms.count; i++) {
                if (rooms.conversations[i].user == receiver) {
                    conv = rooms.conversations[i];
                    convID = i;
                }
            }

            if (fullLoad == true) {
                for (int i=0; i<conv.count; i++) {
                    msg = conv.messages[i];

                    if (msg.author != username) {  
                        cout << setw(width) << right << msg.content << endl; 
                    } else {
                        cout << msg.content << endl;  
                    }
                    rooms.conversations[convID].messages[i].unread = false;
                }
            } else if (convID == -1) {

            } else {
                msg = conv.messages[conv.count - 1];

                if (msg.author != username && msg.unread == true) {
                    cout << setw(width) << right << msg.content << endl;  
                    rooms.conversations[convID].messages[conv.count - 1].unread = false;
                }
            }

            return true;
        }

        int mainPage() {
            string input;

            running = true;
            syncThread = thread(&APP::synchronization, this);

            Connection connection(PORT, SERVER_IP);
            connection.synchronization("ALL", rooms);

            while (true) {
                inConv = false;
                displayConversations();

                getline(cin, input);

                if (input == "0") {
                    running = false;
                    syncThread.join();
                    return 1;
                } else if (input == "exit") {
                    running = false;
                    syncThread.join();
                    return 0;
                } else {
                    receiver = input;
                    Connection connection(PORT, SERVER_IP);
                    if (connection.isUserExistent(receiver) == true && receiver != username) {
                        conversation();
                    }
                }
            }
        }


        int conversation() {
            inConv = true;

            cout << "\033[2J\033[H";
            cout << "konwersacja z " << receiver << endl;
            cout << "\nNapisz 'file::/' i podaj ścieżke do pliku lub " << endl;
            cout << "Napisz 'back' aby wrócić na stronę główną lub napisz wiadomość: " << endl;

            if (displayMessages(true) == false) {
                return 1;
            }

            string message, file, filename;
            while (true) {
                getline(cin, message);
                if (message == "back") {
                    return 0;
                } else if (message.length() > 11 && message.substr(0, 7) == "file::/" && message.find(".") != string::npos) {
                    filename = message.substr(7, message.length());

                    Connection connection(PORT, SERVER_IP);
                    if (connection.fileUpload(receiver, filename) == false) {
                        return 1;
                    }
                    cout << "Wysłano plik: " << filename << endl;
                } else if (message == "call::") {
                    Connection connection(PORT, SERVER_IP);
                    if (connection.call(receiver) == false) {
                        return 1;
                    }
                    cout << "Dzwonienie do " << receiver << endl;
                } else {
                    Connection connection(PORT, SERVER_IP);
                    if (connection.upload(receiver, message) == false) {
                        return 1;
                    }
                }
            }

            return 0;
        }
};



int main() {
    if (sodium_init() < 0) {
        return 1;
    }

    string mode;
    cout << "\033[2J\033[H";

    while  (true) {
        Connection conn(PORT, SERVER_IP);  
        if (!isLoggedIn) {
            cout << "Wpisz 1 aby utworzyć konto, 2 aby się zalogować lub 'exit' aby zakończyć: " << endl;
        }

        getline(cin, mode);

        if (mode == "1" && !isLoggedIn) {
            conn.createAccountHandler();
        } else if (mode == "2" && !isLoggedIn) {
            if (conn.loginHandler() == true) {
                APP app;
                app.mainPage();

                if (app.mainPage() == 0) {
                    Connection connection(PORT, SERVER_IP);
                    connection.logoutHandler();
                    cout << "Wyłączanie aplikacji..." << endl;
                    break;
                } else {
                    Connection connection(PORT, SERVER_IP);
                    connection.logoutHandler();
                }  
            }
        } else if (mode == "exit") {
            break;
        } else {
            cout << "Nieznana komenda." << endl;
        }

        conn.disconnect();
    }

    isLoggedIn = false;
    return 0;
}