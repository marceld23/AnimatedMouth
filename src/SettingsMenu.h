#pragma once

#include "Settings.h"

// On-device settings screen, navigated with the three Core2 touch buttons:
//   A         -> decrease the selected value
//   C         -> increase the selected value
//   B (short) -> move to the next setting
//   B (hold)  -> save & close the menu
//
// Shows the current battery level. Drawn directly to M5.Display (it only
// repaints on input, plus a slow periodic refresh for the battery reading).
class SettingsMenu {
 public:
  // Enter the menu and draw it. Pass the shared Settings instance to edit.
  void open(Settings* settings);

  // Process one tick of input. Returns true while the menu should stay open,
  // false once the user closed it (caller then saves & returns to the mouth).
  // M5.update() must have been called by the caller this tick.
  bool update();

 private:
  enum Row : int { kRowSensitivity = 0, kRowBrightness = 1, kRowCount = 2 };

  void draw();
  void drawRow(int row, const char* label, const char* value, float fraction);
  void drawBattery();

  Settings* settings_ = nullptr;
  int cursor_ = kRowSensitivity;
  uint32_t lastBatteryMs_ = 0;
  int lastBatteryLevel_ = -1;
};
