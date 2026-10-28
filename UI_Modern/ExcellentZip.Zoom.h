#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>
#include <vector>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Comctl32.lib")

namespace ExcellentZipZoom
{
    inline bool ParsePercent(const std::wstring& Text, int& Value)
    {
        Value = 0;
        size_t Start = 0, End = Text.size();
        while (Start < End && iswspace(Text[Start])) ++Start;
        while (End > Start && iswspace(Text[End - 1])) --End;
        if (End > Start && Text[End - 1] == L'%') --End;
        while (End > Start && iswspace(Text[End - 1])) --End;
        if (Start == End) return false;
        for (size_t Index = Start; Index < End; ++Index)
        {
            if (Text[Index] < L'0' || Text[Index] > L'9' || Value > 200) return false;
            Value = Value * 10 + Text[Index] - L'0';
        }
        return Value >= 100 && Value <= 200;
    }

    inline int ReadPercent()
    {
        DWORD Value = 100, Bytes = sizeof(Value);
        if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\ExcellentZip\\Appearance", L"InterfaceZoom",
            RRF_RT_REG_DWORD, nullptr, &Value, &Bytes) != ERROR_SUCCESS || Value < 100 || Value > 200)
            return 100;
        return static_cast<int>(Value);
    }

    inline int Percent()
    {
        static const int Value = ReadPercent(); // Keep every open surface consistent until the next launch.
        return Value;
    }

    inline bool Save(int PercentValue, std::wstring& Error)
    {
        Error.clear();
        if (PercentValue < 100 || PercentValue > 200)
        {
            Error = L"Choose an interface zoom between 100% and 200%.";
            return false;
        }
        HKEY Key = nullptr;
        LONG Result = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExcellentZip\\Appearance",
            0, nullptr, 0, KEY_SET_VALUE, nullptr, &Key, nullptr);
        if (Result == ERROR_SUCCESS)
        {
            const DWORD Value = static_cast<DWORD>(PercentValue);
            Result = RegSetValueExW(Key, L"InterfaceZoom", 0, REG_DWORD,
                reinterpret_cast<const BYTE*>(&Value), sizeof(Value));
            RegCloseKey(Key);
        }
        if (Result != ERROR_SUCCESS)
            Error = L"ExcellentZip could not save interface zoom. Check your account permissions and try again.";
        return Result == ERROR_SUCCESS;
    }

    inline int Scale(int Pixels) { return MulDiv(Pixels, Percent(), 100); }
    // The archive-dialog project uses older header targets and only needs template scaling.
#if _WIN32_WINNT >= 0x0605 && WINVER >= 0x0605
    inline int DpiScale(int Pixels, HWND Window)
    {
        return MulDiv(Scale(Pixels), static_cast<int>(GetDpiForWindow(Window)), USER_DEFAULT_SCREEN_DPI);
    }

#endif

    inline bool ScaleDialogTemplate(void* Data, size_t Bytes, int Zoom)
    {
        if (!Data || Bytes < 18 || Zoom < 100 || Zoom > 200) return false;
        auto* Words = static_cast<WORD*>(Data);
        const bool Extended = Words[0] == 1 && Words[1] == 0xffff;
        const size_t Header = Extended ? 26 : 18;
        if (Bytes < Header) return false;
        DWORD Style = 0;
        std::memcpy(&Style, static_cast<BYTE*>(Data) + (Extended ? 12 : 0), sizeof(Style));
        if (!(Style & DS_SETFONT)) return false;
        size_t Index = Header / sizeof(WORD);
        const size_t Count = Bytes / sizeof(WORD);
        for (int Field = 0; Field < 3; ++Field)
        {
            if (Index >= Count) return false;
            if (Words[Index] == 0xffff)
            {
                if (Count - Index < 2) return false;
                Index += 2;
            }
            else
            {
                while (Index < Count && Words[Index]) ++Index;
                if (Index >= Count) return false;
                ++Index;
            }
        }
        if (Index >= Count || Words[Index] == 0) return false;
        Words[Index] = static_cast<WORD>(MulDiv(Words[Index], Zoom, 100));
        return true;
    }

    inline std::vector<BYTE> DialogTemplate(HINSTANCE Instance, LPCWSTR Name)
    {
        const HRSRC Resource = FindResourceW(Instance, Name, RT_DIALOG);
        const DWORD Bytes = Resource ? SizeofResource(Instance, Resource) : 0;
        const void* Data = Resource ? LockResource(LoadResource(Instance, Resource)) : nullptr;
        if (!Data || !Bytes) return {};
        const auto* Begin = static_cast<const BYTE*>(Data);
        std::vector<BYTE> Result(Begin, Begin + Bytes);
        ScaleDialogTemplate(Result.data(), Result.size(), Percent());
        return Result;
    }

    inline LRESULT CALLBACK FontLifetime(HWND Window, UINT Message, WPARAM W, LPARAM L, UINT_PTR Id, DWORD_PTR)
    {
        const auto Result = DefSubclassProc(Window, Message, W, L);
        if (Message == WM_NCDESTROY)
        {
            const auto Font = reinterpret_cast<HFONT>(RemovePropW(Window, L"ExcellentZip.ZoomFont"));
            if (Font) DeleteObject(Font);
            RemovePropW(Window, L"ExcellentZip.ZoomDpi");
            RemoveWindowSubclass(Window, FontLifetime, Id);
        }
        return Result;
    }

#if _WIN32_WINNT >= 0x0605 && WINVER >= 0x0605
    inline void ApplyListFont(HWND Window)
    {
        if (!Window || Percent() == 100) return;
        const UINT Dpi = GetDpiForWindow(Window);
        if (reinterpret_cast<UINT_PTR>(GetPropW(Window, L"ExcellentZip.ZoomDpi")) == Dpi) return;
        NONCLIENTMETRICSW Metrics{};
        Metrics.cbSize = sizeof(Metrics);
        if (!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0, Dpi)) return;
        Metrics.lfMessageFont.lfHeight = Scale(Metrics.lfMessageFont.lfHeight);
        const auto Font = CreateFontIndirectW(&Metrics.lfMessageFont);
        if (!Font) return;
        if (!SetWindowSubclass(Window, FontLifetime, 0x455a, 0))
        {
            DeleteObject(Font);
            return;
        }
        const auto Previous = reinterpret_cast<HFONT>(GetPropW(Window, L"ExcellentZip.ZoomFont"));
        if (!SetPropW(Window, L"ExcellentZip.ZoomFont", Font))
        {
            DeleteObject(Font);
            return;
        }
        SendMessageW(Window, WM_SETFONT, reinterpret_cast<WPARAM>(Font), TRUE);
        const HWND Header = ListView_GetHeader(Window);
        if (Header) SendMessageW(Header, WM_SETFONT, reinterpret_cast<WPARAM>(Font), TRUE);
        SetPropW(Window, L"ExcellentZip.ZoomDpi", reinterpret_cast<HANDLE>(static_cast<UINT_PTR>(Dpi)));
        if (Previous) DeleteObject(Previous);
    }

#endif

    constexpr UINT kFitDialog = WM_APP + 0x45a;

    inline void ScrollDialog(HWND Window, int Bar, int Code, int Wheel = 0)
    {
        SCROLLINFO Info{sizeof(Info), SIF_ALL};
        if (!GetScrollInfo(Window, Bar, &Info)) return;
        const int Previous = Info.nPos;
        switch (Code)
        {
        case SB_LINEUP: Info.nPos -= Scale(24); break;
        case SB_LINEDOWN: Info.nPos += Scale(24); break;
        case SB_PAGEUP: Info.nPos -= static_cast<int>(Info.nPage); break;
        case SB_PAGEDOWN: Info.nPos += static_cast<int>(Info.nPage); break;
        case SB_TOP: Info.nPos = Info.nMin; break;
        case SB_BOTTOM: Info.nPos = Info.nMax; break;
        case SB_THUMBTRACK: Info.nPos = Info.nTrackPos; break;
        default: Info.nPos -= Wheel; break;
        }
        Info.fMask = SIF_POS;
        SetScrollInfo(Window, Bar, &Info, TRUE);
        GetScrollInfo(Window, Bar, &Info);
        ScrollWindowEx(Window, Bar == SB_HORZ ? Previous - Info.nPos : 0,
            Bar == SB_VERT ? Previous - Info.nPos : 0, nullptr, nullptr, nullptr, nullptr,
            SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    }

    inline LRESULT CALLBACK DialogViewport(HWND Window, UINT Message, WPARAM W, LPARAM L, UINT_PTR Id, DWORD_PTR)
    {
        if (Message == kFitDialog)
        {
            MONITORINFO Monitor{sizeof(Monitor)};
            RECT Bounds{}, Client{};
            GetWindowRect(Window, &Bounds);
            GetClientRect(Window, &Client);
            if (GetMonitorInfoW(MonitorFromWindow(Window, MONITOR_DEFAULTTONEAREST), &Monitor))
            {
                const int Width = (std::min)(Bounds.right - Bounds.left, Monitor.rcWork.right - Monitor.rcWork.left - 24);
                const int Height = (std::min)(Bounds.bottom - Bounds.top, Monitor.rcWork.bottom - Monitor.rcWork.top - 24);
                // Each scrollbar consumes space on the other axis; retain the whole dialog canvas.
                const bool Horizontal = Width < Bounds.right - Bounds.left || Height < Bounds.bottom - Bounds.top;
                const bool Vertical = Horizontal;
                if (Horizontal || Vertical)
                {
                    SetWindowLongPtrW(Window, GWL_STYLE, GetWindowLongPtrW(Window, GWL_STYLE) |
                        (Horizontal ? WS_HSCROLL : 0) | (Vertical ? WS_VSCROLL : 0));
                    SetWindowPos(Window, nullptr, Monitor.rcWork.left + (Monitor.rcWork.right - Monitor.rcWork.left - Width) / 2,
                        Monitor.rcWork.top + (Monitor.rcWork.bottom - Monitor.rcWork.top - Height) / 2,
                        Width, Height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                    RECT Viewport{};
                    GetClientRect(Window, &Viewport);
                    SCROLLINFO Scroll{sizeof(Scroll), SIF_RANGE | SIF_PAGE, 0, Client.bottom - 1,
                        static_cast<UINT>(Viewport.bottom)};
                    if (Vertical) SetScrollInfo(Window, SB_VERT, &Scroll, TRUE);
                    Scroll.nMax = Client.right - 1;
                    Scroll.nPage = static_cast<UINT>(Viewport.right);
                    if (Horizontal) SetScrollInfo(Window, SB_HORZ, &Scroll, TRUE);
                }
            }
            return 0;
        }
        if (Message == WM_VSCROLL || Message == WM_HSCROLL)
        {
            ScrollDialog(Window, Message == WM_VSCROLL ? SB_VERT : SB_HORZ, LOWORD(W));
            return 0;
        }
        if (Message == WM_MOUSEWHEEL && (GetWindowLongPtrW(Window, GWL_STYLE) & WS_VSCROLL))
        {
            ScrollDialog(Window, SB_VERT, -1, MulDiv(GET_WHEEL_DELTA_WPARAM(W), Scale(48), WHEEL_DELTA));
            return 0;
        }
        if (Message == WM_NCDESTROY) RemoveWindowSubclass(Window, DialogViewport, Id);
        return DefSubclassProc(Window, Message, W, L);
    }

    inline void WatchDialog(HWND Window)
    {
        if (Percent() > 100 && !(GetWindowLongPtrW(Window, GWL_STYLE) & WS_CHILD) &&
            SetWindowSubclass(Window, DialogViewport, 0x455b, 0))
            PostMessageW(Window, kFitDialog, 0, 0);
    }
}
