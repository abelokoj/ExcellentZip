// Exercise real registration beneath an isolated registry root; never change Windows default choices.
#include "ExcellentZip.DefaultApps.h"
#include "ExcellentZip.IconCatalog.h"
#include <Shellapi.h>
#include <cassert>
#include <iostream>
#include <string>

static std::wstring OpenedUri;
static bool LaunchSucceeds = true;
static BOOL WINAPI CaptureSettings(SHELLEXECUTEINFOW* Info)
{
    OpenedUri = Info->lpFile;
    return LaunchSucceeds;
}
#define ShellExecuteExW CaptureSettings
#include "../UI_Modern/ExcellentZip.DefaultApps.cpp"
#undef ShellExecuteExW

static std::wstring Read(HKEY Root, const std::wstring& Path, const wchar_t* Name)
{
    wchar_t Value[32768]{};
    DWORD Bytes = sizeof(Value);
    assert(::RegGetValueW(Root, Path.c_str(), Name, RRF_RT_REG_SZ,
        nullptr, Value, &Bytes) == ERROR_SUCCESS);
    return Value;
}

int wmain()
{
    using namespace ExcellentZipDefaults;
    assert(KnownExtension(L".ZIP") && std::wstring(KnownExtension(L".ZIP")) == L"zip");
    for (const auto* Extension : ExcellentZipIcons::kExtensions)
        assert(SettingsUri(Extension) == L"ms-settings:defaultapps?fileExt=." + std::wstring(Extension));
    for (const auto* Invalid : {L"exe", L"zip&evil=1", L"..\\zip", L"", L"z!p"})
    {
        assert(!KnownExtension(Invalid));
        assert(SettingsUri(Invalid).empty());
    }
    assert(SettingsUri().find(L"ms-settings:defaultapps?registeredApp") == 0);
    GUID Id{};
    wchar_t Guid[40]{};
    assert(SUCCEEDED(::CoCreateGuid(&Id)));
    assert(::StringFromGUID2(Id, Guid, ARRAYSIZE(Guid)));
    const auto TestPath = L"Software\\ExcellentZip.Tests\\Defaults-" + std::wstring(Guid);
    HKEY Root = nullptr;
    assert(::RegCreateKeyExW(HKEY_CURRENT_USER, TestPath.c_str(), 0, nullptr,
        0, KEY_ALL_ACCESS, nullptr, &Root, nullptr) == ERROR_SUCCESS);
    assert(::RegOverridePredefKey(HKEY_CURRENT_USER, Root) == ERROR_SUCCESS);
    const auto Sentinels = {
        L"Software\\Classes\\.zip",
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.zip\\UserChoice"};
    for (const auto* Path : Sentinels)
        assert(SetString(Path, nullptr, L"OtherApp") == ERROR_SUCCESS);
    std::wstring Error;
    assert(!Open(nullptr, L"zip&evil=1", Error) && !Error.empty() && OpenedUri.empty());
    assert(Open(nullptr, L".ZIP", Error) && Error.empty());
    assert(OpenedUri == L"ms-settings:defaultapps?fileExt=.zip");
    assert(Read(Root, L"Software\\RegisteredApplications", L"ExcellentZip") == L"Software\\ExcellentZip\\Capabilities");
    assert(Read(Root, L"Software\\Classes\\Applications\\ExcellentZip.exe", L"FriendlyAppName") == L"ExcellentZip");
    for (const auto* Extension : ExcellentZipIcons::kExtensions)
    {
        const auto Suffix = L"." + std::wstring(Extension);
        const auto ProgId = L"ExcellentZip.Archive." + std::wstring(Extension);
        assert(Read(Root, L"Software\\ExcellentZip\\Capabilities\\FileAssociations", Suffix.c_str()) == ProgId);
        assert(Read(Root, L"Software\\Classes\\" + ProgId + L"\\shell\\open\\command", nullptr)
            .ends_with(L"ExcellentZip.exe\" \"%1\" -open"));
        assert(Read(Root, L"Software\\Classes\\" + ProgId + L"\\DefaultIcon", nullptr)
            .find(L"icons\\4-monogram-ez\\" + std::wstring(Extension) + L".ico") != std::wstring::npos);
    }
    assert(Open(nullptr, nullptr, Error));
    assert(OpenedUri == SettingsUri());
    LaunchSucceeds = false;
    assert(!Open(nullptr, L"rar", Error) && !Error.empty());
    for (const auto* Path : Sentinels)
        assert(Read(Root, Path, nullptr) == L"OtherApp");
    assert(::RegOverridePredefKey(HKEY_CURRENT_USER, nullptr) == ERROR_SUCCESS);
    ::RegCloseKey(Root);
    assert(TestPath.starts_with(L"Software\\ExcellentZip.Tests\\Defaults-{"));
    assert(::RegDeleteTreeW(HKEY_CURRENT_USER, TestPath.c_str()) == ERROR_SUCCESS);
    std::cout << "Default-app tests: 62 registrations, exact name, safe URLs, launch errors and existing default preservation passed.\n";
}
