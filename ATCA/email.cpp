#include <iostream>
#include <vector>
#include <cstring>
#include <curl/curl.h>

using namespace std;


struct SmtpConfig {
    string server = "smtps://smtp.gmail.com:465";
    string username;
    string password;
};


class EmailPayload {
    private:
        vector<string> lines;

    public:
        EmailPayload(const string& to, const string& from, const string& subject, const string& body, bool verificationCode = false) {
            lines.push_back("To: <" + to + ">\r\n");
            lines.push_back("From: <" + from + "> (ATCA)\r\n");

            if (verificationCode && body.size() == 7) {
                lines.push_back("Subject: Kod weryfikacyjny do serwisu ATCA\r\n");
                lines.push_back("\r\n");
                lines.push_back("Oto Twoj kod weryfikacyjny:\r\n");
                lines.push_back(body + "\r\n");
            } else {
                lines.push_back("Subject: " + subject + "\r\n");
                lines.push_back("\r\n");
                lines.push_back(body + "\r\n");
                lines.push_back("\r\n");
            }
        }

        vector<string> getLines() const {
            return lines;
        }
};


class EmailSender {
    private:
        string receiver;
        string sender;
        SmtpConfig config;
        EmailPayload payload;
        size_t lines_read;

        static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
            if ((size * nmemb) < 1) return 0;

            EmailSender *self = static_cast<EmailSender*>(userp);

            const auto& lines = self->payload.getLines();
            if (self->lines_read < lines.size()) {
                const string& line = lines[self->lines_read];
                size_t len = min(line.size(), size * nmemb);
                memcpy(ptr, line.c_str(), len);
                self->lines_read++;
                return len;
            }
            return 0;
        }

    public:
        EmailSender(const string& to, const string& from, const SmtpConfig& cfg, const EmailPayload& pl)
            : receiver(to), sender(from), config(cfg), payload(pl), lines_read(0) {}

        bool send() {
            CURL *curl = curl_easy_init();
            if (!curl) return false;

            struct curl_slist *receivers = NULL;
            string mail_from = "<" + sender + ">";
            string mail_to   = "<" + receiver + ">";

            curl_easy_setopt(curl, CURLOPT_URL, config.server.c_str());
            curl_easy_setopt(curl, CURLOPT_USERNAME, config.username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, config.password.c_str());
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mail_from.c_str());

            receivers = curl_slist_append(receivers, mail_to.c_str());
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, receivers);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
            curl_easy_setopt(curl, CURLOPT_READDATA, this);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            CURLcode res = curl_easy_perform(curl);

            curl_slist_free_all(receivers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                cout << "Błąd wysyłki: " << curl_easy_strerror(res) << endl;
                return false;
            }

            return true;
        }
};


// int main() {
//     SmtpConfig cfg;
//     cfg.username = "sroczynskizbigniew6@gmail.com";
//     cfg.password = "zqzynyuyfjotnrnv";

//     string recipient = "lukisroczka094@gmail.com";
//     string sender    = cfg.username;
//     string subject   = "Test Modularny";
//     string body      = "5431236";

//     EmailPayload payload(recipient, sender, subject, body, true);
//     EmailSender senderObj(recipient, sender, cfg, payload);

//     senderObj.send();

//     return 0;
// }