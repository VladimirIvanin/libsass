#ifndef SASS_COLOR_SPACES_H
#define SASS_COLOR_SPACES_H

#include "sass.hpp"
#include <cmath>
#include <array>

namespace Sass {

  // Forward declarations
  class Color;
  class Color_RGBA;
  class Color_HSLA;

  // Color space constants for CSS Color Level 4
  namespace ColorSpaces {
    
    // Constants from https://www.w3.org/TR/css-color-4/
    constexpr double D50[3] = {0.3457 / 0.3585, 1.00000, (1.0 - 0.3457 - 0.3585) / 0.3585};
    
    // Lab constants
    constexpr double LAB_KAPPA = 24389.0 / 27.0;  // 29^3/3^3
    constexpr double LAB_EPSILON = 216.0 / 24389.0;  // 6^3/29^3
    
    // Conversion matrices
    // Matrix for converting linear sRGB to XYZ D50
    constexpr double LINEAR_SRGB_TO_XYZ_D50[9] = {
      0.43606574687426936, 0.38515150959015960, 0.14307841996513868,
      0.22249317711056518, 0.71688701309448240, 0.06061980979495235,
      0.01392392146316939, 0.09708132423141015, 0.71409935681588070
    };
    
    // Matrix for converting XYZ D50 to linear sRGB
    constexpr double XYZ_D50_TO_LINEAR_SRGB[9] = {
      3.13413585290011780, -1.61738599801804200, -0.49066221791109754,
      -0.97879547655577770, 1.91625437739598840, 0.03344287339036693,
      0.07195539255794733, -0.22897675981518200, 1.40538603511311820
    };
    
    // Matrix for converting linear sRGB to XYZ D65
    constexpr double LINEAR_SRGB_TO_XYZ_D65[9] = {
      0.41239079926595950, 0.35758433938387796, 0.18048078840183430,
      0.21263900587151036, 0.71516867876775590, 0.07219231536073371,
      0.01933081871559185, 0.11919477979462598, 0.95053215224966060
    };
    
    // Matrix for converting XYZ D65 to linear sRGB
    constexpr double XYZ_D65_TO_LINEAR_SRGB[9] = {
      3.24096994190452130, -1.53738317757009350, -0.49861076029300330,
      -0.96924363628087980, 1.87596750150772060, 0.04155505740717561,
      0.05563007969699360, -0.20397695888897657, 1.05697151424287860
    };
    
    // Matrix for converting linear sRGB to LMS
    constexpr double LINEAR_SRGB_TO_LMS[9] = {
      0.41222146947076300, 0.53633253726173480, 0.05144599326750220,
      0.21190349581782520, 0.68069955064523420, 0.10739695353694050,
      0.08830245919005641, 0.28171883913612150, 0.62997870167382210
    };
    
    // Matrix for converting LMS to linear sRGB
    constexpr double LMS_TO_LINEAR_SRGB[9] = {
      4.07674163607595800, -3.30771153925806200, 0.23096990318210417,
      -1.26843797328503200, 2.60975734928768900, -0.34131937600265710,
      -0.00419607613867551, -0.70341861793593630, 1.70761469407461200
    };
    
    // Matrix for converting LMS to OKLab
    constexpr double LMS_TO_OKLAB[9] = {
      0.21045426830931400, 0.79361777470230540, -0.00407204301161930,
      1.97799853243116840, -2.42859224204858000, 0.45059370961741100,
      0.02590404246554780, 0.78277171245752960, -0.80867575492307740
    };
    
    // Matrix for converting OKLab to LMS
    constexpr double OKLAB_TO_LMS[9] = {
      1.00000000000000020, 0.39633777737617490, 0.21580375730991360,
      0.99999999999999980, -0.10556134581565854, -0.06385417282581334,
      0.99999999999999990, -0.08948417752981180, -1.29148554801940940
    };
    
    // Utility functions for matrix multiplication
    inline std::array<double, 3> multiplyMatrix(const double matrix[9], double a, double b, double c) {
      return {
        matrix[0] * a + matrix[1] * b + matrix[2] * c,
        matrix[3] * a + matrix[4] * b + matrix[5] * c,
        matrix[6] * a + matrix[7] * b + matrix[8] * c
      };
    }
    
    // sRGB gamma encoding/decoding
    inline double gammaEncode(double linear) {
      if (linear <= 0.0031308) {
        return 12.92 * linear;
      }
      return 1.055 * std::pow(linear, 1.0 / 2.4) - 0.055;
    }
    
    inline double gammaDecode(double encoded) {
      if (encoded <= 0.04045) {
        return encoded / 12.92;
      }
      return std::pow((encoded + 0.055) / 1.055, 2.4);
    }
    
    // HWB conversion helpers
    inline double hueToRgb(double m1, double m2, double h) {
      h = h < 0 ? h + 1 : (h > 1 ? h - 1 : h);
      if (h * 6 < 1) return m1 + (m2 - m1) * h * 6;
      if (h * 2 < 1) return m2;
      if (h * 3 < 2) return m1 + (m2 - m1) * (2.0 / 3.0 - h) * 6;
      return m1;
    }
    
    // Lab conversion helpers
    inline double labF(double t) {
      if (t > LAB_EPSILON) {
        return std::cbrt(t);
      }
      return (LAB_KAPPA * t + 16) / 116;
    }
    
    inline double labFinv(double t) {
      double cubed = t * t * t;
      if (cubed > LAB_EPSILON) {
        return cubed;
      }
      return (116 * t - 16) / LAB_KAPPA;
    }
    
    // LCH/Lab conversion
    inline void labToLch(double L, double a, double b, double& l, double& c, double& h) {
      l = L;
      c = std::sqrt(a * a + b * b);
      h = std::atan2(b, a) * 180.0 / M_PI;
      if (h < 0) h += 360;
    }
    
    inline void lchToLab(double l, double c, double h, double& L, double& a, double& b) {
      L = l;
      double hRad = h * M_PI / 180.0;
      a = c * std::cos(hRad);
      b = c * std::sin(hRad);
    }
    
  } // namespace ColorSpaces
  
  // New color classes for CSS Color Level 4
  
  // HWB Color
  struct HWB {
    double h;  // Hue (0-360)
    double w;  // Whiteness (0-100)
    double b;  // Blackness (0-100)
    double a;  // Alpha (0-1)
    
    HWB(double hue = 0, double white = 0, double black = 0, double alpha = 1.0)
      : h(hue), w(white), b(black), a(alpha) {}
      
    Color_RGBA* toRGBA() const;
    static HWB fromRGBA(const Color_RGBA* rgb);
  };
  
  // Lab Color
  struct Lab {
    double l;  // Lightness (0-100)
    double a;  // a-axis (-125 to 125)
    double b;  // b-axis (-125 to 125)
    double alpha;  // Alpha (0-1)
    
    Lab(double lightness = 0, double aAxis = 0, double bAxis = 0, double alphaVal = 1.0)
      : l(lightness), a(aAxis), b(bAxis), alpha(alphaVal) {}
      
    Color_RGBA* toRGBA() const;
    static Lab fromRGBA(const Color_RGBA* rgb);
  };
  
  // LCH Color
  struct LCH {
    double l;  // Lightness (0-100)
    double c;  // Chroma (0-150)
    double h;  // Hue (0-360)
    double alpha;  // Alpha (0-1)
    
    LCH(double lightness = 0, double chroma = 0, double hue = 0, double alphaVal = 1.0)
      : l(lightness), c(chroma), h(hue), alpha(alphaVal) {}
      
    Color_RGBA* toRGBA() const;
    static LCH fromRGBA(const Color_RGBA* rgb);
    Lab toLab() const;
    static LCH fromLab(const Lab& lab);
  };
  
  // OKLab Color
  struct OKLab {
    double l;  // Lightness (0-1)
    double a;  // a-axis (-0.4 to 0.4)
    double b;  // b-axis (-0.4 to 0.4)
    double alpha;  // Alpha (0-1)
    
    OKLab(double lightness = 0, double aAxis = 0, double bAxis = 0, double alphaVal = 1.0)
      : l(lightness), a(aAxis), b(bAxis), alpha(alphaVal) {}
      
    Color_RGBA* toRGBA() const;
    static OKLab fromRGBA(const Color_RGBA* rgb);
  };
  
  // OKLCH Color
  struct OKLCH {
    double l;  // Lightness (0-1)
    double c;  // Chroma (0-0.4)
    double h;  // Hue (0-360)
    double alpha;  // Alpha (0-1)
    
    OKLCH(double lightness = 0, double chroma = 0, double hue = 0, double alphaVal = 1.0)
      : l(lightness), c(chroma), h(hue), alpha(alphaVal) {}
      
    Color_RGBA* toRGBA() const;
    static OKLCH fromRGBA(const Color_RGBA* rgb);
    OKLab toOKLab() const;
    static OKLCH fromOKLab(const OKLab& oklab);
  };
  
} // namespace Sass

#endif

