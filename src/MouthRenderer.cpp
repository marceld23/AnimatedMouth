#include "MouthRenderer.h"

#include <math.h>

namespace {
// Display is 320 x 240 (rotation 1). The mouth is centered.
constexpr int kScreenW = 320;
constexpr int kScreenH = 240;
constexpr int kCx = 160;
constexpr int kCy = 122;

// Mouth geometry (pixels).
constexpr float kMaxHalfW = 132.0f;   // half-width at width = 1.0
constexpr float kOpenTopMax = 42.0f;  // upper aperture travel at open = 1.0
constexpr float kOpenBotMax = 64.0f;  // lower aperture travel (lower jaw drops more)
constexpr float kUpperLipThk = 24.0f; // upper lip thickness at center
constexpr float kLowerLipThk = 32.0f; // lower lip thickness at center (fuller)
constexpr float kBowAmp = 9.0f;       // cupid's-bow central dip depth
constexpr float kBowSigma2 = 0.020f;  // cupid's-bow narrowness (smaller = sharper)

// --- Tiny integer-RGB helper (interpolation in 888, output 565) ------------
struct RGB {
  int r, g, b;
};

inline uint16_t to565(const RGB& c) {
  const int r = c.r < 0 ? 0 : c.r > 255 ? 255 : c.r;
  const int g = c.g < 0 ? 0 : c.g > 255 ? 255 : c.g;
  const int b = c.b < 0 ? 0 : c.b > 255 ? 255 : c.b;
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

// t in 0..256
inline RGB mix(const RGB& a, const RGB& b, int t) {
  return {a.r + (((b.r - a.r) * t) >> 8), a.g + (((b.g - a.g) * t) >> 8),
          a.b + (((b.b - a.b) * t) >> 8)};
}

// Multiply a color by a 0..256 brightness factor (cheap shading).
inline RGB shade(const RGB& c, int f) {
  return {(c.r * f) >> 8, (c.g * f) >> 8, (c.b * f) >> 8};
}

// --- Palette ----------------------------------------------------------------
constexpr RGB kBg = {0, 0, 0};                 // black background
constexpr RGB kLipDark = {120, 38, 52};        // lip edge / shadow
constexpr RGB kLipMid = {178, 70, 84};         // lip body
constexpr RGB kLipHi = {226, 132, 140};        // lip highlight ridge
constexpr RGB kLipCrease = {86, 26, 38};       // closed-lip seam
constexpr RGB kCavity = {44, 12, 20};          // dark interior
constexpr RGB kTeeth = {238, 236, 226};        // enamel
constexpr RGB kTeethShadow = {150, 150, 150};  // tooth gap / shaded enamel
constexpr RGB kTongue = {196, 92, 104};        // tongue body
constexpr RGB kTongueDark = {150, 58, 72};     // tongue groove

inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

// Vertical lip gradient. t256 = 0 at outer edge .. 256 at inner (aperture) edge.
RGB lipColor(bool upper, int t256) {
  if (upper) {
    // outer(dark) -> highlight ridge -> inner(mid, slight shadow at opening)
    if (t256 < 120) return mix(kLipDark, kLipHi, (t256 * 256) / 120);
    return mix(kLipHi, kLipMid, ((t256 - 120) * 256) / 136);
  }
  // lower lip: inner(top, mid) -> bright highlight -> outer(bottom, dark)
  if (t256 < 130) return mix(kLipMid, kLipHi, (t256 * 256) / 130);
  return mix(kLipHi, kLipDark, ((t256 - 130) * 256) / 126);
}
}  // namespace

bool MouthRenderer::begin() {
  canvas_.setPsram(true);
  canvas_.setColorDepth(16);
  if (!canvas_.createSprite(kScreenW, kScreenH)) {
    return false;
  }
  current_ = MouthShape{0.0f, 0.9f, 0.0f};
  target_ = current_;
  drawFace();
  return true;
}

void MouthRenderer::setTarget(Viseme v) { target_ = targetFor(v); }

MouthRenderer::MouthShape MouthRenderer::targetFor(Viseme v) {
  switch (v) {
    case Viseme::Closed:  return {0.00f, 0.92f, 0.0f};
    case Viseme::Small:   return {0.34f, 0.82f, 0.0f};
    case Viseme::OpenA:   return {1.00f, 0.80f, 0.15f};
    case Viseme::RoundOU: return {0.90f, 0.40f, 0.0f};
    case Viseme::WideIE:  return {0.66f, 1.00f, 0.7f};
    case Viseme::TeethS:  return {0.42f, 0.96f, 1.0f};
  }
  return {0.0f, 0.9f, 0.0f};
}

void MouthRenderer::render() {
  current_.open += (target_.open - current_.open) * kSmoothing;
  current_.width += (target_.width - current_.width) * kSmoothing;
  current_.teeth += (target_.teeth - current_.teeth) * kSmoothing;
  drawFace();
}

void MouthRenderer::drawFace() {
  canvas_.fillScreen(to565(kBg));

  const float halfW = kMaxHalfW * current_.width;
  if (halfW < 6.0f) {
    canvas_.pushSprite(0, 0);
    return;
  }
  const float open = current_.open;
  const float teeth = current_.teeth;
  const int iHalfW = static_cast<int>(halfW);

  for (int xi = -iHalfW; xi <= iHalfW; ++xi) {
    const int xcol = kCx + xi;
    if (xcol < 0 || xcol >= kScreenW) continue;

    const float u = xi / halfW;            // -1 .. 1
    const float parab = 1.0f - u * u;      // 1 at center, 0 at corners
    if (parab <= 0.0f) continue;
    const float taper = sqrtf(parab);      // softer taper for lip thickness

    // Aperture (opening) edges.
    const float At = kOpenTopMax * open * parab;
    const float Ab = kOpenBotMax * open * parab;
    // Cupid's bow: a central downward dip on the upper-lip outer contour.
    const float bow = kBowAmp * expf(-(u * u) / (2.0f * kBowSigma2));

    const float upThk = kUpperLipThk * taper;
    const float loThk = kLowerLipThk * taper;

    const int yUI = static_cast<int>(kCy - At);            // upper inner (aperture top)
    const int yUO = static_cast<int>(kCy - At - upThk + bow);  // upper outer (lip top)
    const int yLI = static_cast<int>(kCy + Ab);            // lower inner (aperture bottom)
    const int yLO = static_cast<int>(kCy + Ab + loThk);    // lower outer (lip bottom)

    // Column brightness: sides a touch darker for roundness.
    const int csCol = 256 - static_cast<int>(fabsf(u) * 54.0f);

    // --- Upper lip band -----------------------------------------------------
    const int upH = yUI - yUO;
    if (upH > 0) {
      for (int y = yUO; y < yUI; ++y) {
        const int t = ((y - yUO) << 8) / upH;
        canvas_.drawPixel(xcol, y, to565(shade(lipColor(true, t), csCol)));
      }
    }

    // --- Aperture (teeth / tongue / cavity) --------------------------------
    const int openH = yLI - yUI;
    if (openH >= 3) {
      // Base dark cavity.
      canvas_.drawFastVLine(xcol, yUI, openH, to565(kCavity));

      // Upper teeth band from the top of the aperture.
      if (teeth > 0.05f) {
        int th = static_cast<int>(openH * 0.46f * teeth);
        if (th > 0) {
          for (int y = yUI; y < yUI + th; ++y) {
            // Enamel: slightly brighter at top, darker toward the biting edge.
            const int tt = ((y - yUI) << 8) / th;
            RGB enamel = mix(kTeeth, kTeethShadow, tt / 2);
            // Tooth gaps: thin shadow every ~18 px.
            if (((xcol + 9) % 18) < 2) enamel = kTeethShadow;
            canvas_.drawPixel(xcol, y, to565(shade(enamel, csCol)));
          }
        }
      }

      // Tongue: rounded form filling the lower-center of the aperture.
      if (fabsf(u) < 0.80f) {
        const float tp = sqrtf(clampf(1.0f - (u / 0.80f) * (u / 0.80f), 0.0f, 1.0f));
        const int tongueH = static_cast<int>(openH * 0.52f * tp);
        if (tongueH > 1) {
          const int yTop = yLI - tongueH;
          for (int y = yTop; y < yLI; ++y) {
            const int tt = ((y - yTop) << 8) / tongueH;
            // Body brighter at the tip (top), darker toward the back/edges.
            RGB col = mix(kTongue, kTongueDark, tt / 2);
            // Central groove.
            if (fabsf(u) < 0.05f) col = kTongueDark;
            canvas_.drawPixel(xcol, y, to565(shade(col, csCol)));
          }
        }
      }
    } else if (openH >= 0) {
      // Lips touching: draw the lip crease seam.
      canvas_.drawFastVLine(xcol, yUI - 1, 2, to565(shade(kLipCrease, csCol)));
    }

    // --- Lower lip band -----------------------------------------------------
    const int loH = yLO - yLI;
    if (loH > 0) {
      for (int y = yLI; y < yLO; ++y) {
        const int t = ((y - yLI) << 8) / loH;
        canvas_.drawPixel(xcol, y, to565(shade(lipColor(false, t), csCol)));
      }
    }
  }

  canvas_.pushSprite(0, 0);
}
