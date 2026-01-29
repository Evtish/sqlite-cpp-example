/* ----- Конвертация std::string в std::wstring и наоборот (только для Windows) ----- */

#ifdef _WIN32
#include <Windows.h>
#include <cstdlib>

#include <iostream>

#include "converter.hpp"

// https://stackoverflow.com/questions/3074776/how-to-convert-char-array-to-wchar-t-array
// Конвертирует строки C-стиля в std::wstring
int conv_char_to_wstring(std::wstring &wstr, const char *str) {
    if (!str) {
		wstr = L"";
		return 0;
	}

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0); // Размер будущей std::wstring
    if (size_needed == 0) {
        std::wcerr << L"Ошибка при выполнении функции MultiByteToWideChar\n" << std::flush;
		return 1;
    }

    wstr = std::wstring(size_needed - 1, L'\0'); // Заполнение строки нулевыми символами
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, size_needed); // Конвертация
    return 0;
}

// Конвертирует std::wstring в std::string
int conv_wstring_to_string(std::string &str, const std::wstring &wstr) {
	if (wstr.empty()) {
		str = "";
		return 0;
	}

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int) wstr.size(), nullptr, 0, nullptr, nullptr); // Размер будущей std::string
    char *buffer = new char[len]; // Выделение буфера
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int) wstr.size(), buffer, len, nullptr, nullptr); // Конвертация
    std::string str(buffer);

    delete[] buffer; // Очистка буфера
    return str;
}
#endif
