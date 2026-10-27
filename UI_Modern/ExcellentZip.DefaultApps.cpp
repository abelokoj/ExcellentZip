// Windows owns default-opener confirmation; this code only registers capabilities and opens Settings.
#include "ExcellentZip.DefaultApps.h"
#include "ExcellentZip.Icons.h"
#include "ExcellentZip.IconCatalog.h"
#include <AppModel.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Shellapi.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Advapi32.lib")

namespace ExcellentZipDefaults
{
    namespace
    {
        std::wstring ApplicationId()
        {
            wchar_t Value[512]{};
            UINT32 Size = ARRAYSIZE(Value);
            return ::GetCurrentApplicationUserModelId(&Size, Value) == ERROR_SUCCESS
                ? std::wstring(Value) : std::wstring();
        }

        LONG SetString(const std::wstring& Path, const wchar_t* Name, const std::wstring& Value)
        {
            HKEY Key = nullptr;
            LONG Result = ::RegCreateKeyExW(HKEY_CURRENT_USER, Path.c_str(), 0,
                nullptr, 0, KEY_SET_VALUE, nullptr, &Key, nullptr);
            if (Result == ERROR_SUCCESS)
            {
                Result = ::RegSetValueExW(Key, Name, 0, REG_SZ,
                    reinterpret_cast<const BYTE*>(Value.c_str()),
                    static_cast<DWORD>((Value.size() + 1) * sizeof(wchar_t)));
                ::RegCloseKey(Key);
            }
            return Result;
        }

        std::wstring Query(const wchar_t* Extension, ASSOCSTR Property)
        {
            const auto* Known = KnownExtension(Extension);
            if (!Known)
                return {};
            const auto Suffix = L"." + std::wstring(Known);
            wchar_t Value[32768]{};
            DWORD Size = ARRAYSIZE(Value);
            if (FAILED(::AssocQueryStringW(ASSOCF_NONE, Property,
                Suffix.c_str(), nullptr, Value, &Size)))
                return {};
            return Value;
        }
    }

    const wchar_t* KnownExtension(const wchar_t* Extension)
    {
        if (!Extension)
            return nullptr;
        if (*Extension == L'.')
            ++Extension;
        for (const auto* Known : ExcellentZipIcons::kExtensions)
            if (::_wcsicmp(Known, Extension) == 0)
                return Known;
        return nullptr;
    }

    std::wstring SettingsUri(const wchar_t* Extension)
    {
        if (Extension)
        {
            const auto* Known = KnownExtension(Extension);
            if (!Known)
                return {};
            // Older Windows versions may open the parent page; the extension is also shown in our guide.
            return L"ms-settings:defaultapps?fileExt=." + std::wstring(Known);
        }
        const auto Id = ApplicationId();
        if (!Id.empty())
        {
            wchar_t Escaped[4096]{};
            DWORD Size = ARRAYSIZE(Escaped);
            if (SUCCEEDED(::UrlEscapeW(Id.c_str(), Escaped, &Size,
                URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8)))
                return L"ms-settings:defaultapps?registeredAUMID=" + std::wstring(Escaped);
        }
        return L"ms-settings:defaultapps?registeredAppUser=ExcellentZip";
    }

    bool Register(std::wstring& Error)
    {
        Error.clear();
        if (!ApplicationId().empty())
            return true; // Packaged apps already declare their associations in the manifest.
        const auto Directory = ExcellentZipIcons::AppDirectory();
        if (Directory.empty())
        {
            Error = L"ExcellentZip could not find its application folder. Repair or reinstall the app.";
            return false;
        }
        const auto Executable = Directory + L"ExcellentZip.exe";
        const auto Command = L"\"" + Executable + L"\" \"%1\" -open";
        const std::wstring Capabilities = L"Software\\ExcellentZip\\Capabilities";
        const int Pack = ExcellentZipIcons::SelectedPack();
        LONG Result = ERROR_SUCCESS;
        const auto Write = [&](const std::wstring& Path, const wchar_t* Name,
            const std::wstring& Value)
        {
            if (Result == ERROR_SUCCESS)
                Result = SetString(Path, Name, Value);
        };
        Write(Capabilities, L"ApplicationName", L"ExcellentZip");
        Write(Capabilities, L"ApplicationDescription", L"Open, create and extract archives with ExcellentZip.");
        Write(Capabilities, L"ApplicationIcon", L"\"" + Executable + L"\",0");
        Write(L"Software\\Classes\\Applications\\ExcellentZip.exe", L"FriendlyAppName", L"ExcellentZip");
        for (const auto* Extension : ExcellentZipIcons::kExtensions)
        {
            const auto Suffix = L"." + std::wstring(Extension);
            const auto ProgId = L"ExcellentZip.Archive." + std::wstring(Extension);
            const auto Class = L"Software\\Classes\\" + ProgId;
            Write(Class, nullptr, L"ExcellentZip " + std::wstring(Extension) + L" archive");
            Write(Class + L"\\shell\\open\\command", nullptr, Command);
            Write(Class + L"\\DefaultIcon", nullptr, L"\"" +
                ExcellentZipIcons::IconPath(Pack, Extension) + L"\",0");
            Write(L"Software\\Classes\\" + Suffix + L"\\OpenWithProgids", ProgId.c_str(), L"");
            Write(Capabilities + L"\\FileAssociations", Suffix.c_str(), ProgId);
        }
        // Publish the capability entry only after its file classes are ready.
        Write(L"Software\\RegisteredApplications", L"ExcellentZip", Capabilities);
        if (Result != ERROR_SUCCESS)
        {
            Error = L"Windows could not register ExcellentZip's file types. Check your account permissions or run Setup again.";
            return false;
        }
        ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        return true;
    }

    bool Open(HWND Owner, const wchar_t* Extension, std::wstring& Error)
    {
        const auto Uri = SettingsUri(Extension);
        if (Uri.empty())
        {
            Error = L"Choose a supported file type.";
            return false;
        }
        if (!Register(Error))
            return false;
        SHELLEXECUTEINFOW Info{};
        Info.cbSize = sizeof(Info);
        Info.fMask = SEE_MASK_FLAG_NO_UI;
        Info.hwnd = Owner;
        Info.lpVerb = L"open";
        Info.lpFile = Uri.c_str();
        Info.nShow = SW_SHOWNORMAL;
        if (!::ShellExecuteExW(&Info))
        {
            Error = L"Windows Settings could not open. Open Settings > Apps > Default apps and choose ExcellentZip.";
            return false;
        }
        return true;
    }

    bool IsDefault(const wchar_t* Extension)
    {
        const auto* Known = KnownExtension(Extension);
        if (!Known)
            return false;
        const auto ProgId = Query(Known, ASSOCSTR_PROGID);
        const auto Expected = L"ExcellentZip.Archive." + std::wstring(Known);
        if (::_wcsicmp(ProgId.c_str(), Expected.c_str()) == 0)
            return true;
        const auto Id = ApplicationId();
        return !Id.empty() && ::_wcsicmp(Query(Known, ASSOCSTR_APPID).c_str(), Id.c_str()) == 0;
    }

    std::wstring CurrentApp(const wchar_t* Extension)
    {
        if (IsDefault(Extension))
            return L"ExcellentZip";
        const auto Name = Query(Extension, ASSOCSTR_FRIENDLYAPPNAME);
        return Name.empty() ? L"Not set" : Name;
    }
}
