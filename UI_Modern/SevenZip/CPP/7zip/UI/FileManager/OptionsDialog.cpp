// OptionsDialog.cpp

#include "StdAfx.h"

#include "../../../Windows/Control/Dialog.h"
#include "../../../Windows/Control/PropertyPage.h"

#include "DialogSize.h"

#include "EditPage.h"
#include "EditPageRes.h"
#include "FoldersPage.h"
#include "FoldersPageRes.h"
#include "MenuPage.h"
#include "MenuPageRes.h"
#include "SettingsPage.h"
#include "SettingsPageRes.h"
#include "DefaultAppsPage.h"
#include "DefaultAppsPageRes.h"
#include "../../../../../ExcellentZip.DefaultApps.h"

#include "App.h"
#include "LangUtils.h"
#include "MyLoadMenu.h"

#include "resource.h"

using namespace NWindows;

void OptionsDialog(HWND hwndOwner, HINSTANCE hInstance);
void OptionsDialog(HWND hwndOwner, HINSTANCE /* hInstance */)
{
  CMenuPage menuPage;
  CFoldersPage foldersPage;
  CEditPage editPage;
  CSettingsPage settingsPage;
  CDefaultAppsPage defaultAppsPage;

  CObjectVector<NControl::CPageInfo> pages;
  BIG_DIALOG_SIZE(200, 200);

  const UINT pageIDs[] = {
      SIZED_DIALOG(IDD_MENU),
      SIZED_DIALOG(IDD_FOLDERS),
      SIZED_DIALOG(IDD_EDIT),
      SIZED_DIALOG(IDD_SETTINGS),
      IDD_DEFAULT_APPS
  };

  NControl::CPropertyPage *pagePointers[] = { &menuPage, &foldersPage, &editPage, &settingsPage, &defaultAppsPage };

  for (unsigned i = 0; i < ARRAY_SIZE(pageIDs); i++)
  {
    const unsigned Index = !ExcellentZipDefaults::PendingSelection.empty()
        ? (i == 0 ? 4 : i == 4 ? 0 : i) : i;
    NControl::CPageInfo &page = pages.AddNew();
    page.ID = pageIDs[Index];
    LangString_OnlyFromLangFile(page.ID, page.Title);
    page.Page = pagePointers[Index];
  }

  INT_PTR res = NControl::MyPropertySheet(pages, hwndOwner, LangString(IDS_OPTIONS));

  if (res != -1 && res != 0)
  {
    g_App.SetListSettings();
    g_App.RefreshAllPanels();
    // ::PostMessage(hwndOwner, kLangWasChangedMessage, 0 , 0);
  }
}
