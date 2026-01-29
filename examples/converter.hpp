/* ----- Конвертация std::string в std::wstring и наоборот (только для Windows) ----- */

#ifdef _WIN32
#pragma once

#include <string>

std::wstring conv_char_to_wstring(const char *text); // Конвертирует строки C-стиля в std::wstring
std::string conv_wstring_to_string(const std::wstring &wstr); // Конвертирует std::wstring в std::string
#endif
