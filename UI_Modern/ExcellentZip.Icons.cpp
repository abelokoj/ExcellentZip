// ExcellentZip icon selection. New code is licensed under the MIT License.
#include "ExcellentZip.Icons.h"
#include "ExcellentZip.IconCatalog.h"
#include "ExcellentZip.IconDownloadCatalog.h"

#include <AppModel.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <vector>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")

namespace ExcellentZipIcons
{
    static constexpr const wchar_t* kSettings = L"Software\\ExcellentZip\\Appearance";

    int SelectedPack()
    {
        DWORD Pack = kDefaultPack;
        DWORD Size = sizeof(Pack);
        if (::RegGetValueW(HKEY_CURRENT_USER, kSettings, L"IconPack",
            RRF_RT_REG_DWORD, nullptr, &Pack, &Size) != ERROR_SUCCESS ||
            Pack < 1 || Pack > kPackCount)
        {
            return kDefaultPack;
        }
        const int Result = static_cast<int>(Pack);
        return Result != kDefaultPack && !PackAvailable(Result) ? kDefaultPack : Result;
    }

    std::wstring AppDirectory()
    {
        HMODULE Module = nullptr;
        if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&SelectedPack), &Module))
            return {};
        std::vector<wchar_t> Path(32768);
        DWORD Length = ::GetModuleFileNameW(Module, Path.data(), static_cast<DWORD>(Path.size()));
        if (!Length || Length >= Path.size())
            return {};
        std::wstring Root(Path.data(), Length);
        const size_t Slash = Root.find_last_of(L"\\/");
        if (Slash == std::wstring::npos)
            return {};
        Root.resize(Slash + 1);
        return Root;
    }

    std::wstring OptionalCacheDirectory()
    {
        PWSTR Local = nullptr;
        if (FAILED(::SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &Local)))
            return {};
        std::wstring Root(Local);
        ::CoTaskMemFree(Local);
        return Root + L"\\ExcellentZip\\icons\\" + kOptionalSetId;
    }

    std::wstring IconPath(int Pack, const wchar_t* Extension)
    {
        if (Pack < 1 || Pack > kPackCount)
            Pack = kDefaultPack;
        auto Root = Pack == kDefaultPack ? AppDirectory() : OptionalCacheDirectory();
        if (Root.empty())
            return {};
        if (Pack == kDefaultPack)
            Root += L"icons";
        // Only catalogued stems can become paths, including names from shell input.
        const wchar_t* Stem = L"archive";
        if (Extension && ::_wcsicmp(Extension, L"app") == 0)
            Stem = L"app";
        else if (Extension)
            for (const auto* Known : kExtensions)
                if (::_wcsicmp(Known, Extension) == 0)
                {
                    Stem = Known;
                    break;
                }
        return Root + L"\\" + kPackFolders[Pack - 1] + L"\\" + Stem + L".ico";
    }

    static bool Exists(const std::wstring& Path)
    {
        DWORD Attributes = ::GetFileAttributesW(Path.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES && !(Attributes & FILE_ATTRIBUTE_DIRECTORY);
    }

    static bool ValidatePack(int Pack, std::wstring& Error)
    {
        if (Pack < 1 || Pack > kPackCount)
        {
            Error = L"Choose one of the seven icon packs.";
            return false;
        }
        if (!Exists(IconPath(Pack, L"app")) || !Exists(IconPath(Pack, L"archive")))
        {
            Error = Pack == kDefaultPack ? L"The default icons are incomplete. Repair or reinstall ExcellentZip." :
                L"This extra icon pack is not installed or is incomplete. Select it in Settings to download it.";
            return false;
        }
        for (const auto* Extension : kExtensions)
            if (!Exists(IconPath(Pack, Extension)))
            {
                Error = L"An icon is missing for ." + std::wstring(Extension) +
                    (Pack == kDefaultPack ? L". Repair or reinstall ExcellentZip." : L". Select this pack in Settings to download it again.");
                return false;
            }
        return true;
    }

    bool PackAvailable(int Pack)
    {
        std::wstring Error;
        return ValidatePack(Pack, Error);
    }

    bool OwnsNativeAssociation(const wchar_t* ProgId, const wchar_t* Extension,
        const wchar_t* Command, const wchar_t* Executable)
    {
        if (!ProgId || !Extension || !Command || !Executable || !*Executable)
            return false;
        bool Known = false;
        for (const auto* Item : kExtensions)
            if (::_wcsicmp(Item, Extension) == 0)
            {
                Known = true;
                break;
            }
        if (!Known)
            return false;
        const std::wstring ExpectedId = L"ExcellentZip.Archive." + std::wstring(Extension);
        const std::wstring ExpectedCommand = L"\"" + std::wstring(Executable) + L"\" \"%1\" -open";
        return ::_wcsicmp(ProgId, ExpectedId.c_str()) == 0 &&
            ::_wcsicmp(Command, ExpectedCommand.c_str()) == 0;
    }

    static std::wstring RegistryString(HKEY Root, const std::wstring& Path)
    {
        DWORD Size = 0;
        if (::RegGetValueW(Root, Path.c_str(), nullptr, RRF_RT_REG_SZ,
            nullptr, nullptr, &Size) != ERROR_SUCCESS || Size > 65536 || !Size)
            return {};
        std::vector<wchar_t> Value(Size / sizeof(wchar_t) + 1);
        if (::RegGetValueW(Root, Path.c_str(), nullptr, RRF_RT_REG_SZ,
            nullptr, Value.data(), &Size) != ERROR_SUCCESS)
            return {};
        return Value.data();
    }

    static LONG SetDefaultIcon(const std::wstring& ProgId, const std::wstring& Value, bool& Changed)
    {
        const std::wstring KeyPath = L"Software\\Classes\\" + ProgId + L"\\DefaultIcon";
        if (RegistryString(HKEY_CURRENT_USER, KeyPath) == Value)
            return ERROR_SUCCESS;
        HKEY Key = nullptr;
        LONG Result = ::RegCreateKeyExW(HKEY_CURRENT_USER, KeyPath.c_str(), 0, nullptr,
            0, KEY_SET_VALUE, nullptr, &Key, nullptr);
        if (Result == ERROR_SUCCESS)
        {
            Result = ::RegSetValueExW(Key, nullptr, 0, REG_SZ,
                reinterpret_cast<const BYTE*>(Value.c_str()),
                static_cast<DWORD>((Value.size() + 1) * sizeof(wchar_t)));
            ::RegCloseKey(Key);
            if (Result == ERROR_SUCCESS)
                Changed = true;
        }
        return Result;
    }

    static LONG RefreshAssociationIcons(int Pack, bool& Changed)
    {
        std::vector<wchar_t> Executable(32768);
        const DWORD ExecutableLength = ::GetModuleFileNameW(nullptr,
            Executable.data(), static_cast<DWORD>(Executable.size()));
        if (!ExecutableLength || ExecutableLength >= Executable.size())
            return ERROR_BAD_PATHNAME;
        wchar_t ApplicationId[512] = {};
        UINT32 Length = ARRAYSIZE(ApplicationId);
        const bool Packaged = ::GetCurrentApplicationUserModelId(&Length,
            ApplicationId) == ERROR_SUCCESS;
        for (const auto* Extension : kExtensions)
        {
            const std::wstring Value = L"\"" + IconPath(Pack, Extension) + L"\",0";
            const std::wstring NativeId = L"ExcellentZip.Archive." + std::wstring(Extension);
            const auto Command = RegistryString(HKEY_CLASSES_ROOT,
                NativeId + L"\\shell\\open\\command");
            // Update only our registered class; this never chooses the default app.
            if (OwnsNativeAssociation(NativeId.c_str(), Extension,
                Command.c_str(), Executable.data()))
            {
                const LONG Result = SetDefaultIcon(NativeId, Value, Changed);
                if (Result != ERROR_SUCCESS)
                    return Result;
            }
            if (!Packaged)
                continue;
            const std::wstring Suffix = L"." + std::wstring(Extension);
            wchar_t Owner[512] = {};
            DWORD OwnerLength = ARRAYSIZE(Owner);
            if (FAILED(::AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_APPID,
                Suffix.c_str(), nullptr, Owner, &OwnerLength)) ||
                ::_wcsicmp(Owner, ApplicationId) != 0)
                continue;
            wchar_t ProgId[512] = {};
            DWORD ProgIdLength = ARRAYSIZE(ProgId);
            if (FAILED(::AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_PROGID,
                Suffix.c_str(), nullptr, ProgId, &ProgIdLength)) ||
                ::wcschr(ProgId, L'\\') || ::wcschr(ProgId, L'/') || !ProgId[0])
                continue;
            const LONG Result = SetDefaultIcon(ProgId, Value, Changed);
            if (Result != ERROR_SUCCESS)
                return Result;
        }
        return ERROR_SUCCESS;
    }

    void RefreshLegacyAssociations()
    {
        std::wstring Error;
        const int Pack = SelectedPack();
        if (!ValidatePack(Pack, Error))
            return;
        bool Changed = false;
        RefreshAssociationIcons(Pack, Changed);
        if (Changed)
            ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }

    bool ApplyPack(int Pack, std::wstring& Error)
    {
        Error.clear();
        if (!ValidatePack(Pack, Error))
            return false;
        const int PreviousPack = SelectedPack();
        HKEY Key = nullptr;
        LONG Result = ::RegCreateKeyExW(HKEY_CURRENT_USER, kSettings, 0, nullptr,
            0, KEY_SET_VALUE, nullptr, &Key, nullptr);
        if (Result == ERROR_SUCCESS)
        {
            const DWORD Value = static_cast<DWORD>(Pack);
            Result = ::RegSetValueExW(Key, L"IconPack", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&Value), sizeof(Value));
            ::RegCloseKey(Key);
        }
        if (Result != ERROR_SUCCESS)
        {
            Error = L"ExcellentZip could not save the icon pack. Check your Windows account permissions.";
            return false;
        }
        bool Changed = false;
        Result = RefreshAssociationIcons(Pack, Changed);
        if (Result != ERROR_SUCCESS)
        {
            // shortcut: icon writes are per value; use a transaction if cross-type atomicity becomes required.
            HKEY PreviousKey = nullptr;
            if (::RegOpenKeyExW(HKEY_CURRENT_USER, kSettings, 0, KEY_SET_VALUE,
                &PreviousKey) == ERROR_SUCCESS)
            {
                const DWORD PreviousValue = static_cast<DWORD>(PreviousPack);
                ::RegSetValueExW(PreviousKey, L"IconPack", 0, REG_DWORD,
                    reinterpret_cast<const BYTE*>(&PreviousValue), sizeof(PreviousValue));
                ::RegCloseKey(PreviousKey);
            }
            bool Restored = false;
            RefreshAssociationIcons(PreviousPack, Restored);
            ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
            Error = L"Windows blocked an ExcellentZip file-icon update. Restoring the previous pack was attempted; "
                L"check account permissions and try Apply again. Some Explorer icons may need a refresh.";
            return false;
        }
        if (Changed || Pack != PreviousPack)
            ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        return true;
    }

    void UpdateWindowIcon(HWND Window)
    {
        struct WindowIcons
        {
            HICON Small = nullptr;
            HICON Large = nullptr;
            ~WindowIcons()
            {
                if (Small) ::DestroyIcon(Small);
                if (Large) ::DestroyIcon(Large);
            }
        };
        static WindowIcons Icons;
        const auto Path = IconPath(SelectedPack(), L"app");
        HICON Small = static_cast<HICON>(::LoadImageW(nullptr, Path.c_str(), IMAGE_ICON,
            ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
        HICON Large = static_cast<HICON>(::LoadImageW(nullptr, Path.c_str(), IMAGE_ICON,
            ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE));
        if (!Small || !Large)
        {
            if (Small) ::DestroyIcon(Small);
            if (Large) ::DestroyIcon(Large);
            return;
        }
        ::SendMessageW(Window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Small));
        ::SendMessageW(Window, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Large));
        if (Icons.Small) ::DestroyIcon(Icons.Small);
        if (Icons.Large) ::DestroyIcon(Icons.Large);
        Icons.Small = Small;
        Icons.Large = Large;
    }
}
