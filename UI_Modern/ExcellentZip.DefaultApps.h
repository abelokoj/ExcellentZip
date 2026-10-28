#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace ExcellentZipDefaults
{
    const wchar_t* KnownExtension(const wchar_t* Extension);
    std::wstring SettingsUri(const wchar_t* Extension = nullptr);
    bool ParseExtensions(const std::wstring& Text, std::vector<const wchar_t*>& Extensions, std::wstring& Error);
    bool Register(const std::vector<const wchar_t*>& Extensions, std::wstring& Error);
    bool Register(std::wstring& Error);
    // One-shot selection supplied by Setup; consumed when Options is opened.
    inline std::wstring PendingSelection;
    bool Open(HWND Owner, const wchar_t* Extension, std::wstring& Error);
    bool IsDefault(const wchar_t* Extension);
    std::wstring CurrentApp(const wchar_t* Extension);
}
