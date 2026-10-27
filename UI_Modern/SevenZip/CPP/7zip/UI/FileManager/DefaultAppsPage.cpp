#include "StdAfx.h"
#include "DefaultAppsPage.h"
#include "DefaultAppsPageRes.h"
#include "../../../../../ExcellentZip.DefaultApps.h"
#include "../../../../../ExcellentZip.Icons.h"
#include "../../../../../ExcellentZip.IconCatalog.h"

namespace
{
  constexpr int kCount = ARRAYSIZE(ExcellentZipIcons::kExtensions);
  constexpr UINT_PTR kRefreshTimer = 2600;
}

bool CDefaultAppsPage::OnInit()
{
  const HWND List = GetItem(IDC_DEFAULTS_LIST);
  ListView_SetExtendedListViewStyle(List, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
  RECT Size = { 0, 0, 16, 16 };
  MapDialogRect(*this, &Size);
  const int Pixels = MulDiv(16, Size.bottom, 16);
  _images = ImageList_Create(Pixels, Pixels, ILC_COLOR32 | ILC_MASK, kCount, 0);
  if (_images) ListView_SetImageList(List, _images, LVSIL_SMALL);
  RECT Bounds{};
  ::GetClientRect(List, &Bounds);
  LVCOLUMNW Column{};
  Column.mask = LVCF_TEXT | LVCF_WIDTH;
  Column.pszText = const_cast<LPWSTR>(L"File type");
  Column.cx = MulDiv(Bounds.right, 1, 3);
  ListView_InsertColumn(List, 0, &Column);
  Column.pszText = const_cast<LPWSTR>(L"Current default app");
  Column.cx = Bounds.right - Column.cx - GetSystemMetrics(SM_CXVSCROLL) - 6;
  ListView_InsertColumn(List, 1, &Column);
  const int Pack = ExcellentZipIcons::SelectedPack();
  _updating = true;
  for (int i = 0; i < kCount; ++i)
  {
    const auto Path = ExcellentZipIcons::IconPath(Pack, ExcellentZipIcons::kExtensions[i]);
    const auto Icon = static_cast<HICON>(::LoadImageW(nullptr, Path.c_str(), IMAGE_ICON,
      Pixels, Pixels, LR_LOADFROMFILE));
    int Image = -1;
    if (Icon)
    {
      if (_images) Image = ImageList_AddIcon(_images, Icon);
      ::DestroyIcon(Icon);
    }
    auto Label = L"." + std::wstring(ExcellentZipIcons::kExtensions[i]);
    LVITEMW Item{};
    Item.mask = LVIF_TEXT | LVIF_IMAGE;
    Item.iItem = i;
    Item.pszText = Label.data();
    Item.iImage = Image;
    ListView_InsertItem(List, &Item);
  }
  _imagePack = Pack;
  _updating = false;
  HideItem(IDB_DEFAULTS_STOP);
  RefreshApps();
  UpdateSelection();
  ::SetTimer(*this, kRefreshTimer, 1500, nullptr);
  return true;
}

CDefaultAppsPage::~CDefaultAppsPage()
{
  if (_images) ImageList_Destroy(_images);
}

bool CDefaultAppsPage::OnDestroy()
{
  ::KillTimer(*this, kRefreshTimer);
  if (_images)
  {
    ListView_SetImageList(GetItem(IDC_DEFAULTS_LIST), nullptr, LVSIL_SMALL);
    ImageList_Destroy(_images);
    _images = nullptr;
  }
  return false;
}

void CDefaultAppsPage::RefreshApps()
{
  _updating = true;
  const int Pack = ExcellentZipIcons::SelectedPack();
  if (_images && Pack != _imagePack)
  {
    int X = 0, Y = 0;
    ImageList_GetIconSize(_images, &X, &Y);
    ImageList_RemoveAll(_images);
    for (int i = 0; i < kCount; ++i)
    {
      const auto Path = ExcellentZipIcons::IconPath(Pack, ExcellentZipIcons::kExtensions[i]);
      const auto Icon = static_cast<HICON>(::LoadImageW(nullptr, Path.c_str(), IMAGE_ICON, X, Y, LR_LOADFROMFILE));
      LVITEMW Item{};
      Item.mask = LVIF_IMAGE;
      Item.iItem = i;
      Item.iImage = Icon ? ImageList_AddIcon(_images, Icon) : -1;
      ListView_SetItem(GetItem(IDC_DEFAULTS_LIST), &Item);
      if (Icon) ::DestroyIcon(Icon);
    }
    _imagePack = Pack;
  }
  for (int i = 0; i < kCount; ++i)
  {
    auto Name = ExcellentZipDefaults::CurrentApp(ExcellentZipIcons::kExtensions[i]);
    ListView_SetItemText(GetItem(IDC_DEFAULTS_LIST), i, 1, Name.data());
  }
  _updating = false;
  ExcellentZipIcons::RefreshLegacyAssociations();
}

void CDefaultAppsPage::UpdateSelection()
{
  if (_updating || !_pending.empty()) return;
  int Count = 0;
  for (int i = 0; i < kCount; ++i)
    if (ListView_GetCheckState(GetItem(IDC_DEFAULTS_LIST), i)) ++Count;
  wchar_t Text[160]{};
  swprintf_s(Text, L"%d of %d selected. Confirm your choices in Windows. File icons use your saved icon pack.", Count, kCount);
  SetItemText(IDT_DEFAULTS_STATUS, Text);
  EnableItem(IDB_DEFAULTS_CHOOSE, Count != 0);
}

void CDefaultAppsPage::FinishSelection()
{
  _pending.clear();
  _position = 0;
  EnableItem(IDC_DEFAULTS_LIST, true);
  EnableItem(IDB_DEFAULTS_SELECT_ALL, true);
  EnableItem(IDB_DEFAULTS_CLEAR, true);
  EnableItem(IDB_DEFAULTS_ALL, true);
  HideItem(IDB_DEFAULTS_STOP);
  SetItemText(IDB_DEFAULTS_CHOOSE, L"Choose &selected...");
  RefreshApps();
  UpdateSelection();
}

void CDefaultAppsPage::OpenNextType()
{
  if (_position >= _pending.size())
  {
    int Confirmed = 0;
    for (int Index : _pending)
      if (ExcellentZipDefaults::IsDefault(ExcellentZipIcons::kExtensions[Index])) ++Confirmed;
    const int Total = static_cast<int>(_pending.size());
    FinishSelection();
    wchar_t Status[200]{};
    swprintf_s(Status, L"ExcellentZip is the default for %d of the %d chosen file types. Use Refresh after changing Windows defaults.", Confirmed, Total);
    SetItemText(IDT_DEFAULTS_STATUS, Status);
    return;
  }
  const auto* Extension = ExcellentZipIcons::kExtensions[_pending[_position]];
  std::wstring Error;
  if (!ExcellentZipDefaults::Open(*this, Extension, Error))
  {
    ::MessageBoxW(*this, Error.c_str(), L"ExcellentZip", MB_OK | MB_ICONERROR);
    FinishSelection();
    return;
  }
  wchar_t Status[230]{};
  swprintf_s(Status, L".%s (%u of %u): choose ExcellentZip in Windows, then return and select Next. If Windows opens the full list, search for .%s.",
    Extension, static_cast<unsigned>(_position + 1), static_cast<unsigned>(_pending.size()), Extension);
  SetItemText(IDT_DEFAULTS_STATUS, Status);
  SetItemText(IDB_DEFAULTS_CHOOSE, _position + 1 == _pending.size() ? L"&Finish" : L"&Next file type");
}

bool CDefaultAppsPage::OnButtonClicked(int ID, HWND Button)
{
  const HWND List = GetItem(IDC_DEFAULTS_LIST);
  switch (ID)
  {
    case IDB_DEFAULTS_SELECT_ALL:
    case IDB_DEFAULTS_CLEAR:
      _updating = true;
      for (int i = 0; i < kCount; ++i)
        ListView_SetCheckState(List, i, ID == IDB_DEFAULTS_SELECT_ALL);
      _updating = false;
      UpdateSelection();
      return true;
    case IDB_DEFAULTS_REFRESH:
      RefreshApps();
      UpdateSelection();
      return true;
    case IDB_DEFAULTS_STOP:
      FinishSelection();
      return true;
    case IDB_DEFAULTS_ALL:
    {
      std::wstring Error;
      if (!ExcellentZipDefaults::Open(*this, nullptr, Error))
        ::MessageBoxW(*this, Error.c_str(), L"ExcellentZip", MB_OK | MB_ICONERROR);
      else
        SetItemText(IDT_DEFAULTS_STATUS, L"In Windows, choose ExcellentZip for the supported file types. Return here and select Refresh to check the results.");
      return true;
    }
    case IDB_DEFAULTS_CHOOSE:
      if (!_pending.empty())
      {
        RefreshApps();
        ++_position;
        OpenNextType();
        return true;
      }
      for (int i = 0; i < kCount; ++i)
        if (ListView_GetCheckState(List, i)) _pending.push_back(i);
      if (_pending.empty()) return true;
      if (_pending.size() == kCount)
      {
        FinishSelection();
        return OnButtonClicked(IDB_DEFAULTS_ALL, Button);
      }
      _position = 0;
      EnableItem(IDC_DEFAULTS_LIST, false);
      EnableItem(IDB_DEFAULTS_SELECT_ALL, false);
      EnableItem(IDB_DEFAULTS_CLEAR, false);
      EnableItem(IDB_DEFAULTS_ALL, false);
      ShowItem(IDB_DEFAULTS_STOP, SW_SHOW);
      OpenNextType();
      return true;
    default:
      return CPropertyPage::OnButtonClicked(ID, Button);
  }
}

bool CDefaultAppsPage::OnNotify(UINT ID, LPNMHDR Message)
{
  if (ID == IDC_DEFAULTS_LIST && Message->code == LVN_ITEMCHANGED)
  {
    UpdateSelection();
    return true;
  }
  return CPropertyPage::OnNotify(ID, Message);
}

LONG CDefaultAppsPage::OnSetActive()
{
  if (GetItem(IDC_DEFAULTS_LIST)) RefreshApps();
  return PSNRET_NOERROR;
}

bool CDefaultAppsPage::OnTimer(WPARAM Timer, LPARAM)
{
  if (Timer != kRefreshTimer) return false;
  // Re-query only while this page is visible and Windows may have changed the defaults.
  if (::IsWindowVisible(*this) && ::GetForegroundWindow() == ::GetAncestor(*this, GA_ROOT))
    RefreshApps();
  return true;
}
