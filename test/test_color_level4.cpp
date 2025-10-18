#include "../src/sass.hpp"
#include "../src/ast.hpp"
#include "../src/context.hpp"
#include "../src/parser.hpp"
#include "../src/eval.hpp"
#include "../src/color_spaces.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#define ASSERT_TRUE(cond) \
  if (!(cond)) { \
    std::cerr << "Expected condition to be true at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

#define ASSERT_FALSE(cond) \
  ASSERT_TRUE(!(cond))

#define ASSERT_NEAR(a, b, epsilon) \
  if (std::abs((a) - (b)) > (epsilon)) { \
    std::cerr << "Expected " << (a) << " to be near " << (b) \
              << " (epsilon=" << (epsilon) << ") at " \
              << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

#define ASSERT_EQ(a, b) \
  if ((a) != (b)) { \
    std::cerr << "Expected " << (a) << " == " << (b) \
              << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

using namespace Sass;

bool TestHWBBasicRed() {
  HWB hwb(0, 0, 0, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  ASSERT_NEAR(rgb->a(), 1.0, 0.01);
  return true;
}

bool TestHWBBasicGreen() {
  HWB hwb(120, 0, 0, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 255.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  ASSERT_NEAR(rgb->a(), 1.0, 0.01);
  return true;
}

bool TestHWBBasicBlue() {
  HWB hwb(240, 0, 0, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 255.0, 1.0);
  ASSERT_NEAR(rgb->a(), 1.0, 0.01);
  return true;
}

bool TestHWBWithWhiteness() {
  HWB hwb(0, 50, 0, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_TRUE(rgb->r() > 128);
  ASSERT_TRUE(rgb->g() > 100);
  ASSERT_TRUE(rgb->b() > 100);
  return true;
}

bool TestHWBWithBlackness() {
  HWB hwb(0, 0, 50, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_TRUE(rgb->r() < 130);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  return true;
}

bool TestHWBWithAlpha() {
  HWB hwb(180, 20, 10, 0.5);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_NEAR(rgb->a(), 0.5, 0.01);
  return true;
}

bool TestLabBlack() {
  Lab lab(0, 0, 0, 1.0);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  return true;
}

bool TestLabWhite() {
  Lab lab(100, 0, 0, 1.0);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 255.0, 1.0);
  ASSERT_NEAR(rgb->b(), 255.0, 1.0);
  return true;
}

bool TestLabGray() {
  Lab lab(50, 0, 0, 1.0);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_TRUE(rgb->r() > 100 && rgb->r() < 150);
  ASSERT_TRUE(rgb->g() > 100 && rgb->g() < 150);
  ASSERT_TRUE(rgb->b() > 100 && rgb->b() < 150);
  return true;
}

bool TestLabWithColor() {
  Lab lab(50, 50, 25, 1.0);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_TRUE(rgb->r() > 0);
  ASSERT_TRUE(rgb->g() > 0);
  ASSERT_TRUE(rgb->b() > 0);
  return true;
}

bool TestLabWithAlpha() {
  Lab lab(60, 30, -40, 0.8);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_NEAR(rgb->a(), 0.8, 0.01);
  return true;
}

bool TestLCHBlack() {
  LCH lch(0, 0, 0, 1.0);
  Color_RGBA* rgb = lch.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  return true;
}

bool TestLCHWhite() {
  LCH lch(100, 0, 0, 1.0);
  Color_RGBA* rgb = lch.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 255.0, 1.0);
  ASSERT_NEAR(rgb->b(), 255.0, 1.0);
  return true;
}

bool TestLCHWithColor() {
  LCH lch(50, 50, 40, 1.0);
  Color_RGBA* rgb = lch.toRGBA();
  ASSERT_TRUE(rgb->r() > 0);
  ASSERT_TRUE(rgb->g() > 0);
  ASSERT_TRUE(rgb->b() > 0);
  return true;
}

bool TestLCHWithAlpha() {
  LCH lch(70, 40, 220, 0.9);
  Color_RGBA* rgb = lch.toRGBA();
  ASSERT_NEAR(rgb->a(), 0.9, 0.01);
  return true;
}

bool TestLCHToLabConversion() {
  LCH lch(50, 50, 40, 1.0);
  Lab lab = lch.toLab();
  ASSERT_NEAR(lab.l, 50.0, 0.1);
  return true;
}

bool TestLabToLCHConversion() {
  Lab lab(50, 30, 40, 1.0);
  LCH lch = LCH::fromLab(lab);
  ASSERT_NEAR(lch.l, 50.0, 0.1);
  return true;
}

bool TestOKLabBlack() {
  OKLab oklab(0, 0, 0, 1.0);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  return true;
}

bool TestOKLabWhite() {
  OKLab oklab(1.0, 0, 0, 1.0);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 255.0, 1.0);
  ASSERT_NEAR(rgb->b(), 255.0, 1.0);
  return true;
}

bool TestOKLabGray() {
  OKLab oklab(0.5, 0, 0, 1.0);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_TRUE(rgb->r() > 50 && rgb->r() < 200);
  ASSERT_TRUE(rgb->g() > 50 && rgb->g() < 200);
  ASSERT_TRUE(rgb->b() > 50 && rgb->b() < 200);
  return true;
}

bool TestOKLabWithColor() {
  OKLab oklab(0.6, 0.1, -0.1, 1.0);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_TRUE(rgb->r() > 0);
  ASSERT_TRUE(rgb->g() > 0);
  ASSERT_TRUE(rgb->b() > 0);
  return true;
}

bool TestOKLabWithAlpha() {
  OKLab oklab(0.7, -0.05, 0.08, 0.75);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_NEAR(rgb->a(), 0.75, 0.01);
  return true;
}

bool TestOKLCHBlack() {
  OKLCH oklch(0, 0, 0, 1.0);
  Color_RGBA* rgb = oklch.toRGBA();
  ASSERT_NEAR(rgb->r(), 0.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  return true;
}

bool TestOKLCHWhite() {
  OKLCH oklch(1.0, 0, 0, 1.0);
  Color_RGBA* rgb = oklch.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 255.0, 1.0);
  ASSERT_NEAR(rgb->b(), 255.0, 1.0);
  return true;
}

bool TestOKLCHWithColor() {
  OKLCH oklch(0.6, 0.15, 50, 1.0);
  Color_RGBA* rgb = oklch.toRGBA();
  ASSERT_TRUE(rgb->r() > 0);
  ASSERT_TRUE(rgb->g() > 0);
  ASSERT_TRUE(rgb->b() > 0);
  return true;
}

bool TestOKLCHWithAlpha() {
  OKLCH oklch(0.65, 0.12, 150, 0.85);
  Color_RGBA* rgb = oklch.toRGBA();
  ASSERT_NEAR(rgb->a(), 0.85, 0.01);
  return true;
}

bool TestOKLCHToOKLabConversion() {
  OKLCH oklch(0.6, 0.15, 50, 1.0);
  OKLab oklab = oklch.toOKLab();
  ASSERT_NEAR(oklab.l, 0.6, 0.01);
  return true;
}

bool TestOKLabToOKLCHConversion() {
  OKLab oklab(0.6, 0.1, 0.1, 1.0);
  OKLCH oklch = OKLCH::fromOKLab(oklab);
  ASSERT_NEAR(oklch.l, 0.6, 0.01);
  return true;
}

bool TestGammaEncode() {
  double linear = 0.5;
  double encoded = ColorSpaces::gammaEncode(linear);
  ASSERT_TRUE(encoded > 0.7 && encoded < 0.8);
  return true;
}

bool TestGammaDecode() {
  double encoded = 0.5;
  double linear = ColorSpaces::gammaDecode(encoded);
  ASSERT_TRUE(linear > 0.2 && linear < 0.25);
  return true;
}

bool TestGammaRoundTrip() {
  double original = 0.5;
  double encoded = ColorSpaces::gammaEncode(original);
  double decoded = ColorSpaces::gammaDecode(encoded);
  ASSERT_NEAR(original, decoded, 0.01);
  return true;
}

bool TestLabFFunction() {
  double t = 0.5;
  double result = ColorSpaces::labF(t);
  ASSERT_TRUE(result > 0.7 && result < 0.8);
  return true;
}

bool TestLabFinvFunction() {
  double t = 0.5;
  double result = ColorSpaces::labFinv(t);
  ASSERT_TRUE(result > 0.1 && result < 0.2);
  return true;
}

bool TestLabRoundTrip() {
  double original = 0.5;
  double f = ColorSpaces::labF(original);
  double finv = ColorSpaces::labFinv(f);
  ASSERT_NEAR(original, finv, 0.01);
  return true;
}

bool TestMatrixMultiply() {
  double matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
  auto result = ColorSpaces::multiplyMatrix(matrix, 1.0, 2.0, 3.0);
  ASSERT_NEAR(result[0], 1.0, 0.001);
  ASSERT_NEAR(result[1], 2.0, 0.001);
  ASSERT_NEAR(result[2], 3.0, 0.001);
  return true;
}

bool TestHueToRgbBasic() {
  double result = ColorSpaces::hueToRgb(0, 1, 0.5);
  ASSERT_NEAR(result, 1.0, 0.01);
  return true;
}

bool TestHWBConversion() {
  HWB hwb(0, 0, 0, 1.0);
  Color_RGBA* rgb = hwb.toRGBA();
  ASSERT_NEAR(rgb->r(), 255.0, 1.0);
  ASSERT_NEAR(rgb->g(), 0.0, 1.0);
  ASSERT_NEAR(rgb->b(), 0.0, 1.0);
  delete rgb;
  return true;
}

bool TestLabConversion() {
  Lab lab(50, 0, 0, 1.0);
  Color_RGBA* rgb = lab.toRGBA();
  ASSERT_TRUE(rgb->r() > 100 && rgb->r() < 150);
  ASSERT_TRUE(rgb->g() > 100 && rgb->g() < 150);
  ASSERT_TRUE(rgb->b() > 100 && rgb->b() < 150);
  delete rgb;
  return true;
}

bool TestLCHConversion() {
  LCH lch(50, 0, 0, 1.0);
  Color_RGBA* rgb = lch.toRGBA();
  ASSERT_TRUE(rgb->r() > 100 && rgb->r() < 150);
  ASSERT_TRUE(rgb->g() > 100 && rgb->g() < 150);
  ASSERT_TRUE(rgb->b() > 100 && rgb->b() < 150);
  delete rgb;
  return true;
}

bool TestOKLabConversion() {
  OKLab oklab(0.5, 0, 0, 1.0);
  Color_RGBA* rgb = oklab.toRGBA();
  ASSERT_TRUE(rgb->r() > 50 && rgb->r() < 200);
  ASSERT_TRUE(rgb->g() > 50 && rgb->g() < 200);
  ASSERT_TRUE(rgb->b() > 50 && rgb->b() < 200);
  delete rgb;
  return true;
}

bool TestOKLCHConversion() {
  OKLCH oklch(0.5, 0, 0, 1.0);
  Color_RGBA* rgb = oklch.toRGBA();
  ASSERT_TRUE(rgb->r() > 50 && rgb->r() < 200);
  ASSERT_TRUE(rgb->g() > 50 && rgb->g() < 200);
  ASSERT_TRUE(rgb->b() > 50 && rgb->b() < 200);
  delete rgb;
  return true;
}

#define TEST(fn) \
  if (fn()) { \
    passed.push_back(#fn); \
  } else { \
    failed.push_back(#fn); \
    std::cerr << "Failed: " #fn << std::endl; \
  }

int main(int argc, char **argv) {
  std::vector<std::string> passed;
  std::vector<std::string> failed;

  TEST(TestHWBBasicRed);
  TEST(TestHWBBasicGreen);
  TEST(TestHWBBasicBlue);
  TEST(TestHWBWithWhiteness);
  TEST(TestHWBWithBlackness);
  TEST(TestHWBWithAlpha);

  TEST(TestLabBlack);
  TEST(TestLabWhite);
  TEST(TestLabGray);
  TEST(TestLabWithColor);
  TEST(TestLabWithAlpha);

  TEST(TestLCHBlack);
  TEST(TestLCHWhite);
  TEST(TestLCHWithColor);
  TEST(TestLCHWithAlpha);
  TEST(TestLCHToLabConversion);
  TEST(TestLabToLCHConversion);

  TEST(TestOKLabBlack);
  TEST(TestOKLabWhite);
  TEST(TestOKLabGray);
  TEST(TestOKLabWithColor);
  TEST(TestOKLabWithAlpha);

  TEST(TestOKLCHBlack);
  TEST(TestOKLCHWhite);
  TEST(TestOKLCHWithColor);
  TEST(TestOKLCHWithAlpha);
  TEST(TestOKLCHToOKLabConversion);
  TEST(TestOKLabToOKLCHConversion);

  TEST(TestGammaEncode);
  TEST(TestGammaDecode);
  TEST(TestGammaRoundTrip);

  TEST(TestLabFFunction);
  TEST(TestLabFinvFunction);
  TEST(TestLabRoundTrip);

  TEST(TestMatrixMultiply);
  TEST(TestHueToRgbBasic);

  TEST(TestHWBConversion);
  TEST(TestLabConversion);
  TEST(TestLCHConversion);
  TEST(TestOKLabConversion);
  TEST(TestOKLCHConversion);

  std::cerr << argv[0] << ": Passed: " << passed.size()
            << ", failed: " << failed.size()
            << "." << std::endl;
  return failed.size();
}
