/*
 * PROJECT:    ExcellentZip
 * FILE:       ExcellentZip.UI.h
 * PURPOSE:    Definition for ExcellentZip Modern UI Shared Infrastructures
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#ifndef EXCELLENTZIP_UI
#define EXCELLENTZIP_UI

#include <Mile.Helpers.CppBase.h>
#include <Mile.Helpers.CppWinRT.h>

namespace ExcellentZip::UI
{
    winrt::handle ShowAboutDialog(
        _In_ HWND ParentWindowHandle);

}

#endif // !EXCELLENTZIP_UI
