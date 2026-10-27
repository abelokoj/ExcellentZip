// Optional icon downloads. New code is licensed under the MIT License.
#include "ExcellentZip.Icons.h"
#include "ExcellentZip.IconCatalog.h"
#include "ExcellentZip.IconDownloadCatalog.h"

#include <CommCtrl.h>
#include <WinHttp.h>
#include <bcrypt.h>
#include <winrt/base.h>
#include <array>
#include <atomic>
#include <filesystem>
#include <thread>

#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Ole32.lib")

namespace ExcellentZipIcons
{
    namespace
    {
        struct InternetHandle
        {
            HINTERNET Value = nullptr;
            ~InternetHandle() { if (Value) ::WinHttpCloseHandle(Value); }
        };

        struct HashHandle
        {
            BCRYPT_HASH_HANDLE Value = nullptr;
            ~HashHandle() { if (Value) ::BCryptDestroyHash(Value); }
        };

        struct DownloadState
        {
            std::atomic<bool> Cancelled{false}, Done{false};
            std::atomic<unsigned long long> Received{0};
            std::atomic<int> Phase{0};
            bool Succeeded = false;
            std::wstring Error;
            std::thread Worker;
        };

        std::wstring UniqueName()
        {
            GUID Id{};
            wchar_t Text[40]{};
            if (FAILED(::CoCreateGuid(&Id)) || !::StringFromGUID2(Id, Text, ARRAYSIZE(Text)))
                return {};
            return Text;
        }

        bool FileMatches(const std::wstring& Path, unsigned long long Bytes,
            const wchar_t* Digest, const std::atomic<bool>& Cancelled)
        {
            HANDLE Raw = ::CreateFileW(Path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
            if (Raw == INVALID_HANDLE_VALUE)
                return false;
            winrt::handle File{Raw};
            LARGE_INTEGER Size{};
            if (!::GetFileSizeEx(File.get(), &Size) ||
                Size.QuadPart < 0 || static_cast<unsigned long long>(Size.QuadPart) != Bytes)
                return false;
            HashHandle Hash;
            if (::BCryptCreateHash(BCRYPT_SHA256_ALG_HANDLE, &Hash.Value,
                nullptr, 0, nullptr, 0, 0) < 0)
                return false;
            std::array<BYTE, 65536> Buffer{};
            DWORD Read = 0;
            for (;;)
            {
                if (Cancelled || !::ReadFile(File.get(), Buffer.data(),
                    static_cast<DWORD>(Buffer.size()), &Read, nullptr))
                    return false;
                if (!Read)
                    break;
                if (::BCryptHashData(Hash.Value, Buffer.data(), Read, 0) < 0)
                    return false;
            }
            std::array<BYTE, 32> Result{};
            if (::BCryptFinishHash(Hash.Value, Result.data(),
                static_cast<ULONG>(Result.size()), 0) < 0)
                return false;
            constexpr wchar_t Hex[] = L"0123456789abcdef";
            std::wstring Actual;
            Actual.reserve(64);
            for (BYTE Value : Result)
            {
                Actual += Hex[Value >> 4];
                Actual += Hex[Value & 15];
            }
            return Actual == Digest;
        }

        bool VerifyOptionalFiles(const std::wstring& Directory,
            const std::atomic<bool>& Cancelled, std::wstring& Error)
        {
            for (const auto& File : kOptionalFiles)
            {
                const auto Path = Directory + L"\\" + kPackFolders[File.Pack - 1] +
                    L"\\" + File.Stem + L".ico";
                if (!FileMatches(Path, File.Bytes, File.Sha256, Cancelled))
                {
                    Error = L"The downloaded icons are incomplete or could not be verified. Try downloading them again.";
                    return false;
                }
            }
            return true;
        }

        bool ExtractArchive(const std::wstring& Archive, const std::wstring& Output,
            const std::wstring& Engine, DownloadState& State)
        {
            std::wstring Command = L"\"" + Engine + L"\" x -y -bd -bb0 -bso0 -bse0 -bsp0 -o\"" +
                Output + L"\" \"" + Archive + L"\"";
            STARTUPINFOW Startup{};
            Startup.cb = sizeof(Startup);
            PROCESS_INFORMATION Process{};
            if (!::CreateProcessW(Engine.c_str(), Command.data(), nullptr, nullptr,
                FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &Startup, &Process))
            {
                State.Error = L"ExcellentZip could not start its archive engine. Repair or reinstall the app.";
                return false;
            }
            winrt::handle Child{Process.hProcess}, Thread{Process.hThread};
            const ULONGLONG Started = ::GetTickCount64();
            for (;;)
            {
                const DWORD Wait = ::WaitForSingleObject(Child.get(), 100);
                if (Wait == WAIT_OBJECT_0)
                    break;
                if (Wait == WAIT_FAILED || State.Cancelled ||
                    ::GetTickCount64() - Started > 180000)
                {
                    ::TerminateProcess(Child.get(), ERROR_CANCELLED);
                    ::WaitForSingleObject(Child.get(), 5000);
                    State.Error = L"The icon download could not be unpacked. Try again.";
                    return false;
                }
            }
            DWORD ExitCode = 1;
            if (!::GetExitCodeProcess(Child.get(), &ExitCode) || ExitCode != 0)
            {
                State.Error = L"ExcellentZip could not unpack the downloaded icons. Try again.";
                return false;
            }
            return true;
        }

        bool InstallVerifiedArchive(const std::wstring& Archive,
            const std::wstring& Staging, const std::wstring& Destination,
            const std::wstring& Engine, DownloadState& State)
        {
            if (State.Cancelled)
                return false;
            State.Phase = 1;
            // Keep the verified archive locked against changes until extraction finishes.
            HANDLE Raw = ::CreateFileW(Archive.c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
            if (Raw == INVALID_HANDLE_VALUE)
            {
                State.Error = L"Windows could not read the downloaded icons.";
                return false;
            }
            winrt::handle ArchiveLock{Raw};
            if (!FileMatches(Archive, kOptionalArchiveBytes, kOptionalArchiveSha256, State.Cancelled))
            {
                State.Error = L"The icon download could not be verified. Nothing was installed. Try again later.";
                return false;
            }
            State.Phase = 2;
            if (!ExtractArchive(Archive, Staging, Engine, State) ||
                !VerifyOptionalFiles(Staging, State.Cancelled, State.Error) || State.Cancelled)
                return false;
            State.Phase = 3;
            std::error_code Code;
            const auto Backup = Destination + L".previous-" + UniqueName();
            const bool HadPrevious = std::filesystem::exists(Destination, Code);
            if (Code)
            {
                State.Error = L"Windows could not access the icon folder. Check your account permissions.";
                return false;
            }
            if (HadPrevious)
            {
                std::filesystem::rename(Destination, Backup, Code);
                if (Code)
                {
                    State.Error = L"Windows is using the previous icon files. Close other ExcellentZip windows and try again.";
                    return false;
                }
            }
            std::filesystem::rename(Staging, Destination, Code);
            if (Code)
            {
                if (HadPrevious)
                {
                    std::error_code Restore;
                    std::filesystem::rename(Backup, Destination, Restore);
                    if (Restore)
                    {
                        State.Error = L"Windows could not restore the previous icon folder. Choose the default icons and try again.";
                        return false;
                    }
                }
                State.Error = L"Windows could not install the downloaded icons. Check free disk space and try again.";
                return false;
            }
            if (HadPrevious)
            {
                // Delete only our known files; preserve any unrelated user-added files.
                for (const auto& File : kOptionalFiles)
                {
                    std::filesystem::remove(Backup + L"\\" + kPackFolders[File.Pack - 1] +
                        L"\\" + File.Stem + L".ico", Code);
                }
                for (int Pack = 1; Pack <= kPackCount; ++Pack)
                    if (Pack != kDefaultPack)
                        std::filesystem::remove(Backup + L"\\" + kPackFolders[Pack - 1], Code);
                std::filesystem::remove(Backup, Code);
            }
            return true;
        }

        bool DownloadArchive(const std::wstring& Path, DownloadState& State)
        {
            if (State.Cancelled)
                return false;
            InternetHandle Session{::WinHttpOpen(L"ExcellentZip icon download",
                WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS, 0)};
            if (!Session.Value || !::WinHttpSetTimeouts(Session.Value, 5000, 10000, 10000, 15000))
                return false;
            InternetHandle Connection{::WinHttpConnect(Session.Value,
                L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0)};
            if (!Connection.Value)
                return false;
            const std::wstring Resource = L"/abelokoj/ExcellentZip/main/icons/" +
                std::wstring(kOptionalArchiveName);
            InternetHandle Request{::WinHttpOpenRequest(Connection.Value, L"GET",
                Resource.c_str(), nullptr, WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE)};
            DWORD Redirects = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            if (!Request.Value || !::WinHttpSetOption(Request.Value,
                WINHTTP_OPTION_REDIRECT_POLICY, &Redirects, sizeof(Redirects)) ||
                !::WinHttpSendRequest(Request.Value, L"Accept-Encoding: identity\r\n",
                    static_cast<DWORD>(-1), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
                !::WinHttpReceiveResponse(Request.Value, nullptr))
                return false;
            DWORD Status = 0, Size = sizeof(Status);
            if (!::WinHttpQueryHeaders(Request.Value,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &Status, &Size, WINHTTP_NO_HEADER_INDEX) ||
                Status != HTTP_STATUS_OK)
                return false;
            HANDLE Raw = ::CreateFileW(Path.c_str(), GENERIC_WRITE, 0, nullptr,
                CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
            if (Raw == INVALID_HANDLE_VALUE)
            {
                State.Error = L"Windows could not save the icon download. Check free disk space and try again.";
                return false;
            }
            winrt::handle File{Raw};
            std::array<BYTE, 65536> Buffer{};
            unsigned long long Total = 0;
            for (;;)
            {
                DWORD Read = 0;
                if (State.Cancelled || !::WinHttpReadData(Request.Value, Buffer.data(),
                    static_cast<DWORD>(Buffer.size()), &Read))
                    return false;
                if (!Read)
                    break;
                if (Total + Read > kOptionalArchiveBytes)
                {
                    State.Error = L"The icon download had an unexpected size. Nothing was installed.";
                    return false;
                }
                DWORD Written = 0;
                if (!::WriteFile(File.get(), Buffer.data(), Read, &Written, nullptr) || Written != Read)
                {
                    State.Error = L"Windows could not save the icon download. Check free disk space and try again.";
                    return false;
                }
                Total += Read;
                State.Received = Total;
            }
            if (Total != kOptionalArchiveBytes)
            {
                State.Error = L"The icon download was interrupted. Your current icons have not changed. Try again.";
                return false;
            }
            return true;
        }

        bool DownloadAndInstall(const std::wstring& Destination,
            const std::wstring& Engine, DownloadState& State)
        {
            if (State.Cancelled || Destination.empty())
                return false;
            const auto Parent = std::filesystem::path(Destination).parent_path();
            const auto Id = UniqueName();
            if (Id.empty())
                return false;
            const auto Temporary = Parent / (L".download-" + Id);
            std::error_code Code;
            std::filesystem::create_directories(Parent, Code);
            if (Code || !std::filesystem::create_directory(Temporary, Code) || Code)
            {
                State.Error = L"Windows could not create the icon download folder. Check free disk space and try again.";
                return false;
            }
            bool Success = false;
            try
            {
                const auto Archive = (Temporary / L"optional.zip").wstring();
                if (DownloadArchive(Archive, State))
                    Success = InstallVerifiedArchive(Archive, (Temporary / L"packs").wstring(),
                        Destination, Engine, State);
            }
            catch (...)
            {
                State.Error = L"ExcellentZip could not finish the icon download. Your current icons have not changed.";
            }
            // This is our freshly created, unique staging folder, never the installed cache.
            std::filesystem::remove_all(Temporary, Code);
            if (!Success && State.Error.empty() && !State.Cancelled)
                State.Error = L"Could not reach GitHub. Check your internet connection and try again.";
            return Success;
        }

        HRESULT CALLBACK DownloadDialog(HWND Window, UINT Notice,
            WPARAM, LPARAM, LONG_PTR Context)
        {
            auto& State = *reinterpret_cast<DownloadState*>(Context);
            if (Notice == TDN_CREATED)
            {
                ::SendMessageW(Window, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 100));
                try
                {
                    State.Worker = std::thread([&State]()
                    {
                        try
                        {
                            State.Succeeded = DownloadAndInstall(OptionalCacheDirectory(),
                                AppDirectory() + L"ExcellentZipC.exe", State);
                        }
                        catch (...)
                        {
                            State.Error = L"ExcellentZip could not finish the icon download.";
                        }
                        State.Done = true;
                    });
                }
                catch (...)
                {
                    State.Error = L"ExcellentZip could not start the icon download. Try again.";
                    State.Done = true;
                }
            }
            else if (Notice == TDN_BUTTON_CLICKED)
            {
                if (!State.Done)
                {
                    State.Cancelled = true;
                    return S_FALSE;
                }
            }
            else if (Notice == TDN_TIMER)
            {
                if (State.Done)
                {
                    ::SendMessageW(Window, TDM_CLICK_BUTTON, IDCANCEL, 0);
                }
                else
                {
                    const auto Bytes = State.Received.load();
                    const int Phase = State.Phase.load();
                    const unsigned int Position = Phase == 0
                        ? static_cast<unsigned int>(80 * Bytes / kOptionalArchiveBytes)
                        : (Phase == 1 ? 85 : (Phase == 2 ? 90 : 95));
                    ::SendMessageW(Window, TDM_SET_PROGRESS_BAR_POS, Position, 0);
                    wchar_t Content[256]{};
                    if (State.Cancelled)
                        wcscpy_s(Content, L"Cancelling the download. Your current icons will stay selected.");
                    else if (Phase == 0)
                        swprintf_s(Content, L"Downloading all six additional icon packs from GitHub...\n%.1f MB of %.1f MB",
                            static_cast<double>(Bytes) / 1000000.0,
                            static_cast<double>(kOptionalArchiveBytes) / 1000000.0);
                    else
                        wcscpy_s(Content, Phase == 1 ? L"Checking the downloaded icons..." :
                            (Phase == 2 ? L"Unpacking the additional icon packs..." : L"Installing the icon packs..."));
                    ::SendMessageW(Window, TDM_SET_ELEMENT_TEXT, TDE_CONTENT,
                        reinterpret_cast<LPARAM>(Content));
                }
            }
            return S_OK;
        }
    }

    bool EnsurePackAvailable(HWND Owner, int Pack)
    {
        if (PackAvailable(Pack))
            return true;
        if (Pack < 1 || Pack > kPackCount || Pack == kDefaultPack)
        {
            ::MessageBoxW(Owner, L"The default icons are missing or damaged. Repair or reinstall ExcellentZip.",
                L"ExcellentZip", MB_OK | MB_ICONERROR);
            return false;
        }
        wchar_t Question[512]{};
        swprintf_s(Question, L"This icon pack is not installed.\n\nDownload all six additional icon packs from GitHub (%.1f MB)? "
            L"After downloading, you can use any of them offline.\n\nYour current icons will stay selected if you choose No.",
            static_cast<double>(kOptionalArchiveBytes) / 1000000.0);
        if (::MessageBoxW(Owner, Question, L"Download additional icon packs?",
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
            return false;
        DownloadState State;
        TASKDIALOGCONFIG Dialog{};
        Dialog.cbSize = sizeof(Dialog);
        Dialog.hwndParent = Owner;
        Dialog.dwFlags = TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER |
            TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
        Dialog.dwCommonButtons = TDCBF_CANCEL_BUTTON;
        Dialog.pszWindowTitle = L"ExcellentZip";
        Dialog.pszMainInstruction = L"Downloading additional icon packs";
        Dialog.pszContent = L"Connecting to GitHub...";
        Dialog.pfCallback = DownloadDialog;
        Dialog.lpCallbackData = reinterpret_cast<LONG_PTR>(&State);
        const HRESULT Result = ::TaskDialogIndirect(&Dialog, nullptr, nullptr, nullptr);
        if (FAILED(Result))
            State.Cancelled = true;
        if (State.Worker.joinable())
            State.Worker.join();
        if (State.Cancelled && SUCCEEDED(Result))
            return false;
        if (FAILED(Result) || !State.Succeeded)
        {
            if (State.Error.empty())
                State.Error = L"ExcellentZip could not open the download window. Try again.";
            ::MessageBoxW(Owner, State.Error.c_str(), L"ExcellentZip", MB_OK | MB_ICONERROR);
            return false;
        }
        return PackAvailable(Pack);
    }
}
