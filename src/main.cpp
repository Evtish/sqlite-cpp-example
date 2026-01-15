#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sqlite3.h>

static int db_select_callback(void *res, int n, char **value_of_clmns, char **name_of_clmns) {
    std::unordered_map<std::string, std::vector<std::string>> *conv_res = reinterpret_cast<std::unordered_map<std::string, std::vector<std::string>>*>(res);
    for (int i = 0; i < n; i++) {
        if (name_of_clmns[i]) {
            std::string clmn_name = name_of_clmns[i];
            std::string clmn_value = value_of_clmns[i] ? value_of_clmns[i] : "";
            (*conv_res)[clmn_name].push_back(clmn_value);
        }
    }
    return 0;
}

int main() {
    sqlite3 *db_con;
    if (sqlite3_open("test.db", &db_con) != SQLITE_OK) {
        std::cerr << "Ошибка при открытии БД\n";
        return 1;
    }

    std::string name{};
    std::cout << "Введите имя: ";
    std::getline(std::cin, name);

    const std::string stmt = "SELECT id FROM student WHERE name = '" + name + "';";

    std::unordered_map<std::string, std::vector<std::string>> res{};
    char* error_message = nullptr;
    if (sqlite3_exec(db_con, stmt.c_str(), db_select_callback, &res, &error_message) != SQLITE_OK) {
        std::cerr << "Ошибка при выполнении запроса: " << error_message << '\n';
        sqlite3_free(error_message);
    }

    for (const std::pair<const std::string, std::vector<std::string>> &line: res) {
        std::cout << line.first << ": ";
        for (const std::string& elem : line.second) {
            std::cout << elem << ' ';
        }
        std::cout << '\n';
    }
    
    while (sqlite3_close(db_con) != SQLITE_OK);

    return 0;
}
