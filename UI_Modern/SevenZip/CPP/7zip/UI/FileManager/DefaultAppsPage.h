#pragma once
#include "../../../Windows/Control/PropertyPage.h"
#include <CommCtrl.h>
#include <vector>

class CDefaultAppsPage: public NWindows::NControl::CPropertyPage
{
  HIMAGELIST _images = nullptr;
  bool _updating = false;
  int _imagePack = 0;
  std::vector<int> _pending;
  size_t _position = 0;
  void RefreshApps();
  void UpdateSelection();
  void FinishSelection();
  void OpenNextType();
  bool OnInit() override;
  bool OnNotify(UINT controlID, LPNMHDR message) override;
  bool OnButtonClicked(int buttonID, HWND buttonHWND) override;
  LONG OnSetActive() override;
  bool OnTimer(WPARAM timerID, LPARAM callback) override;
  bool OnDestroy() override;
public:
  ~CDefaultAppsPage();
};
