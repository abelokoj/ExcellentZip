// Windows/Control/PropertyPage.cpp

#include "StdAfx.h"

#ifndef _UNICODE
#include "../../Common/StringConvert.h"
#endif

#include "PropertyPage.h"
#include "../../../../ExcellentZip.Zoom.h"

extern HINSTANCE g_hInstance;
#ifndef _UNICODE
extern bool g_IsNT;
#endif

namespace NWindows {
namespace NControl {

static INT_PTR APIENTRY MyProperyPageProcedure(HWND dialogHWND, UINT message, WPARAM wParam, LPARAM lParam)
{
  CWindow tempDialog(dialogHWND);
  if (message == WM_INITDIALOG)
    tempDialog.SetUserDataLongPtr(((PROPSHEETPAGE *)lParam)->lParam);
  CDialog *dialog = (CDialog *)(tempDialog.GetUserDataLongPtr());
  if (dialog == NULL)
    return FALSE;
  if (message == WM_INITDIALOG)
    dialog->Attach(dialogHWND);
  try { return BoolToBOOL(dialog->OnMessage(message, wParam, lParam)); }
  catch(...) { return TRUE; }
}

bool CPropertyPage::OnNotify(UINT /* controlID */, LPNMHDR lParam)
{
  switch (lParam->code)
  {
    case PSN_APPLY: SetMsgResult(OnApply(LPPSHNOTIFY(lParam))); break;
    case PSN_KILLACTIVE: SetMsgResult(BoolToBOOL(OnKillActive(LPPSHNOTIFY(lParam)))); break;
    case PSN_SETACTIVE: SetMsgResult(OnSetActive(LPPSHNOTIFY(lParam))); break;
    case PSN_RESET: OnReset(LPPSHNOTIFY(lParam)); break;
    default: return false;
  }
  return true;
}

static int CALLBACK ZoomSheetCallback(HWND Window, UINT Message, LPARAM Parameter)
{
  if (Message == PSCB_PRECREATE)
  {
    MEMORY_BASIC_INFORMATION Memory{};
    const auto* Data = reinterpret_cast<const BYTE*>(Parameter);
    if (VirtualQuery(Data, &Memory, sizeof(Memory)))
    {
      const size_t Bytes = Memory.RegionSize - (Data - static_cast<const BYTE*>(Memory.BaseAddress));
      ExcellentZipZoom::ScaleDialogTemplate(reinterpret_cast<void*>(Parameter), Bytes, ExcellentZipZoom::Percent());
    }
  }
  if (Message == PSCB_INITIALIZED) ExcellentZipZoom::WatchDialog(Window);
  return 0;
}

INT_PTR MyPropertySheet(const CObjectVector<CPageInfo> &pagesInfo, HWND hwndParent, const UString &title)
{
  #ifndef _UNICODE
  AStringVector titles;
  #endif
  #ifndef _UNICODE
  CRecordVector<PROPSHEETPAGEA> pagesA;
  #endif
  CRecordVector<PROPSHEETPAGEW> pagesW;
  std::vector<std::vector<BYTE>> Templates(pagesInfo.Size());

  unsigned i;
  #ifndef _UNICODE
  for (i = 0; i < pagesInfo.Size(); i++)
    titles.Add(GetSystemString(pagesInfo[i].Title));
  #endif

  for (i = 0; i < pagesInfo.Size(); i++)
  {
    const CPageInfo &pageInfo = pagesInfo[i];
    #ifndef _UNICODE
    {
      PROPSHEETPAGE page;
      page.dwSize = sizeof(page);
      page.dwFlags = 0;
      page.hInstance = g_hInstance;
      page.pszTemplate = MAKEINTRESOURCE(pageInfo.ID);
      page.pszIcon = NULL;
      page.pfnDlgProc = NWindows::NControl::MyProperyPageProcedure;

      if (titles[i].IsEmpty())
        page.pszTitle = NULL;
      else
      {
        page.dwFlags |= PSP_USETITLE;
        page.pszTitle = titles[i];
      }
      page.lParam = (LPARAM)pageInfo.Page;
      page.pfnCallback = NULL;
      pagesA.Add(page);
    }
    #endif
    {
      PROPSHEETPAGEW page;
      page.dwSize = sizeof(page);
      page.dwFlags = 0;
      page.hInstance = g_hInstance;
      Templates[i] = ExcellentZipZoom::DialogTemplate(g_hInstance, MAKEINTRESOURCEW(pageInfo.ID));
      if (!Templates[i].empty())
      {
        page.dwFlags |= PSP_DLGINDIRECT;
        page.pResource = reinterpret_cast<LPCDLGTEMPLATE>(Templates[i].data());
      }
      else page.pszTemplate = MAKEINTRESOURCEW(pageInfo.ID);
      page.pszIcon = NULL;
      page.pfnDlgProc = NWindows::NControl::MyProperyPageProcedure;

      if (pageInfo.Title.IsEmpty())
        page.pszTitle = NULL;
      else
      {
        page.dwFlags |= PSP_USETITLE;
        page.pszTitle = pageInfo.Title;
      }
      page.lParam = (LPARAM)pageInfo.Page;
      page.pfnCallback = NULL;
      pagesW.Add(page);
    }
  }

  #ifndef _UNICODE
  if (!g_IsNT)
  {
    PROPSHEETHEADER sheet;
    sheet.dwSize = sizeof(sheet);
    sheet.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
    sheet.hwndParent = hwndParent;
    sheet.hInstance = g_hInstance;
    AString titleA (GetSystemString(title));
    sheet.pszCaption = titleA;
    sheet.nPages = pagesInfo.Size();
    sheet.nStartPage = 0;
    sheet.ppsp = &pagesA.Front();
    sheet.pfnCallback = ZoomSheetCallback;
    return ::PropertySheetA(&sheet);
  }
  else
  #endif
  {
    PROPSHEETHEADERW sheet;
    sheet.dwSize = sizeof(sheet);
    sheet.dwFlags = PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP | PSH_USECALLBACK;
    sheet.hwndParent = hwndParent;
    sheet.hInstance = g_hInstance;
    sheet.pszCaption = title;
    sheet.nPages = pagesInfo.Size();
    sheet.nStartPage = 0;
    sheet.ppsp = &pagesW.Front();
    sheet.pfnCallback = ZoomSheetCallback;
    return ::PropertySheetW(&sheet);
  }
}

}}
