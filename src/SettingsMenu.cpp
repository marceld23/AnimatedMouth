#include "SettingsMenu.h"

#include <M5Unified.h>
#include <stdio.h>

namespace {
constexpr int kScreenW = 320;

// Colors.
constexpr uint16_t kColBg = 0x0000;       // black
constexpr uint16_t kColTitle = 0xFFFF;    // white
constexpr uint16_t kColLabel = 0xC618;    // light grey
constexpr uint16_t kColValue = 0xFFFF;    // white
constexpr uint16_t kColBarBg = 0x2104;    // dark grey
constexpr uint16_t kColBarFill = 0x07FF;  // cyan
constexpr uint16_t kColSel = 0xFD20;      // orange (selection)
constexpr uint16_t kColHint = 0x8410;     // grey

// Row geometry.
constexpr int kRowY0 = 84;
constexpr int kRowDY = 66;
constexpr int kBarX = 28;
constexpr int kBarW = 264;
constexpr int kBarH = 16;

// Battery icon geometry (top-right).
constexpr int kBattX = 250;
constexpr int kBattY = 14;
constexpr int kBattW = 46;
constexpr int kBattH = 22;
}  // namespace

void SettingsMenu::open(Settings* settings) {
  settings_ = settings;
  cursor_ = kRowSensitivity;
  lastBatteryLevel_ = -1;  // force battery redraw
  lastBatteryMs_ = 0;
  draw();
}

bool SettingsMenu::update() {
  if (settings_ == nullptr) return false;

  bool needRedraw = false;

  // B held -> save & exit.
  if (M5.BtnB.wasHold()) {
    return false;
  }
  // B short -> next setting.
  if (M5.BtnB.wasClicked()) {
    cursor_ = (cursor_ + 1) % kRowCount;
    needRedraw = true;
  }
  // A / C -> decrease / increase the selected value.
  if (M5.BtnA.wasPressed() || M5.BtnC.wasPressed()) {
    const int dir = M5.BtnC.wasPressed() ? +1 : -1;
    if (cursor_ == kRowSensitivity) {
      settings_->adjustSensitivity(dir);
    } else {
      settings_->adjustBrightness(dir);
      M5.Display.setBrightness(settings_->brightness);  // live preview
    }
    needRedraw = true;
  }

  // Periodically refresh the battery reading.
  const uint32_t now = millis();
  if (now - lastBatteryMs_ >= 5000) {
    lastBatteryMs_ = now;
    if (M5.Power.getBatteryLevel() != lastBatteryLevel_) {
      drawBattery();
    }
  }

  if (needRedraw) draw();
  return true;
}

void SettingsMenu::draw() {
  M5.Display.fillScreen(kColBg);

  M5.Display.setTextColor(kColTitle, kColBg);
  M5.Display.setTextDatum(textdatum_t::top_left);
  M5.Display.setTextSize(3);
  M5.Display.drawString("Settings", 20, 10);

  char val[16];

  snprintf(val, sizeof(val), "%.2fx", settings_->sensitivity);
  drawRow(kRowSensitivity, "Sensitivity", val, settings_->sensitivityFraction());

  const int pct = static_cast<int>(settings_->brightnessFraction() * 100.0f + 0.5f);
  snprintf(val, sizeof(val), "%d%%", pct);
  drawRow(kRowBrightness, "Brightness", val, settings_->brightnessFraction());

  // Bottom hint.
  M5.Display.setTextColor(kColHint, kColBg);
  M5.Display.setTextDatum(textdatum_t::bottom_center);
  M5.Display.setTextSize(1);
  M5.Display.drawString("A: -      B: next  /  hold = exit      C: +", kScreenW / 2, 234);

  drawBattery();
}

void SettingsMenu::drawRow(int row, const char* label, const char* value,
                           float fraction) {
  const bool selected = (row == cursor_);
  const int y = kRowY0 + row * kRowDY;

  // Selection marker.
  if (selected) {
    M5.Display.fillTriangle(10, y + 2, 10, y + 18, 20, y + 10, kColSel);
  }

  M5.Display.setTextSize(2);
  M5.Display.setTextColor(selected ? kColSel : kColLabel, kColBg);
  M5.Display.setTextDatum(textdatum_t::top_left);
  M5.Display.drawString(label, kBarX, y);

  M5.Display.setTextColor(kColValue, kColBg);
  M5.Display.setTextDatum(textdatum_t::top_right);
  M5.Display.drawString(value, kBarX + kBarW, y);

  // Progress bar.
  const int barY = y + 26;
  const float f = fraction < 0 ? 0 : (fraction > 1 ? 1 : fraction);
  M5.Display.fillRoundRect(kBarX, barY, kBarW, kBarH, 4, kColBarBg);
  const int fillW = static_cast<int>(kBarW * f);
  if (fillW > 0) {
    M5.Display.fillRoundRect(kBarX, barY, fillW, kBarH, 4,
                             selected ? kColSel : kColBarFill);
  }
}

void SettingsMenu::drawBattery() {
  int level = M5.Power.getBatteryLevel();  // 0..100 (-1 if unknown)
  lastBatteryLevel_ = level;
  if (level < 0) level = 0;
  const bool charging = M5.Power.isCharging();

  // Clear the area behind the icon + label.
  M5.Display.fillRect(kBattX - 70, kBattY - 2, 138, kBattH + 6, kColBg);

  // Percentage text left of the icon.
  char txt[8];
  snprintf(txt, sizeof(txt), "%d%%", level);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(kColLabel, kColBg);
  M5.Display.setTextDatum(textdatum_t::middle_right);
  M5.Display.drawString(txt, kBattX - 8, kBattY + kBattH / 2);

  // Battery outline + nub.
  M5.Display.drawRoundRect(kBattX, kBattY, kBattW, kBattH, 3, kColLabel);
  M5.Display.fillRect(kBattX + kBattW, kBattY + 6, 4, kBattH - 12, kColLabel);

  // Fill proportional to level; color by charge state / level.
  uint16_t fill = 0x07E0;  // green
  if (charging) {
    fill = 0x07FF;  // cyan while charging
  } else if (level <= 20) {
    fill = 0xF800;  // red when low
  } else if (level <= 40) {
    fill = 0xFD20;  // orange
  }
  const int innerW = kBattW - 6;
  const int w = (innerW * level) / 100;
  if (w > 0) {
    M5.Display.fillRect(kBattX + 3, kBattY + 3, w, kBattH - 6, fill);
  }
}
