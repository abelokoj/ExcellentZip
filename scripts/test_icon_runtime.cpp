// Standalone, non-mutating checks for ExcellentZip's native icon boundary.
#include "ExcellentZip.IconHandler.h"
#include "ExcellentZip.IconCatalog.h"
#include <cassert>
#include <iostream>

int wmain()
{
    using namespace ExcellentZipIcons;
    static_assert(kDefaultPack == 4);
    static_assert(ARRAYSIZE(kPackNames) == 7 && ARRAYSIZE(kPackFolders) == 7);
    static_assert(ARRAYSIZE(kExtensions) == 62);
    assert(SelectedPack() >= 1 && SelectedPack() <= 7);
    for (int Pack = 1; Pack <= 7; ++Pack)
    {
        for (const auto* Extension : kExtensions)
        {
            auto Path = IconPath(Pack, Extension);
            assert(Path.find(std::wstring(L"IconPacks\\") + kPackFolders[Pack - 1]) != std::wstring::npos);
            assert(Path.ends_with(std::wstring(L"\\") + Extension + L".ico"));
        }
    }
    assert(IconPath(0, L"zip") == IconPath(4, L"zip"));
    assert(IconPath(8, L"zip") == IconPath(4, L"zip"));
    assert(IconPath(4, L"ZIP") == IconPath(4, L"zip"));
    assert(IconPath(4, L"..\\other-app") == IconPath(4, L"archive"));
    assert(IconPath(4, L"zip\\evil") == IconPath(4, L"archive"));
    const wchar_t* Executable = LR"(C:\Apps\ExcellentZip\ExcellentZip.exe)";
    const wchar_t* Command = LR"("C:\Apps\ExcellentZip\ExcellentZip.exe" "%1" -open)";
    assert(OwnsNativeAssociation(L"ExcellentZip.Archive.zip", L"zip", Command, Executable));
    assert(OwnsNativeAssociation(L"excellentzip.archive.ZIP", L"ZIP", Command, Executable));
    assert(!OwnsNativeAssociation(L"OtherApp.zip", L"zip", Command, Executable));
    assert(!OwnsNativeAssociation(L"ExcellentZip.Archive.rar", L"zip", Command, Executable));
    const wchar_t* WrongCommand = LR"("C:\Other\ExcellentZip.exe" "%1" -open)";
    assert(!OwnsNativeAssociation(L"ExcellentZip.Archive.zip", L"zip", WrongCommand, Executable));
    const wchar_t* ExtraCommand = LR"("C:\Apps\ExcellentZip\ExcellentZip.exe" "%1" -open -extra)";
    assert(!OwnsNativeAssociation(L"ExcellentZip.Archive.zip", L"zip", ExtraCommand, Executable));
    assert(!OwnsNativeAssociation(L"ExcellentZip.Archive.zip", L"..\\zip", Command, Executable));
    assert(!OwnsNativeAssociation(nullptr, L"zip", Command, Executable));
    std::wstring Error;
    assert(!ApplyPack(0, Error) && !Error.empty());
    assert(!ApplyPack(8, Error) && !Error.empty());
    auto Handler = winrt::make<IconHandler>();
    assert(Handler->Load(nullptr, 0) == E_POINTER);
    assert(Handler->Load(L"C:\\not-opened\\archive.ZIP", STGM_READ) == S_OK);
    auto Extractor = Handler.as<IExtractIconW>();
    wchar_t Path[32768] = {};
    int Index = -1;
    UINT Flags = 0;
    assert(Extractor->GetIconLocation(0, Path, ARRAYSIZE(Path), &Index, &Flags) == S_OK);
    assert(std::wstring(Path).ends_with(L"\\zip.ico"));
    assert(Index == 0 && (Flags & GIL_DONTCACHE));
    assert(Extractor->GetIconLocation(0, Path, 2, &Index, &Flags) == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    assert(Extractor->GetIconLocation(0, nullptr, 0, &Index, &Flags) == E_POINTER);
    assert(Handler->Load(L"C:\\not-opened\\unknown.type", 0) == S_OK);
    assert(Extractor->GetIconLocation(0, Path, ARRAYSIZE(Path), &Index, &Flags) == S_OK);
    assert(std::wstring(Path).ends_with(L"\\archive.ico"));
    void* Object = reinterpret_cast<void*>(1);
    auto Factory = winrt::make<IconFactory>();
    assert(Factory->CreateInstance(reinterpret_cast<IUnknown*>(1), IID_IPersistFile, &Object) == CLASS_E_NOAGGREGATION);
    assert(Object == nullptr);
    assert(Factory->CreateInstance(nullptr, IID_IPersistFile, nullptr) == E_POINTER);
    std::cout << "Native icon tests passed: seven packs, 62 types, defaults, traversal, COM, and buffer checks.\n";
}
