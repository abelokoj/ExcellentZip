/*
 * PROJECT:    ExcellentZip
 * FILE:       ExcellentZip.UI.cpp
 * PURPOSE:    Implementation for ExcellentZip Modern UI Shared Infrastructures
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include "ExcellentZip.UI.h"

#include <Mile.Helpers.h>
#include <Mile.Xaml.h>

#include <ExcellentZip.Modern.h>

#include "../SevenZip/CPP/Common/Common.h"
#include "../SevenZip/CPP/7zip/UI/Common/LoadCodecs.h"

extern CCodecs* g_CodecsObj;

winrt::handle ExcellentZip::UI::ShowAboutDialog(
    _In_ HWND ParentWindowHandle)
{
    return winrt::handle(Mile::CreateThread([=]()
    {
        UString ExtendedMessage;
        if (g_CodecsObj)
        {
            g_CodecsObj->GetCodecsErrorMessage(ExtendedMessage);
        }

        ::K7ModernShowAboutDialog(ParentWindowHandle, ExtendedMessage);
    }));
}

