#pragma once
#include <Windows.h>
#include <string>

namespace ExcellentZipDefaults
{
    const wchar_t* KnownExtension(const wchar_t* Extension);
    std::wstring SettingsUri(const wchar_t* Extension = nullptr);
    bool Register(std::wstring& Error);
    bool Open(HWND Owner, const wchar_t* Extension, std::wstring& Error);
    bool IsDefault(const wchar_t* Extension);
    std::wstring CurrentApp(const wchar_t* Extension);
}
