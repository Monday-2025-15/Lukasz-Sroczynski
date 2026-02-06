#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <sqlite3.h>

using namespace std;


class Database {
    private:
        sqlite3* DB;
        string sql;
        char* messaggeError;
    public:
        Database() {
            sqlite3_open("ATCA.db", &DB);
        }

        ~Database() {
            sqlite3_close(DB);
        }

        int newTable(string table_name, vector<pair<string, string>> columns) {
            string sql = "CREATE TABLE IF NOT EXISTS '" + table_name + "' (";
            for (size_t i=0; i < columns.size() - 1; i++) {
                sql = sql + "" + columns[i].first + " " + columns[i].second + " NOT NULL, ";
            }
            sql = sql + "" + columns.back().first + " " + columns.back().second + " NOT NULL);";

            return sqlite3_exec(DB, sql.c_str(), nullptr, nullptr, &messaggeError);
        }

        int insertData(string table_name, vector<string> new_column) {
            sql = "INSERT INTO '" + table_name + "' VALUES (NULL, ";

            for (size_t i=0; i < new_column.size() - 1; i++) {
                sql = sql + "'" + new_column[i] + "', ";
            }
            sql = sql + "'" + new_column.back() + "');";

            return sqlite3_exec(DB, sql.c_str(), nullptr, nullptr, &messaggeError);
        }

        static int callback(void* data, int argc, char** argv, char** azColName) {
            string* result = static_cast<string*>(data);
            *result = "";
            for (int i = 0; i < argc; i++) {
                *result = *result + (argv[i] ? argv[i] : "NULL") + " | ";
            }
            return 0;
        }

        static int callback2(void* data, int argc, char** argv, char** azColName) {
            string* result = static_cast<string*>(data);
            *result = "";
            for (int i = 0; i < argc; i++) {
                *result = *result + (argv[i] ? argv[i] : "NULL");
            }
            return 0;
        }

        string search(string table_name, string column, string value) {
            string result;

            sql = "SELECT * FROM '" + table_name + "' WHERE " + column + " = '" + value + "';";
            sqlite3_exec(DB, sql.c_str(), callback, &result, &messaggeError);

            return result;
        }

        string hashText(const string& text) {
            size_t h = hash<string>{}(text);
            stringstream ss;
            ss << hex << h;
            return ss.str();
        }

        bool changeData(string table_name, string column, string value, string condition_column, string condition_value) {
            sql = "UPDATE '" + table_name + "' SET " + column + " = '" + value + "' WHERE " + condition_column + " = '" + condition_value + "';";

            return sqlite3_exec(DB, sql.c_str(), nullptr, nullptr, &messaggeError) == SQLITE_OK;
        }

        string searchInColumn(string table_name, string column, string value, string column_value) {
            string result;

            sql = "SELECT " + column_value + " FROM '" + table_name + "' WHERE " + column + " = '" + value + "';";
            sqlite3_exec(DB, sql.c_str(), callback2, &result, &messaggeError);

            return result;
        }
};



// int main()
// {
//     Database db;
//     db.newTable("USERS", {{"ID", "INTEGER PRIMARY KEY AUTOINCREMENT"}, {"USER ID", "TEXT"}, {"USERNAME", "TEXT"}, {"EMAIL", "TEXT"}, {"PASSWORD", "TEXT"}, {"PUBLIC_KEY", "TEXT"}});


//     string username = "LukiPLG";
//     db.insertData("USERS", {db.hashText(username), "LukiPLG", "lukisroczka094@gmail.com", "qwerty", "e2315r7fdwtquyv"});
//     db.newTable(db.hashText(username), {{"ID", "INTEGER PRIMARY KEY AUTOINCREMENT"}, {"SENDER", "TEXT"}, {"RECEIVER", "TEXT"}, {"TIME", "TEXT"}, {"MESSAGE", "TEXT"}});
//     db.insertData(db.hashText(username), {"LukiPLG", "adamKwietny", "21:37:42:00", "Kup se mozg."});


//     username = "adamKwietny";
//     db.insertData("USERS", {db.hashText(username), "adamKwietny", "Kwiatek11220.com", "ytrewq", "1r2e678dfgwuhoicq"});
//     db.newTable(db.hashText(username), {{"ID", "INTEGER PRIMARY KEY AUTOINCREMENT"}, {"SENDER", "TEXT"}, {"RECEIVER", "TEXT"}, {"TIME", "TEXT"}, {"MESSAGE", "TEXT"}});
//     db.insertData(db.hashText(username), {"LukiPLG", "adamKwietny", "21:37:42:00", "Kup se mozg."});

    
//     cout << db.search("USERS", "USERNAME", "LukiPLG") << endl;

//     return (0);
// }