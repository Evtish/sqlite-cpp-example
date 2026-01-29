/* ----- Конвертация std::string в std::wstring и наоборот (только для Windows) ----- */

#ifdef _WIN32
#include <Windows.h>

#include "converter.hpp"

// Источник: https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
// Конвертация широкой Unicode строки в UTF8 строку
std::string utf8_encode(const std::wstring &wstr) {
    if (wstr.empty())
		return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, (int) wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, (int) wstr.size(), strTo, size_needed, NULL, NULL);

    return strTo;
}

// Конвертация UTF8 строки в широкую Unicode строку
std::wstring utf8_decode(const std::string &str) {
    if (str.empty())
		return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, (int) str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, (int) str.size(), wstrTo, size_needed);

    return wstrTo;
}
#endif
