// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include "color_spaces.hpp"
#include "ast_values.hpp"
#include "util.hpp"
#include <cmath>
#include <algorithm>

namespace Sass {

  // ============================================================================
  // HWB Conversions
  // ============================================================================
  
  Color_RGBA* HWB::toRGBA() const {
    // From https://www.w3.org/TR/css-color-4/#hwb-to-rgb
    double hue = std::fmod(h, 360.0);
    if (hue < 0) hue += 360.0;
    double scaledHue = hue / 360.0;
    
    double white = w / 100.0;
    double black = b / 100.0;
    
    // Normalize if sum > 1
    double sum = white + black;
    if (sum > 1.0) {
      white /= sum;
      black /= sum;
    }
    
    double factor = 1.0 - white - black;
    
    // Convert to RGB using HSL-like formula
    auto toRgb = [&](double hue) -> double {
      return ColorSpaces::hueToRgb(0, 1, hue) * factor + white;
    };
    
    double r = toRgb(scaledHue + 1.0 / 3.0) * 255.0;
    double g = toRgb(scaledHue) * 255.0;
    double b = toRgb(scaledHue - 1.0 / 3.0) * 255.0;
    
    return SASS_MEMORY_NEW(Color_RGBA, SourceSpan("[hwb]"), r, g, b, a);
  }
  
  HWB HWB::fromRGBA(const Color_RGBA* rgb) {
    double r = rgb->r() / 255.0;
    double g = rgb->g() / 255.0;
    double b = rgb->b() / 255.0;
    
    double max = std::max({r, g, b});
    double min = std::min({r, g, b});
    double chroma = max - min;
    
    double hue = 0;
    if (chroma != 0) {
      if (max == r) {
        hue = std::fmod((g - b) / chroma, 6.0);
      } else if (max == g) {
        hue = (b - r) / chroma + 2.0;
      } else {
        hue = (r - g) / chroma + 4.0;
      }
      hue *= 60.0;
      if (hue < 0) hue += 360.0;
    }
    
    double whiteness = min * 100.0;
    double blackness = (1.0 - max) * 100.0;
    
    return HWB(hue, whiteness, blackness, rgb->a());
  }
  
  // ============================================================================
  // Lab Conversions
  // ============================================================================
  
  Color_RGBA* Lab::toRGBA() const {
    // Lab -> XYZ D50
    double fy = (l + 16.0) / 116.0;
    double fx = a / 500.0 + fy;
    double fz = fy - b / 200.0;
    
    double xr = ColorSpaces::labFinv(fx);
    double yr = l > ColorSpaces::LAB_KAPPA * ColorSpaces::LAB_EPSILON 
                ? std::pow((l + 16.0) / 116.0, 3.0) 
                : l / ColorSpaces::LAB_KAPPA;
    double zr = ColorSpaces::labFinv(fz);
    
    double x = xr * ColorSpaces::D50[0];
    double y = yr * ColorSpaces::D50[1];
    double z = zr * ColorSpaces::D50[2];
    
    // XYZ D50 -> Linear sRGB
    auto linear = ColorSpaces::multiplyMatrix(ColorSpaces::XYZ_D50_TO_LINEAR_SRGB, x, y, z);
    
    // Linear sRGB -> sRGB (gamma encoding)
    double r = ColorSpaces::gammaEncode(linear[0]) * 255.0;
    double g = ColorSpaces::gammaEncode(linear[1]) * 255.0;
    double b = ColorSpaces::gammaEncode(linear[2]) * 255.0;
    
    // Clamp to valid range
    r = clip(r, 0.0, 255.0);
    g = clip(g, 0.0, 255.0);
    b = clip(b, 0.0, 255.0);
    
    return SASS_MEMORY_NEW(Color_RGBA, SourceSpan("[lab]"), r, g, b, alpha);
  }
  
  Lab Lab::fromRGBA(const Color_RGBA* rgb) {
    // sRGB -> Linear sRGB (gamma decoding)
    double r = ColorSpaces::gammaDecode(rgb->r() / 255.0);
    double g = ColorSpaces::gammaDecode(rgb->g() / 255.0);
    double b = ColorSpaces::gammaDecode(rgb->b() / 255.0);
    
    // Linear sRGB -> XYZ D50
    auto xyz = ColorSpaces::multiplyMatrix(ColorSpaces::LINEAR_SRGB_TO_XYZ_D50, r, g, b);
    
    // Normalize by D50 white point
    double xr = xyz[0] / ColorSpaces::D50[0];
    double yr = xyz[1] / ColorSpaces::D50[1];
    double zr = xyz[2] / ColorSpaces::D50[2];
    
    // XYZ D50 -> Lab
    double fx = ColorSpaces::labF(xr);
    double fy = ColorSpaces::labF(yr);
    double fz = ColorSpaces::labF(zr);
    
    double L = 116.0 * fy - 16.0;
    double a = 500.0 * (fx - fy);
    double b_val = 200.0 * (fy - fz);
    
    return Lab(L, a, b_val, rgb->a());
  }
  
  // ============================================================================
  // LCH Conversions
  // ============================================================================
  
  Color_RGBA* LCH::toRGBA() const {
    Lab lab = toLab();
    return lab.toRGBA();
  }
  
  LCH LCH::fromRGBA(const Color_RGBA* rgb) {
    Lab lab = Lab::fromRGBA(rgb);
    return fromLab(lab);
  }
  
  Lab LCH::toLab() const {
    double L, a, b;
    ColorSpaces::lchToLab(l, c, h, L, a, b);
    return Lab(L, a, b, alpha);
  }
  
  LCH LCH::fromLab(const Lab& lab) {
    double l, c, h;
    ColorSpaces::labToLch(lab.l, lab.a, lab.b, l, c, h);
    return LCH(l, c, h, lab.alpha);
  }
  
  // ============================================================================
  // OKLab Conversions
  // ============================================================================
  
  Color_RGBA* OKLab::toRGBA() const {
    // OKLab -> LMS
    auto lms = ColorSpaces::multiplyMatrix(ColorSpaces::OKLAB_TO_LMS, l, a, b);
    
    // Cube LMS values
    double lCubed = lms[0] * lms[0] * lms[0];
    double mCubed = lms[1] * lms[1] * lms[1];
    double sCubed = lms[2] * lms[2] * lms[2];
    
    // LMS -> Linear sRGB
    auto linear = ColorSpaces::multiplyMatrix(ColorSpaces::LMS_TO_LINEAR_SRGB, lCubed, mCubed, sCubed);
    
    // Linear sRGB -> sRGB (gamma encoding)
    double r = ColorSpaces::gammaEncode(linear[0]) * 255.0;
    double g = ColorSpaces::gammaEncode(linear[1]) * 255.0;
    double b = ColorSpaces::gammaEncode(linear[2]) * 255.0;
    
    // Clamp to valid range
    r = clip(r, 0.0, 255.0);
    g = clip(g, 0.0, 255.0);
    b = clip(b, 0.0, 255.0);
    
    return SASS_MEMORY_NEW(Color_RGBA, SourceSpan("[oklab]"), r, g, b, alpha);
  }
  
  OKLab OKLab::fromRGBA(const Color_RGBA* rgb) {
    // sRGB -> Linear sRGB (gamma decoding)
    double r = ColorSpaces::gammaDecode(rgb->r() / 255.0);
    double g = ColorSpaces::gammaDecode(rgb->g() / 255.0);
    double b = ColorSpaces::gammaDecode(rgb->b() / 255.0);
    
    // Linear sRGB -> LMS
    auto lms = ColorSpaces::multiplyMatrix(ColorSpaces::LINEAR_SRGB_TO_LMS, r, g, b);
    
    // Cube root LMS values
    double lCbrt = std::cbrt(lms[0]);
    double mCbrt = std::cbrt(lms[1]);
    double sCbrt = std::cbrt(lms[2]);
    
    // LMS -> OKLab
    auto oklab = ColorSpaces::multiplyMatrix(ColorSpaces::LMS_TO_OKLAB, lCbrt, mCbrt, sCbrt);
    
    return OKLab(oklab[0], oklab[1], oklab[2], rgb->a());
  }
  
  // ============================================================================
  // OKLCH Conversions
  // ============================================================================
  
  Color_RGBA* OKLCH::toRGBA() const {
    OKLab oklab = toOKLab();
    return oklab.toRGBA();
  }
  
  OKLCH OKLCH::fromRGBA(const Color_RGBA* rgb) {
    OKLab oklab = OKLab::fromRGBA(rgb);
    return fromOKLab(oklab);
  }
  
  OKLab OKLCH::toOKLab() const {
    double hRad = h * M_PI / 180.0;
    double a = c * std::cos(hRad);
    double b = c * std::sin(hRad);
    return OKLab(l, a, b, alpha);
  }
  
  OKLCH OKLCH::fromOKLab(const OKLab& oklab) {
    double c = std::sqrt(oklab.a * oklab.a + oklab.b * oklab.b);
    double h = std::atan2(oklab.b, oklab.a) * 180.0 / M_PI;
    if (h < 0) h += 360.0;
    return OKLCH(oklab.l, c, h, oklab.alpha);
  }
  
} // namespace Sass

