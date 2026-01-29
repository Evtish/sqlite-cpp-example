/* -------------------- Пример класса для работы с БД с защитой от SQL-инъекций ------------------------- */

/* WARNING:
 * Код написан не самым оптимальным образом:
 * стоит разбить подготовку запроса, подстановку в запрос и т.д. на отдельные функции,
 * чтобы избавиться от повторений кода в перегрузках.
 *
 * Возможно, есть более оптимальный способ реализации
 * переменного количества параметров функции, чем std::initializer_list
 */

#include <string>
#include <iostream>

#include "converter.hpp"
#include "database.hpp"

Database::Database() : p_db(nullptr) {

}

Database::Database(const char *filename) {
    // Открыть БД по данному имени файла
    if (sqlite3_open(filename, &p_db) != SQLITE_OK) {
        std::wcerr << L"Ошибка при открытии БД: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        sqlite3_close(p_db);
        p_db = nullptr;
    }
}

Database::~Database() {
	// Закрыть бд, если она была открыта
	if (p_db) {
		sqlite3_close(p_db); 
	}
}

// Вставить/обновить/удалить все записи
Database::StatusCode Database::exec(const char *query, const std::initializer_list<std::wstring> binds) {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(p_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::wcerr << L"Ошибка при подготовке запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        return PREPARE_ERROR;
    }

    {int i = 0;
    for (const std::wstring &bind : binds) {
        const std::string conv_bind = utf8_encode(bind);
        if (sqlite3_bind_text(stmt, i + 1, conv_bind.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            std::wcerr << L"Ошибка при подстановке переменных в запрос: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
            sqlite3_finalize(stmt);
            return BIND_ERROR;
        }
        i++;
    }}

    int step_return_code = sqlite3_step(stmt);
    while (step_return_code == SQLITE_ROW) {
        step_return_code = sqlite3_step(stmt);
    }
    if (step_return_code != SQLITE_DONE) {
        std::wcerr << L"Ошибка при выполнении запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        sqlite3_finalize(stmt);
        return EXEC_ERROR;
    }

    sqlite3_finalize(stmt);
    return OK;
}

// Прочитать первую колонку в первой записи
Database::StatusCode Database::exec(std::wstring &res, const char *query, const std::initializer_list<std::wstring> binds) const {
    sqlite3_stmt *stmt;
    // Подготовка запроса
    if (sqlite3_prepare_v2(p_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::wcerr << L"Ошибка при подготовке запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        return PREPARE_ERROR;
    }

    // Подстановка элементов списка инициализации в запрос вместо знаков вопроса
    {int i = 0;
    for (const std::wstring &bind : binds) {
        const std::string conv_bind = utf8_encode(bind);
        if (sqlite3_bind_text(stmt, i + 1, conv_bind.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            std::wcerr << L"Ошибка при подстановке переменных в запрос: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
            sqlite3_finalize(stmt);
            return BIND_ERROR;
        }
        i++;
    }}

    const int step_return_code = sqlite3_step(stmt);
    // Чтение первой колонки в первой строке
    if (step_return_code == SQLITE_ROW) {
		const char *casted_col_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
		if (!casted_col_text) {
			std::wcerr << L"Ошибка reinterpret_cast\n" << std::flush;
            sqlite3_finalize(stmt);
			return CAST_ERROR;
		}
        res = utf8_decode(casted_col_text);
    }
    // Обработка ошибок
    else if (step_return_code != SQLITE_DONE) {
        std::wcerr << L"Ошибка при выполнении запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        sqlite3_finalize(stmt);
        return EXEC_ERROR;
    }

    sqlite3_finalize(stmt); // Вызов деструктора
    return (res.empty()) ? EMPTY : OK;
}

// Прочитать все колонки в первой записи или первую запись во всех колонках
Database::StatusCode Database::exec(std::vector<std::wstring> &res, const char *query, const std::initializer_list<std::wstring> binds, const bool read_line) const {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(p_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::wcerr << L"Ошибка при подготовке запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        return PREPARE_ERROR;
    }

    {int i = 0;
    for (const std::wstring &bind : binds) {
        const std::string conv_bind = utf8_encode(bind);
        if (sqlite3_bind_text(stmt, i + 1, conv_bind.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            std::wcerr << L"Ошибка при подстановке переменных в запрос: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
            sqlite3_finalize(stmt);
            return BIND_ERROR;
        }
        i++;
    }}

    int step_return_code = sqlite3_step(stmt);
    // Прочитать строку
    if (read_line) {
        if (step_return_code == SQLITE_ROW) {
            const int col_amount = sqlite3_column_count(stmt);
            for (int i = 0; i < col_amount; i++) {
				const char *casted_col_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
				if (!casted_col_text) {
					std::wcerr << L"Ошибка reinterpret_cast\n" << std::flush;
            		sqlite3_finalize(stmt);
					return CAST_ERROR;
				}
                res.push_back(utf8_decode(casted_col_text));
            }
        }
    }
    // Прочитать столбец
    else {
        while (step_return_code == SQLITE_ROW) {
			const char *casted_col_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
			if (!casted_col_text) {
				std::wcerr << L"Ошибка reinterpret_cast\n" << std::flush;
            	sqlite3_finalize(stmt);
				return CAST_ERROR;
			}
            res.push_back(utf8_decode(casted_col_text));
            step_return_code = sqlite3_step(stmt);
        }
    }

    // TODO: исправить костыль step_return_code != SQLITE_ROW
    if (step_return_code != SQLITE_DONE && step_return_code != SQLITE_ROW) {
        std::wcerr << L"Ошибка при выполнении запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        sqlite3_finalize(stmt);
        return EXEC_ERROR;
    }

    sqlite3_finalize(stmt);
    return (res.empty()) ? EMPTY : OK;
}

// Прочитать все колонки во всех записях
Database::StatusCode Database::exec(std::vector<std::vector<std::wstring>> &res, const char *query, const std::initializer_list<std::wstring> binds) const {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(p_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::wcerr << L"Ошибка при подготовке запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        return PREPARE_ERROR;
    }

    {int i = 0;
    for (const std::wstring &bind : binds) {
        const std::string conv_bind = utf8_encode(bind);
        if (sqlite3_bind_text(stmt, i + 1, conv_bind.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            std::wcerr << L"Ошибка при подстановке переменных в запрос: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
            sqlite3_finalize(stmt);
            return BIND_ERROR;
        }
        i++;
    }}

    int step_return_code = sqlite3_step(stmt);
    while (step_return_code == SQLITE_ROW) {
        int col_amount = sqlite3_column_count(stmt);
        res.push_back({});
        for (int i = 0; i < col_amount; i++) {
            std::vector<std::wstring> &line = res.back();
			const char *casted_col_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
			if (!casted_col_text) {
				std::wcerr << L"Ошибка reinterpret_cast\n" << std::flush;
            	sqlite3_finalize(stmt);
				return CAST_ERROR;
			}
            line.push_back(utf8_decode(casted_col_text));
        }
        step_return_code = sqlite3_step(stmt);
    }
    if (step_return_code != SQLITE_DONE) {
        std::wcerr << L"Ошибка при выполнении запроса: " << sqlite3_errmsg(p_db) << '\n' << std::flush;
        sqlite3_finalize(stmt);
        return EXEC_ERROR;
    }

    sqlite3_finalize(stmt);
    return (res.empty()) ? EMPTY : OK;
}
