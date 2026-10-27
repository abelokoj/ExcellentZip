#pragma once

#include <Windows.h>
#include <string>

namespace ExcellentZipIcons
{
    int SelectedPack();
    bool OwnsNativeAssociation(const wchar_t* ProgId, const wchar_t* Extension,
        const wchar_t* Command, const wchar_t* Executable);
    std::wstring IconPath(int Pack, const wchar_t* Extension);
    bool ApplyPack(int Pack, std::wstring& Error);
    void UpdateWindowIcon(HWND Window);
    void RefreshLegacyAssociations();
}
