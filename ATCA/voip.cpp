#include <iostream>
#include <mutex>
#include <optional>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>

#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

using namespace std;


constexpr uint8_t clientAudioData   = 1;
constexpr uint8_t clientEndOfStream = 2;

constexpr uint8_t serverAudioData   = 1;
constexpr uint8_t serverEndOfStream = 2;

struct ClientConnection {
    sf::TcpSocket socket;
    mutex sendMutex;
};


class NetworkRecorder : public sf::SoundRecorder {
    private:
        ClientConnection& conn;
        sf::IpAddress  m_host; 
        unsigned short m_port;
        sf::TcpSocket  m_socket;

        bool onStart() override { return true; }

        bool onProcessSamples(const int16_t* samples, size_t count) override {
            sf::Packet packet;
            packet << clientAudioData;
            packet.append(samples, count * sizeof(int16_t));

            lock_guard lock(conn.sendMutex);
            return conn.socket.send(packet) == sf::Socket::Status::Done;
        }

        void onStop() override {
            sf::Packet packet;
            packet << clientEndOfStream;

            lock_guard lock(conn.sendMutex);
            conn.socket.send(packet);
        }
    public:
        NetworkRecorder(ClientConnection& c, const sf::IpAddress& host) : conn(c), m_host(host) {}

        ~NetworkRecorder() override {
            stop();
        }
};

class NetworkAudioStream : public sf::SoundStream {
    private:
        bool onGetData(sf::SoundStream::Chunk& data) override {
            if (!m_running && m_offset >= m_samples.size())
                return false;

            while (m_offset >= m_samples.size()) {
                if (!m_running)
                    return false;

                sf::sleep(sf::milliseconds(5));
            }

            lock_guard lock(m_mutex);

            m_tempBuffer.assign(
                m_samples.begin() + static_cast<vector<int16_t>::difference_type>(m_offset),
                m_samples.end()
            );

            data.samples = m_tempBuffer.data();
            data.sampleCount = m_tempBuffer.size();

            m_offset += m_tempBuffer.size();
            return true;
        }

        void onSeek(sf::Time timeOffset) override {
            m_offset = static_cast<std::size_t>(timeOffset.asMilliseconds()) * getSampleRate() * getChannelCount() / 1000;
        }

        void receiveLoop() {
        while (m_running) {
            sf::Packet packet;
            auto status = conn.socket.receive(packet);

            if (status != sf::Socket::Status::Done) {
                m_running = false;
                break;
            }

            uint8_t id;
            packet >> id;

            if (id == serverAudioData) {
                const auto* data = reinterpret_cast<const int16_t*>(static_cast<const uint8_t*>(packet.getData()) + 1);

                size_t count = (packet.getDataSize() - 1) / sizeof(int16_t);

                lock_guard lock(m_mutex);
                m_samples.insert(m_samples.end(), data, data + count);
            }
            else if (id == serverEndOfStream) {
                m_running = false;
                break;
            }
        }
}

        sf::TcpListener m_listener;
        sf::TcpSocket m_client;
        recursive_mutex m_mutex;
        vector<int16_t> m_samples;
        vector<int16_t> m_tempBuffer;
        size_t m_offset{};
        bool m_hasFinished{};
        ClientConnection& conn;
        atomic<bool> m_running{true};
    public:
        NetworkAudioStream(ClientConnection& c) : conn(c) {
            initialize(1, 44100, { sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight });
        }

        void start() {
            play();
            receiveLoop();
        }

        void shutdown() {
            m_running = false;
            stop();
        }
};

void runCall(string serverIp, int port) {
    ClientConnection conn;
    char newIP = serverIp.c_str()[0];
    sf::IpAddress ip(newIP);
    conn.socket.connect(ip, port);

    NetworkRecorder recorder(conn, ip);
    NetworkAudioStream stream(conn);

    thread receiverThread([&] {
        stream.start();
    });

    recorder.start(44100);

    cout << "Naduś 3 razy 'ENTER' aby zakończyć" << endl;;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    recorder.stop();
    stream.shutdown();
    conn.socket.disconnect();

    receiverThread.join();

    cout << "Rozmowa zakończona" << endl;
}

// void audioDistributor(unsigned short port) {
//     sf::TcpListener listener;
//     listener.listen(port);

//     sf::TcpSocket clientA, clientB;

//     cout << "Czekam na klienta A...\n";
//     listener.accept(clientA);

//     cout << "Czekam na klienta B...\n";
//     listener.accept(clientB);

//     auto relay = [](sf::TcpSocket& in, sf::TcpSocket& out) {
//         while (true) {
//             sf::Packet p;
//             if (in.receive(p) != sf::Socket::Done)
//                 break;
//             out.send(p);
//         }
//     };

//     thread t1(relay, ref(clientA), ref(clientB));
//     thread t2(relay, ref(clientB), ref(clientA));

//     t1.join();
//     t2.join();
// }


// int main() {
//     char who;
//     std::cout << "Do you want to be a server ('s') or a client ('c')? ";
//     std::cin >> who;

//     if (who == 's')
//     {
//         audioDistributor(2435);
//     }
//     else
//     {
//         runCall("192.168.1.59", 2435);
//     }
//     return 0;
// }