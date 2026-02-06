#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <sys/stat.h>
#include <fstream>
#include <mutex>
#include <SFML/Network.hpp>

#include "functions.cpp"
#include "database.cpp"
#include "email.cpp"

using namespace std;

#define PORT 1110
#define USERS 50

SmtpConfig cfg;
Database db;

vector<bool> ports;
mutex portsMutex;


int getPort() {
    lock_guard<mutex> lock(portsMutex);
    for (int i=0; i<USERS; i++) {
        if (ports[i] == true) {
            ports[i] = false;
            return i;
        }
    }

    return -1;
}

bool accountCreationHandler(const string& date, const string& time, string& data, int length, int clientSocket, char* buffer) {
    string error;
    string generatedToken = verification_code(7);
    
    int delimiterA = data.find("|");
    int delimiterB = data.find("|", delimiterA + 1);
    int delimiterC = data.find("|", delimiterB + 1);

    string email = data.substr(0, delimiterA);
    string password = data.substr(delimiterA + 1, (delimiterB - delimiterA - 1));
    string user = data.substr(delimiterB + 1, (delimiterC - delimiterB - 1));
    string publicKey64 = data.substr(delimiterC  + 1, 64);
    string privateKey = data.substr(delimiterC  + 45, 64);

    string recipient = email;
    string sender    = cfg.username;
    string subject   = "Kod weryfikacyjny";
    string body      = generatedToken;

    EmailPayload payload(recipient, sender, subject, body, true);
    EmailSender senderObj(recipient, sender, cfg, payload);

    if (db.search("USERS", "USERNAME", user) != "") {
        error = "Konto o podanej nazwie użytkownika już istnieje.\0";
        const char* errorMessage = error.c_str();
        send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
        return false;
    } else if (db.search("USERS", "EMAIL", email) != "") {
        error = "Konto o podanym emailu już istnieje.";
        const char* errorMessage = error.c_str();
        send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
        return false;
    } else {
        send(clientSocket, buffer, sizeof(buffer) - 1, 0);
    }

    senderObj.send();

    bool gotTime = true;
    cout << "Generated verification code: " << generatedToken << endl;
    thread t(timer, 5);
    t.detach();


    buffer[4096] = {0};

    int bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
    if (bytes > 0) {
        string receivedToken = string(buffer, 7);
        if (receivedToken != generatedToken || gotTime == false) {
            error = "Niepoprawny kod weryfikacyjny.";
            const char* errorMessage = error.c_str();
            send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            return false;
        } else {
            send(clientSocket, buffer, sizeof(buffer) - 1, 0);

            db.insertData("USERS", {db.hashText(user), user, email, db.hashText(password), "OFFLINE", publicKey64, privateKey});
            db.newTable(db.hashText(user), {{"ID", "INTEGER PRIMARY KEY AUTOINCREMENT"}, {"SENDER", "TEXT"}, {"RECEIVER", "TEXT"}, {"DATE", "TEXT"}, {"TIME", "TEXT"}, {"MESSAGE", "TEXT"}, {"IS_SYNCED", "TEXT"}, {"FILE", "TEXT"}, {"PORT", "TEXT"}});            
            
            return true;
        }
    }

    return false;
}

string loginHandler(const string& date, const string& time, string& data, int length) {
    int delimiter = data.find("|");

    string user = data.substr(0, delimiter);
    string password = data.substr(delimiter + 1, (length - delimiter - 1));

    string result = db.searchInColumn("USERS", "USERNAME", user, "PASSWORD");
    if (result == "") {
        result = db.searchInColumn("USERS", "EMAIL", user, "PASSWORD");
        if (result == "") {
            return "";
        }
    }

    if (result == db.hashText(password)) {
        return db.searchInColumn("USERS", "USER", db.hashText(user), "MSGKEY");
    } else {
        return "";
    }
}

bool messageHandler(int clientSocket, string receiver) {
    string key = db.searchInColumn("USERS", "USER", db.hashText(receiver), "KEY");
    const char* publicKey = key.c_str();
    send(clientSocket, publicKey, 65, 0);

    char buffer[4096] = {0};
    int length, bytes;
    string date, time, type, data;

    bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
    cout << buffer << endl;
    if (bytes > 0) {
        length = strlen(buffer);
        date = string(buffer, 10);
        time = string(buffer + 11, 8);
        type = string(buffer + 20, 3);
        data = string(buffer + 24, length - 24);


        int delimiterA = data.find("|");
        int delimiterB = data.find("|", delimiterA + 1);

        string user = data.substr(0, delimiterA);
        string receiver = data.substr(delimiterA + 1, (delimiterB - delimiterA - 1));
        string content = data.substr(delimiterB + 1, (length - delimiterB - 1));


        if (db.search("USERS", "USERNAME", user) != "" && db.search("USERS", "USERNAME", receiver) != "") {
            db.insertData(db.hashText(user), {user, receiver, date, time, content, "FALSE", "", ""});
            db.insertData(db.hashText(receiver), {user, receiver, date, time, content, "FALSE", "", ""});

            send(clientSocket, buffer, sizeof(buffer) - 1, 0);
            return true;
        }
    }

    return false;
}

bool changeStatusHandler(const string& user) {
    string currentStatus = db.search("USERS", "USERNAME", user);
    bool result;

    if (currentStatus.find("ONLINE") != string::npos) {
        result = db.changeData("USERS", "STATUS", "OFFLINE", "USERNAME", user);            
    } else {
        result = db.changeData("USERS", "STATUS", "ONLINE", "USERNAME", user);
    }

    return result;
}

bool synchronizationHandler(const string& data, int length, int clientSocket) {
    int delimiter = data.find("|");

    string mode = data.substr(0, delimiter);
    string userUnhased = data.substr(delimiter + 1, (length - delimiter - 1));
    string user = db.hashText(userUnhased);

    int id = 1;
    string synced = "FALSE";
    string date, time, sender, receiver, message, result, publicKey64, size, filename, port;

    if (mode == "ALL") {
        synced = db.searchInColumn(user, "ID", to_string(id), "IS_SYNCED");

        while (synced != "") {
            date = db.searchInColumn(user, "ID", to_string(id), "DATE");
            time = db.searchInColumn(user, "ID", to_string(id), "TIME");
            sender = db.searchInColumn(user, "ID", to_string(id), "SENDER");
            receiver = db.searchInColumn(user, "ID", to_string(id), "RECEIVER");
            message = db.searchInColumn(user, "ID", to_string(id), "MESSAGE");
            publicKey64 = db.searchInColumn("USERS", "USER", db.hashText(sender), "KEY");
            size = db.searchInColumn(user, "ID", to_string(id), "FILE");
            port = db.searchInColumn(user, "ID", to_string(id), "PORT");

            if (size == "" && port == "") {
                result = date + time + "|" + sender + "|" + receiver + "|" + message + "|" + publicKey64 + "\n";
                const char* resultMessage = result.c_str();
                send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);
            } else if (port != "0" && size == "" && port != "") {
                result = date + time + "|" + sender + "|" + receiver + "||Call::/" + port + "\n";
                const char* resultMessage = result.c_str();
                send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);
            } else if (port == "" && size != "") {
                filename = db.hashText((date + time + sender)) + ".dat";
                ifstream file(filename);
                if (userUnhased != sender && file.good()) {
                    result = date + time + "|" + sender + "|" + receiver + "|" + message + "|File::/" + size + "\n";
                    const char* resultMessage = result.c_str();
                    send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);

                    if (sendFile(clientSocket, filename) == false) {
                        return false;
                    }
                }
            }
            // result = date + time + "|" + sender + "|" + receiver + "|" + message + "|" + publicKey64 + "\n";
            // const char* resultMessage = result.c_str();
            // send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);

            db.changeData(user, "IS_SYNCED", "TRUE", "ID", to_string(id));

            id++;
            synced = db.searchInColumn(user, "ID", to_string(id), "IS_SYNCED");
        }

    } else if (mode == "UPDATE") {
        synced = db.searchInColumn(user, "ID", to_string(id), "IS_SYNCED");

        while (synced != "") {
            if (synced == "FALSE") {
                date = db.searchInColumn(user, "ID", to_string(id), "DATE");
                time = db.searchInColumn(user, "ID", to_string(id), "TIME");
                sender = db.searchInColumn(user, "ID", to_string(id), "SENDER");
                receiver = db.searchInColumn(user, "ID", to_string(id), "RECEIVER");
                message = db.searchInColumn(user, "ID", to_string(id), "MESSAGE");
                publicKey64 = db.searchInColumn("USERS", "USER", db.hashText(sender), "KEY");
                size = db.searchInColumn(user, "ID", to_string(id), "FILE");
                port = db.searchInColumn(user, "ID", to_string(id), "PORT");

                if (size == "" && port == "") {
                    result = date + time + "|" + sender + "|" + receiver + "|" + message + "|" + publicKey64 + "\n";
                    const char* resultMessage = result.c_str();
                    send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);
                    cout << result << endl;
                } else if (port != "0" && size == "" && port != "") {
                    result = date + time + "|" + sender + "|" + receiver + "||Call::/" + port + "\n";
                    const char* resultMessage = result.c_str();
                    send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);
                } else if (port == "" && size != "") {
                    filename = db.hashText((date + time + sender)) + ".dat";
                    ifstream file(filename);
                    if (userUnhased != sender && file.good()) {
                        result = date + time + "|" + sender + "|" + receiver + "|" + message + "|File::/" + size + "\n";
                        const char* resultMessage = result.c_str();
                        send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);

                        if (sendFile(clientSocket, filename) == false) {
                            return false;
                        }
                    }
                }
                // result = date + time + "|" + sender + "|" + receiver + "|" + message + "|" + publicKey64 + "\n";
                // const char* resultMessage = result.c_str();
                // send(clientSocket, resultMessage, strlen(resultMessage) - 1, 0);

                db.changeData(user, "IS_SYNCED", "TRUE", "ID", to_string(id));
            }

            id++;
            synced = db.searchInColumn(user, "ID", to_string(id), "IS_SYNCED");
        }

    } else {
        return false;
    }

    result = "|END|\n";
    const char* endMessage = result.c_str();
    send(clientSocket, endMessage, strlen(endMessage) - 1, 0);

    return true;
}

uint64_t fileHandler(int clientSocket, string date, string time, string data, int length) {
    int delimiterA = data.find("|");
    int delimiterB = data.find("|", delimiterA + 1);
    int delimiterC = data.find("|", delimiterB + 1);

    string user = data.substr(0, delimiterA);
    string receiver = data.substr(delimiterA + 1, (delimiterB - delimiterA - 1));
    string filename = data.substr(delimiterB + 1, (delimiterC - delimiterB - 1));
    string size = data.substr(delimiterC  + 1, (length - delimiterC - 1));


    if (db.search("USERS", "USERNAME", user) != "" && db.search("USERS", "USERNAME", receiver) != "") {
        db.insertData(db.hashText(user), {user, receiver, date, time, filename, "FALSE", size, ""});
        db.insertData(db.hashText(receiver), {user, receiver, date, time, filename, "FALSE", size, ""});

        return stoull(size);
    }

    return 0;
}

int callHandler(int clientSocket, string date, string time, string data, int length) {
    int delimiterA = data.find("|");

    string user = data.substr(0, delimiterA);
    string receiver = data.substr(delimiterA + 1, (length - delimiterA - 1));
    int port = PORT + 1 + getPort();


    if (db.search("USERS", "USERNAME", user) != "" && db.search("USERS", "USERNAME", receiver) != "" && port > PORT) {
        db.insertData(db.hashText(user), {user, receiver, date, time, "", "FALSE", "", to_string(port)});
        db.insertData(db.hashText(receiver), {user, receiver, date, time, "", "FALSE", "", to_string(port)});

        return port;
    }

    return 0;   
}

bool closeCall(int clientSocket, string date, string time, string data, int length, int port) {
    int delimiterA = data.find("|");

    string user = db.hashText(data.substr(0, delimiterA));
    string receiver = db.hashText(data.substr(delimiterA + 1, (length - delimiterA - 1)));
    
    string id = db.searchInColumn(user, "PORT", to_string(port), "ID");
    db.changeData(user, "PORT", "0", "ID", id);

    id = db.searchInColumn(receiver, "PORT", to_string(port), "ID");
    db.changeData(receiver, "PORT", "0", "ID", id);

    lock_guard<mutex> lock(portsMutex);
    ports[port - 1 - PORT] = true;

    return true;
}

void audioDistributor(unsigned short port) {
    sf::TcpListener listener;
    listener.listen(port);
    listener.setBlocking(false);

    sf::TcpSocket clientA, clientB;
    sf::SocketSelector selector;
    selector.add(listener);

    cout << "Czekam na klienta A" << endl;

    sf::Clock clock;
    bool clientAConnected = false;

    while (clock.getElapsedTime().asSeconds() < 5.f) {
        if (selector.wait(sf::seconds(0.5))) {
            if (selector.isReady(listener)) {
                if (listener.accept(clientA) == sf::Socket::Status::Done) {
                    clientAConnected = true;
                    cout << "Klient A podłączony" << endl;
                    break;
                }
            }
        }
    }

    if (!clientAConnected) {
        clientB.disconnect();
        cout << "Timeout brak klienta A" << endl;
        return;
    }

    cout << "Czekam na klienta B" << endl;;

    bool clientBConnected = false;

    listener.setBlocking(true);
    while (clock.getElapsedTime().asSeconds() < 30.f) {
        if (selector.wait(sf::seconds(0.5))) {
            if (selector.isReady(listener)) {
                listener.accept(clientB);
                clientBConnected = true;
                break;
            }
        }
    }

    if (!clientBConnected) {
        clientA.disconnect();
        cout << "Timeout brak klienta B" << endl;
        return;
    }

    cout << "Klient B podłączony" << endl;

    // auto relay = [](sf::TcpSocket& in, sf::TcpSocket& out) {
    //     while (true) {
    //         sf::Packet p;
    //         if (in.receive(p) != sf::Socket::Done)
    //             break;
    //         out.send(p);
    //     }
    // };

    // thread t1(relay, ref(clientA), ref(clientB));
    // thread t2(relay, ref(clientB), ref(clientA));

    auto relay = [](sf::TcpSocket& in, sf::TcpSocket& out, sf::TcpSocket& sender) {
        while (true) {
            sf::Packet p;
            if (in.receive(p) != sf::Socket::Status::Done)
                break;

            if (&in != &sender)
                out.send(p);
        }
    };


    thread t1(relay, ref(clientA), ref(clientB), ref(clientA));
    thread t2(relay, ref(clientB), ref(clientA), ref(clientB));

    t1.join();
    t2.join();
}


void handleClient(int clientSocket) {
    int bytes, length, port;
    char buffer[4096] = {0};
    string date, time, type, data, error, result, out, path, user, hashedFilename;
    uint64_t size;

    bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
    if (bytes > 0) {
        length = strlen(buffer);
        date = string(buffer, 10);
        time = string(buffer + 11, 8);
        type = string(buffer + 20, 3);
        data = string(buffer + 24, length - 24);
        

        if (type == "VER") {
            if (accountCreationHandler(date, time, data, length, clientSocket, buffer) == false) {
                error = "Podano nie poprawny kod weryfikacyjny";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            }
            
        } else if (type == "LOG") {
            cout << buffer << endl;
            result = loginHandler(date, time, data, length);

            if (result == "") {
                error = "Nie udało się zalogować. Spróbuj ponownie.";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            } else {
                out = string(buffer) + result;

                send(clientSocket, out.c_str(), out.length(), 0);
            }
            
        } else if (type == "MSG") {
            cout << buffer << endl;
            if (messageHandler(clientSocket, data) == false) {
                error = "Podany użytkownik nie istnieje. ";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            }

        } else if (type == "FIL") {
            cout << buffer << endl;
            size = fileHandler(clientSocket, date, time, data, length - 24);
            if (size == 0) {
                error = "Podany użytkownik nie istnieje. ";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            } else {
                send(clientSocket, buffer, sizeof(buffer) - 1, 0);
                user = data.substr(0, data.find("|"));
                hashedFilename = db.hashText((date + time + user)) + ".dat";
                cout << date << time << user << hashedFilename << endl;
                receiveFile(clientSocket, hashedFilename, size);
            }

        } else if (type == "STS") {
            cout << buffer << endl;
            if (changeStatusHandler(data) == false) {
                error = "Błąd podczas zmiany statusu.";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            } else {
                send(clientSocket, buffer, sizeof(buffer) - 1, 0);
            }

        } else if (type == "SYN") {
            if (synchronizationHandler(data, length, clientSocket) == false) {
                error = "Błąd podczas synchronizacji.";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            }

        } else if (type == "EXS") {
            if (db.search("USERS", "USERNAME", data) != "") {
                send(clientSocket, buffer, sizeof(buffer) - 1, 0);
            } else {
                error = "Użytkownik o podanej naziwe nie istnieje.";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            }

        } else if (type == "CAL") {
            cout << buffer << endl;
            port = callHandler(clientSocket, date, time, data, length - 24);
            if (port == 0) {
                error = "Nie mozna chwilowo otworzyć połączenia";
                const char* errorMessage = error.c_str();
                send(clientSocket, errorMessage, strlen(errorMessage) - 1, 0);
            } else {
                send(clientSocket, buffer, sizeof(buffer) - 1, 0);
                audioDistributor(port);
                closeCall(clientSocket, date, time, data, length - 24, port);
            }

        } else if (type == "ECL") {
            cout << buffer << endl;
            port = stoi(data.substr(length - 4, 4));
            closeCall(clientSocket, date, time, data, length - 29, port);

        } else {
            cout << buffer << endl;
            cout << "Nieznany typ otrzymanej wiadomości." << endl;
            send(clientSocket, buffer, sizeof(buffer) - 1, 0);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(clientSocket);
}


void server() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if (listen(serverSocket, USERS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        cout << "Nowe połączenie klienta." << endl;

        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
}


int main() {
    mkdir("ServerFiles", 0755);

    db.newTable("USERS", {{"ID", "INTEGER PRIMARY KEY AUTOINCREMENT"}, {"USER", "TEXT"}, {"USERNAME", "TEXT"}, {"EMAIL", "TEXT"}, {"PASSWORD", "TEXT"}, {"STATUS", "TEXT"}, {"KEY", "TEXT"}, {"MSGKEY", "TEXT"}});

    cfg.username = "mailNotToBeShowned";
    cfg.password = "zqzynyuyfjotnrnv";

    for (int i=0; i<USERS; i++) {
        ports.push_back(true);
    }

    server();
    return 0;
}