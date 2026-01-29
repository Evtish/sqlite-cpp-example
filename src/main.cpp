#ifdef _WIN32
#include "sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

using ResT = std::unordered_map<std::string, std::vector<std::string>>; 

int db_callback_select(void *res, int col_count, char **col_data, char **col_names) {
	ResT *conv_res = reinterpret_cast<ResT *>(res);
	if (!conv_res) return 1;

	for (int i = 0; i < col_count; i++) {
		std::string col_name = col_names[i];
		if (!col_name.empty()) {
			std::string col_value = (col_data[i] ? col_data[i] : "");
			(*conv_res)[col_name].push_back(col_value);
		}
	}

	return 0;
}

int main() {
	sqlite3 *db_con = nullptr;
	if (sqlite3_open("test.db", &db_con) != SQLITE_OK) {
		std::cerr << "Ошибка открытия БД\n" << std::flush;
		return 1;
	}

	char *err_msg = nullptr;
	if (sqlite3_exec(db_con, "CREATE TABLE 'students' (\
		'id'	INTEGER NOT NULL UNIQUE,\
		'name'	TEXT NOT NULL,\
		PRIMARY KEY('id' AUTOINCREMENT)\
	);", nullptr, nullptr, &err_msg) != SQLITE_OK) {
		std::cerr << "Ошибка при выполнении запроса: " << err_msg << '\n' << std::flush;
		sqlite3_free(err_msg);
		return 1;
	}

	std::string name{};
	std::cout << "Введите имя: ";
	std::getline(std::cin, name);

	std::stringstream ss{};
	ss << "SELECT id FROM students WHERE name = '" << name << "';";

	ResT res{};
	if (sqlite3_exec(db_con, ss.str().c_str(), db_callback_select, &res, &err_msg) != SQLITE_OK) {
		std::cerr << "Ошибка при выполнении запроса: " << err_msg << '\n' << std::flush;
		sqlite3_free(err_msg);
		return 1;
	}

	for (const auto &pr : res) {
		std::cout << pr.first << ": ";
		for (const auto &elem : pr.second) {
			std::cout << elem << ' ';
		}
		std::cout << '\n';
	}

	sqlite3_close(db_con);

	return 0;
}
