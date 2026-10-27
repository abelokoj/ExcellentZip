// SettingsPage.cpp

#include "StdAfx.h"

// #include "../../../Common/IntToString.h"
#include "../../../Common/StringConvert.h"

#ifndef UNDER_CE
#include "../../../Windows/MemoryLock.h"
// #include "../../../Windows/System.h"
#endif

// #include "../Common/ZipRegistry.h"

#include "LangUtils.h"
#include "RegistryUtils.h"
#include "SettingsPage.h"
#include "SettingsPageRes.h"
#include "../../../../../ExcellentZip.Icons.h"
#include "../../../../../ExcellentZip.IconCatalog.h"

using namespace NWindows;

static const UInt32 kLangIDs[] =
{
  IDX_SETTINGS_SHOW_DOTS,
  IDX_SETTINGS_SHOW_REAL_FILE_ICONS,
  IDX_SETTINGS_SHOW_SYSTEM_MENU,
  IDX_SETTINGS_FULL_ROW,
  IDX_SETTINGS_SHOW_GRID,
  IDX_SETTINGS_SINGLE_CLICK,
  IDX_SETTINGS_ALTERNATIVE_SELECTION,
  IDX_SETTINGS_LARGE_PAGES
  // , IDT_COMPRESS_MEMORY
};

extern bool IsLargePageSupported();

/*
static void AddMemSize(UString &res, UInt64 size, bool needRound = false)
{
  char c;
  unsigned moveBits = 0;
  if (needRound)
  {
    UInt64 rn = 0;
    if (size >= (1 << 31))
      rn = (1 << 28) - 1;
    UInt32 kRound = (1 << 20) - 1;
    if (rn < kRound)
      rn = kRound;
    size += rn;
    size &= ~rn;
  }
  if (size >= ((UInt64)1 << 31) && (size & 0x3FFFFFFF) == 0)
    { moveBits = 30; c = 'G'; }
  else
    { moveBits = 20; c = 'M'; }
  res.Add_UInt64(size >> moveBits);
  res.Add_Space();
  if (moveBits != 0)
    res += c;
  res += 'B';
}


int CSettingsPage::AddMemComboItem(UInt64 size, UInt64 percents, bool isDefault)
{
  UString sUser;
  UString sRegistry;
  if (size == 0)
  {
    UString s;
    s.Add_UInt64(percents);
    s += '%';
    if (isDefault)
      sUser = "* ";
    else
      sRegistry = s;
    sUser += s;
  }
  else
  {
    AddMemSize(sUser, size);
    sRegistry = sUser;
    for (;;)
    {
      int pos = sRegistry.Find(L' ');
      if (pos < 0)
        break;
      sRegistry.Delete(pos);
    }
    if (!sRegistry.IsEmpty())
      if (sRegistry.Back() == 'B')
        sRegistry.DeleteBack();
  }
  const int index = (int)_memCombo.AddString(sUser);
  _memCombo.SetItemData(index, _memLimitStrings.Size());
  _memLimitStrings.Add(sRegistry);
  return index;
}
*/

bool CSettingsPage::OnInit()
{
  _iconPackCombo.Attach(GetItem(IDC_EXCELLENTZIP_ICON_PACK));
  for (int Pack = 0; Pack < ExcellentZipIcons::kPackCount; ++Pack)
    _iconPackCombo.AddString(ExcellentZipIcons::kPackNames[Pack]);
  _iconPackPreviewPack = ExcellentZipIcons::SelectedPack();
  _iconPackCombo.SetCurSel(_iconPackPreviewPack - 1);
  UpdateIconPackPreview();
  _wasChanged = false;
  _largePages_wasChanged = false;
  /*
  _wasChanged_MemLimit = false;
  _memLimitStrings.Clear();
  _memCombo.Attach(GetItem(IDC_SETTINGS_MEM));
  */

  LangSetDlgItems(*this, kLangIDs, ARRAY_SIZE(kLangIDs));

  CFmSettings st;
  st.Load();

  CheckButton(IDX_SETTINGS_SHOW_DOTS, st.ShowDots);
  CheckButton(IDX_SETTINGS_SHOW_REAL_FILE_ICONS, st.ShowRealFileIcons);
  CheckButton(IDX_SETTINGS_FULL_ROW, st.FullRow);
  CheckButton(IDX_SETTINGS_SHOW_GRID, st.ShowGrid);
  CheckButton(IDX_SETTINGS_SINGLE_CLICK, st.SingleClick);
  CheckButton(IDX_SETTINGS_ALTERNATIVE_SELECTION, st.AlternativeSelection);
  // CheckButton(IDX_SETTINGS_UNDERLINE, st.Underline);

  CheckButton(IDX_SETTINGS_SHOW_SYSTEM_MENU, st.ShowSystemMenu);
  
  if (IsLargePageSupported())
    CheckButton(IDX_SETTINGS_LARGE_PAGES, ReadLockMemoryEnable());
  else
    EnableItem(IDX_SETTINGS_LARGE_PAGES, false);


  /*
  NCompression::CMemUse mu;
  bool needSetCur = NCompression::MemLimit_Load(mu);
  UInt64 curMemLimit;
  {
    AddMemComboItem(0, 90, true);
    _memCombo.SetCurSel(0);
  }
  if (mu.IsPercent)
  {
    const int index = AddMemComboItem(0, mu.Val);
    _memCombo.SetCurSel(index);
    needSetCur = false;
  }
  {
    _ramSize = (UInt64)(sizeof(size_t)) << 29;
    _ramSize_Defined = NSystem::GetRamSize(_ramSize);
    UString s;
    if (_ramSize_Defined)
    {
      s += "/ ";
      AddMemSize(s, _ramSize, true);
    }
    SetItemText(IDT_SETTINGS_MEM_RAM, s);

    curMemLimit = mu.GetBytes(_ramSize);

    // size = 100 << 20; // for debug only;
    for (unsigned i = (27) * 2;; i++)
    {
      UInt64 size = (UInt64)(2 + (i & 1)) << (i / 2);
      if (i > (20 + sizeof(size_t) * 3 * 1 - 1) * 2)
        size = (UInt64)(Int64)-1;
      if (needSetCur && (size >= curMemLimit))
      {
        const int index = AddMemComboItem(curMemLimit);
        _memCombo.SetCurSel(index);
        needSetCur = false;
        if (size == curMemLimit)
          continue;
      }
      if (size == (UInt64)(Int64)-1)
        break;
      AddMemComboItem(size);
    }
  }
  */
  
  // EnableSubItems();

  return CPropertyPage::OnInit();
}

/*
void CSettingsPage::EnableSubItems()
{
  EnableItem(IDX_SETTINGS_UNDERLINE, IsButtonCheckedBool(IDX_SETTINGS_SINGLE_CLICK));
}
*/

/*
static void AddSize_MB(UString &s, UInt64 size)
{
  s.Add_UInt64((size + (1 << 20) - 1) >> 20);
  s += " MB";
}
*/

LONG CSettingsPage::OnApply()
{
  if (_iconPackWasChanged)
  {
    const int Pack = _iconPackCombo.GetCurSel() + 1;
    if (!ExcellentZipIcons::EnsurePackAvailable(*this, Pack))
      return PSNRET_INVALID_NOCHANGEPAGE;
    std::wstring Error;
    if (!ExcellentZipIcons::ApplyPack(_iconPackCombo.GetCurSel() + 1, Error))
    {
      ::MessageBoxW(*this, Error.c_str(), L"ExcellentZip", MB_OK | MB_ICONERROR);
      return PSNRET_INVALID_NOCHANGEPAGE;
    }
    extern HWND g_HWND;
    ExcellentZipIcons::UpdateWindowIcon(g_HWND);
    _iconPackWasChanged = false;
  }
  if (_wasChanged)
  {
    CFmSettings st;
    st.ShowDots = IsButtonCheckedBool(IDX_SETTINGS_SHOW_DOTS);
    st.ShowRealFileIcons = IsButtonCheckedBool(IDX_SETTINGS_SHOW_REAL_FILE_ICONS);
    st.FullRow = IsButtonCheckedBool(IDX_SETTINGS_FULL_ROW);
    st.ShowGrid = IsButtonCheckedBool(IDX_SETTINGS_SHOW_GRID);
    st.SingleClick = IsButtonCheckedBool(IDX_SETTINGS_SINGLE_CLICK);
    st.AlternativeSelection = IsButtonCheckedBool(IDX_SETTINGS_ALTERNATIVE_SELECTION);
    // st.Underline = IsButtonCheckedBool(IDX_SETTINGS_UNDERLINE);
    
    st.ShowSystemMenu = IsButtonCheckedBool(IDX_SETTINGS_SHOW_SYSTEM_MENU);

    st.Save();
    _wasChanged = false;
  }

  #ifndef UNDER_CE
  if (_largePages_wasChanged)
  {
    if (IsLargePageSupported())
    {
      bool enable = IsButtonCheckedBool(IDX_SETTINGS_LARGE_PAGES);
      NSecurity::EnablePrivilege_LockMemory(enable);
      SaveLockMemoryEnable(enable);
    }
    _largePages_wasChanged = false;
  }
  #endif

  /*
  if (_wasChanged_MemLimit)
  {
    const unsigned index = (int)_memCombo.GetItemData_of_CurSel();
    const UString str = _memLimitStrings[index];

    bool needSave = true;
    
    NCompression::CMemUse mu;

    if (_ramSize_Defined)
      mu.Parse(str);
    if (mu.IsDefined)
    {
      const UInt64 usage64 = mu.GetBytes(_ramSize);
      if (_ramSize <= usage64)
      {
        UString s2 = LangString(IDT_COMPRESS_MEMORY);
        if (s2.IsEmpty())
          GetItemText(IDT_COMPRESS_MEMORY, s2);
        UString s;
        
        s += "The selected value is not safe for system performance.";
        s.Add_LF();
        s += "The memory consumption for compression operation will exceed RAM size.";
        s.Add_LF();
        s.Add_LF();
        AddSize_MB(s, usage64);
        
        if (!s2.IsEmpty())
        {
          s += " : ";
          s += s2;
        }
        
        s.Add_LF();
        AddSize_MB(s, _ramSize);
        s += " : RAM";
        
        s.Add_LF();
        s.Add_LF();
        s += "Are you sure you want set that unsafe value for memory usage?";
        
        int res = MessageBoxW(*this, s, L"7-Zip", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (res != IDYES)
          needSave = false;
      }
    }

    if (needSave)
    {
      NCompression::MemLimit_Save(str);
      _wasChanged_MemLimit = false;
    }
    else
      return PSNRET_INVALID_NOCHANGEPAGE;
  }
  */
    
  return PSNRET_NOERROR;
}

/*
bool CSettingsPage::OnCommand(int code, int itemID, LPARAM param)
{
  if (code == CBN_SELCHANGE)
  {
    switch (itemID)
    {
      case IDC_SETTINGS_MEM:
      {
        _wasChanged_MemLimit = true;
        Changed();
        break;
      }
    }
  }
  return CPropertyPage::OnCommand(code, itemID, param);
}
*/

bool CSettingsPage::OnButtonClicked(int buttonID, HWND buttonHWND)
{
  switch (buttonID)
  {
    case IDX_SETTINGS_SINGLE_CLICK:
    /*
      EnableSubItems();
      break;
    */
    case IDX_SETTINGS_SHOW_DOTS:
    case IDX_SETTINGS_SHOW_SYSTEM_MENU:
    case IDX_SETTINGS_SHOW_REAL_FILE_ICONS:
    case IDX_SETTINGS_FULL_ROW:
    case IDX_SETTINGS_SHOW_GRID:
    case IDX_SETTINGS_ALTERNATIVE_SELECTION:
      _wasChanged = true;
      break;

    case IDX_SETTINGS_LARGE_PAGES:
      _largePages_wasChanged = true;
      break;

    default:
      return CPropertyPage::OnButtonClicked(buttonID, buttonHWND);
  }

  Changed();
  return true;
}


CSettingsPage::~CSettingsPage()
{
  for (HICON Icon : _iconPackPreview)
    if (Icon) ::DestroyIcon(Icon);
}

void CSettingsPage::UpdateIconPackPreview()
{
  const int Controls[] = { IDI_EXCELLENTZIP_PACK_APP, IDI_EXCELLENTZIP_PACK_ZIP,
    IDI_EXCELLENTZIP_PACK_7Z, IDI_EXCELLENTZIP_PACK_RAR };
  const wchar_t* Extensions[] = { L"app", L"zip", L"7z", L"rar" };
  RECT Size = { 0, 0, 24, 24 };
  MapDialogRect(*this, &Size);
  for (unsigned i = 0; i < 4; ++i)
  {
    const auto Path = ExcellentZipIcons::IconPath(_iconPackCombo.GetCurSel() + 1, Extensions[i]);
    HICON Icon = static_cast<HICON>(::LoadImageW(nullptr, Path.c_str(), IMAGE_ICON,
      Size.right, Size.bottom, LR_LOADFROMFILE));
    SendItemMessage(Controls[i], STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(Icon));
    if (_iconPackPreview[i]) ::DestroyIcon(_iconPackPreview[i]);
    _iconPackPreview[i] = Icon;
  }
}

bool CSettingsPage::OnCommand(int code, int itemID, LPARAM param)
{
  if (code == CBN_SELCHANGE && itemID == IDC_EXCELLENTZIP_ICON_PACK)
  {
    const int Pack = _iconPackCombo.GetCurSel() + 1;
    if (!ExcellentZipIcons::EnsurePackAvailable(*this, Pack))
    {
      _iconPackCombo.SetCurSel(_iconPackPreviewPack - 1);
      return true;
    }
    _iconPackPreviewPack = Pack;
    _iconPackWasChanged = true;
    UpdateIconPackPreview();
    Changed();
    return true;
  }
  return CPropertyPage::OnCommand(code, itemID, param);
}
