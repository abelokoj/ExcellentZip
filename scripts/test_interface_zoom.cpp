#include "ExcellentZip.Zoom.h"
#include <cassert>
#include <iostream>

int wmain()
{
    GUID Id{};
    wchar_t Guid[40]{};
    assert(SUCCEEDED(CoCreateGuid(&Id)));
    StringFromGUID2(Id, Guid, ARRAYSIZE(Guid));
    const auto TestPath = L"Software\\ExcellentZip.Tests\\Zoom-" + std::wstring(Guid);
    HKEY Root = nullptr;
    assert(RegCreateKeyExW(HKEY_CURRENT_USER, TestPath.c_str(), 0, nullptr, 0,
        KEY_ALL_ACCESS, nullptr, &Root, nullptr) == ERROR_SUCCESS);
    assert(RegOverridePredefKey(HKEY_CURRENT_USER, Root) == ERROR_SUCCESS);
    using namespace ExcellentZipZoom;
    assert(ReadPercent() == 100);
    int Custom = 0;
    for (const auto* Text : {L"100", L"125%", L"135", L" 187% ", L"135 %", L"200"})
        assert(ParsePercent(Text, Custom) && Custom >= 100 && Custom <= 200);
    assert(ParsePercent(L"135%", Custom) && Custom == 135);
    for (const auto* Text : {L"", L"99", L"201", L"1.5", L"150%%", L"100garbage", L"-125", L"99999999999999", L"  %"})
        assert(!ParsePercent(Text, Custom));
    std::wstring Error;
    assert(!Save(99, Error) && !Error.empty());
    assert(!Save(201, Error));
    for (int Percent : {100, 125, 135, 150, 175, 187, 200})
    {
        assert(Save(Percent, Error) && Error.empty());
        assert(ReadPercent() == Percent);
        // A standard dialog template with an empty menu, class and title, then its font.
        WORD Standard[20]{};
        reinterpret_cast<DLGTEMPLATE*>(Standard)->style = DS_SETFONT;
        Standard[12] = 8;
        assert(ScaleDialogTemplate(Standard, sizeof(Standard), Percent));
        assert(Standard[12] == MulDiv(8, Percent, 100));
        WORD Extended[24] = {1, 0xffff};
        *reinterpret_cast<DWORD*>(&Extended[6]) = DS_SETFONT;
        Extended[16] = 9;
        assert(ScaleDialogTemplate(Extended, sizeof(Extended), Percent));
        assert(Extended[16] == MulDiv(9, Percent, 100));
        assert(!ScaleDialogTemplate(Extended, 12, Percent));
        assert(!ScaleDialogTemplate(Standard, sizeof(Standard), 201));
    }
    const DWORD CorruptZoom = 100000;
    assert(RegSetKeyValueW(Root, L"Software\\ExcellentZip\\Appearance", L"InterfaceZoom",
        REG_DWORD, &CorruptZoom, sizeof(CorruptZoom)) == ERROR_SUCCESS);
    assert(ReadPercent() == 100);
    assert(Save(100, Error));
    // A zoomed dialog can put its bottom controls completely below the viewport.
    HWND Frame = CreateWindowExW(0, L"STATIC", L"Zoom viewport test", WS_POPUP | WS_VSCROLL,
        0, 0, 300, 180, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    assert(Frame);
    HWND Bottom = CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD,
        10, 400, 60, 20, Frame, nullptr, GetModuleHandleW(nullptr), nullptr);
    assert(Bottom);
    SCROLLINFO Scroll{sizeof(Scroll), SIF_RANGE | SIF_PAGE, 0, 500, 180};
    SetScrollInfo(Frame, SB_VERT, &Scroll, FALSE);
    ScrollDialog(Frame, SB_VERT, SB_BOTTOM);
    RECT ButtonBounds{};
    GetWindowRect(Bottom, &ButtonBounds);
    MapWindowPoints(HWND_DESKTOP, Frame, reinterpret_cast<POINT*>(&ButtonBounds), 2);
    assert(ButtonBounds.top == 79);
    assert(DestroyWindow(Frame));
    assert(RegOverridePredefKey(HKEY_CURRENT_USER, nullptr) == ERROR_SUCCESS);
    RegCloseKey(Root);
    assert(TestPath.starts_with(L"Software\\ExcellentZip.Tests\\Zoom-{"));
    assert(RegDeleteTreeW(HKEY_CURRENT_USER, TestPath.c_str()) == ERROR_SUCCESS);
    std::cout << "Interface zoom: persisted range, invalid values, standard/extended dialog fonts and bounds passed.\n";
}
