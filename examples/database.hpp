/* -------------------- Пример класса для работы с БД с защитой от SQL-инъекций ------------------------- */

/* WARNING:
 * Код написан не самым оптимальным образом:
 * стоит разбить подготовку запроса, подстановку в запрос и т.д. на отдельные функции,
 * чтобы избавиться от повторений кода в перегрузках.
 *
 * Возможно, есть более оптимальный способ реализации
 * переменного количества параметров функции, чем std::initializer_list
 */

#pragma once

#ifdef _WIN32
#include "sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include <string>
#include <vector>
#include <initializer_list>

class Database {
private:
    sqlite3 *p_db;

public:
    // Перечисление статуса выполнения функций
    enum StatusCode {
        OK,
        EMPTY,
        PREPARE_ERROR,
        BIND_ERROR,
		CAST_ERROR,
        EXEC_ERROR
    };

    Database(); // Конструктор
    Database(const char *filename); // Конструктор
    ~Database(); // Деструктор

    // Перегруженная функция
    StatusCode exec(const char *query, const std::initializer_list<std::wstring> binds = {});
    StatusCode exec(std::wstring &res, const char *query, const std::initializer_list<std::wstring> binds = {}) const;
    StatusCode exec(std::vector<std::wstring> &res, const char *query, const std::initializer_list<std::wstring> binds = {}, const bool read_line = true) const;
    StatusCode exec(std::vector<std::vector<std::wstring>> &res, const char *query, const std::initializer_list<std::wstring> binds = {}) const;
};
