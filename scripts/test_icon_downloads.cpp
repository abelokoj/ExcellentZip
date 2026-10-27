// Offline native checks: no real dialogs, network, user preferences or default-app changes.
#include "ExcellentZip.Icons.h"
#include <CommCtrl.h>
#include <WinHttp.h>
#include <bcrypt.h>
#include <winrt/base.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

static int PromptCount = 0, ProgressCount = 0, NextChoice = IDNO;
static int WINAPI TestMessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT)
{
    ++PromptCount;
    return NextChoice;
}
static HRESULT WINAPI TestTaskDialogIndirect(const TASKDIALOGCONFIG*, int*, int*, BOOL*)
{
    ++ProgressCount;
    return E_FAIL;
}
#define MessageBoxW TestMessageBoxW
#define TaskDialogIndirect TestTaskDialogIndirect
#include "ExcellentZip.IconDownloads.cpp"
#undef MessageBoxW
#undef TaskDialogIndirect

int wmain(int argc, wchar_t** argv)
{
    using namespace ExcellentZipIcons;
    const bool Network = argc == 3 && ::_wcsicmp(argv[2], L"--download") == 0;
    if (argc != 2 && !Network)
        return 2;
    const auto Root = std::filesystem::absolute(argv[1]);
    const auto Archive = Root / L"icons" / kOptionalArchiveName;
    const auto Engine = Root / L"Output/Binaries/Release/x64/ExcellentZipC.exe";
    const auto Fixture = Root / L"Output/OptionalIconTests" / UniqueName();
    std::filesystem::create_directories(Fixture);
    const int Previous = SelectedPack();
    assert(PackAvailable(4));
    assert(EnsurePackAvailable(nullptr, 4));
    assert(PromptCount == 0 && ProgressCount == 0);
    int Missing = 0;
    for (int Pack = 1; Pack <= kPackCount; ++Pack)
        if (Pack != 4 && !PackAvailable(Pack)) { Missing = Pack; break; }
    if (Missing)
    {
        assert(!EnsurePackAvailable(nullptr, Missing));
        assert(PromptCount == 1 && ProgressCount == 0);
        assert(SelectedPack() == Previous);
        NextChoice = IDYES;
        assert(!EnsurePackAvailable(nullptr, Missing)); // Download-window failure keeps selection.
        assert(PromptCount == 3 && ProgressCount == 1);
        assert(SelectedPack() == Previous);
    }
    DownloadState State;
    assert(FileMatches(Archive.wstring(), kOptionalArchiveBytes, kOptionalArchiveSha256, State.Cancelled));
    const auto Damaged = Fixture / L"damaged.zip";
    std::filesystem::copy_file(Archive, Damaged);
    {
        std::fstream File(Damaged, std::ios::in | std::ios::out | std::ios::binary);
        File.seekp(8);
        File.put('\x7f');
    }
    assert(!FileMatches(Damaged.wstring(), kOptionalArchiveBytes, kOptionalArchiveSha256, State.Cancelled));
    const auto Installed = Fixture / L"cache";
    assert(!InstallVerifiedArchive(Damaged.wstring(), (Fixture / L"bad-stage").wstring(),
        Installed.wstring(), Engine.wstring(), State));
    assert(!std::filesystem::exists(Installed));
    State.Error.clear();
    State.Cancelled = true;
    assert(!InstallVerifiedArchive(Archive.wstring(), (Fixture / L"cancel-stage").wstring(),
        Installed.wstring(), Engine.wstring(), State));
    assert(!std::filesystem::exists(Installed));
    State.Cancelled = false;
    if (std::filesystem::is_regular_file(Engine))
    {
        assert(InstallVerifiedArchive(Archive.wstring(), (Fixture / L"good-stage").wstring(),
            Installed.wstring(), Engine.wstring(), State));
        std::wstring Error;
        assert(VerifyOptionalFiles(Installed.wstring(), State.Cancelled, Error));
        assert(!std::filesystem::exists(Installed / L"4-monogram-ez"));
        // Failed repair must leave the verified cache untouched.
        assert(!InstallVerifiedArchive(Damaged.wstring(), (Fixture / L"repair-stage").wstring(),
            Installed.wstring(), Engine.wstring(), State));
        assert(VerifyOptionalFiles(Installed.wstring(), State.Cancelled, Error));
        // A valid repair replaces damaged files without deleting unrelated user additions.
        { std::ofstream Note(Installed / L"user-note.txt"); Note << "Keep this file."; }
        assert(InstallVerifiedArchive(Archive.wstring(), (Fixture / L"repair-good").wstring(),
            Installed.wstring(), Engine.wstring(), State));
        assert(VerifyOptionalFiles(Installed.wstring(), State.Cancelled, Error));
        bool KeptNote = false;
        for (const auto& Entry : std::filesystem::directory_iterator(Fixture))
            if (Entry.is_directory() && Entry.path().filename().wstring().find(L"cache.previous-") == 0)
                KeptNote = std::filesystem::is_regular_file(Entry.path() / L"user-note.txt") || KeptNote;
        assert(KeptNote);
    }
    if (Network)
    {
        assert(std::filesystem::is_regular_file(Engine));
        DownloadState Online;
        const auto Destination = (Fixture / L"online-cache").wstring();
        assert(DownloadAndInstall(Destination, Engine.wstring(), Online));
        std::wstring Error;
        assert(VerifyOptionalFiles(Destination, Online.Cancelled, Error));
        std::wcout << L"Live GitHub download verified: " << Online.Received << L" bytes, all six packs.\n";
        DownloadState Cancel;
        const auto CancelledDestination = (Fixture / L"cancelled-cache").wstring();
        bool CancelledResult = true;
        std::thread Transfer([&] { CancelledResult = DownloadAndInstall(CancelledDestination, Engine.wstring(), Cancel); });
        const auto Started = ::GetTickCount64();
        while (Cancel.Received == 0 && ::GetTickCount64() - Started < 20000)
            ::Sleep(10);
        Cancel.Cancelled = true;
        Transfer.join();
        assert(!CancelledResult && !std::filesystem::exists(CancelledDestination));
        std::wcout << L"Live cancellation left no installed cache.\n";
    }
    assert(SelectedPack() == Previous);
    std::filesystem::remove_all(Fixture);
    std::wcout << L"Native icon tests: consent, cancellation, corruption rejection and offline installation passed.\n";
}
