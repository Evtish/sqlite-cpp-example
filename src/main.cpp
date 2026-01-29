/* Что необходимо дополнить:
 * TODO: чтобы предотвратить SQL-инъекции, заменить sqlite_exec на отдельные функции (sqlite3_prepare, sqlite3_bind, sqlite3_step, sqlite3_column, sqlite3_finalize)
 * TODO: чтобы кириллица в БД работала на Windows, преобразовывать std::string в std::wstring и наоборот
 * TODO: вынести работу с БД в отдельный класс или функции */

#ifdef _WIN32
#include <Windows.h>
#include "sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using ResT = std::unordered_map<std::string, std::vector<std::string>>;	// Псевдоним для типа

// Callback для sqlite3_exec (записывает название и значения столбцов в unordered_map)
// static - чтобы функция была доступна только в пределах этого файла
static int db_callback_select(void *res, int col_count, char **col_data, char **col_names) {
	ResT *conv_res = reinterpret_cast<ResT *>(res);	// Преобразование указателя
	if (!conv_res) return 1;	// Вернуть ошибку при неудачном преобразовании
	

	// Ключ - название столбца, значение - вектор значений этого столбца
	for (int i = 0; i < col_count; i++) {
		std::string col_name = col_names[i];
		if (!col_name.empty()) {
			std::string col_value = (col_data[i] ? col_data[i] : "");	// Пустая строка, если значение = NULL
			(*conv_res)[col_name].push_back(col_value);
		}
	}

	return 0;
}

int main() {
	#ifdef _WIN32
	setlocale(LC_ALL, "Russian");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	#endif

	// Открытие БД
	sqlite3 *db_con = nullptr;
	if (sqlite3_open("test.db", &db_con) != SQLITE_OK) {
		std::cerr << "Ошибка при открытии БД\n" << std::flush;
		return 1;
	}

	// Создание таблицы, если она отсутствует
	char *err_msg = nullptr;
	if (sqlite3_exec(
		db_con,
		"CREATE TABLE IF NOT EXISTS 'students' ( \
			'id'	INTEGER NOT NULL UNIQUE, \
			'name'	TEXT NOT NULL, \
			PRIMARY KEY('id' AUTOINCREMENT) \
		);",
		nullptr, nullptr, &err_msg
	) != SQLITE_OK) {
		std::cerr << "Ошибка при выполнении запроса: " << err_msg << '\n' << std::flush;
		sqlite3_free(err_msg);
		return 1;
	}

	// Формирование запроса с вводом имени (небезопасно, возможна SQL-инъекция)
	std::string name{};
	std::cout << "Введите имя: ";
	std::getline(std::cin, name);
	std::ostringstream ss{};
	ss << "SELECT id FROM students WHERE name = '" << name << "';";

	// Выполнение запроса
	ResT res{};
	if (sqlite3_exec(db_con, ss.str().c_str(), db_callback_select, &res, &err_msg) != SQLITE_OK) {
		std::cerr << "Ошибка при выполнении запроса: " << err_msg << '\n' << std::flush;
		sqlite3_free(err_msg);
		return 1;
	}
	
	// Вывод результата запроса
	for (const auto &pr : res) {
		std::cout << pr.first << ": ";
		for (const auto &elem : pr.second) {
			std::cout << elem << ' ';
		}
		std::cout << '\n';
	}

	sqlite3_close(db_con);	// Закрытие БД

	return 0;
}
