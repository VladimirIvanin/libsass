#include "../src/sass.hpp"
#include "../src/context.hpp"
#include "../src/parser.hpp"
#include "../src/eval.hpp"
#include "../src/fn_colors.hpp"

#include <iostream>
#include <string>
#include <vector>

#define ASSERT_TRUE(cond) \
  if (!(cond)) { \
    std::cerr << "Expected condition to be true at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

#define ASSERT_FALSE(cond) \
  ASSERT_TRUE(!(cond))

#define ASSERT_STR_CONTAINS(str, substr) \
  if ((str).find(substr) == std::string::npos) { \
    std::cerr << "Expected \"" << (str) << "\" to contain \"" << (substr) << "\" at " \
              << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

using namespace Sass;

sass::string compile_scss(const sass::string& input) {
  Sass_Data_Context* ctx = sass_make_data_context((char*)input.c_str());
  sass_compile_data_context(ctx);
  int status = sass_context_get_error_status((Sass_Context*)ctx);
  if (status) {
    sass::string error = sass_context_get_error_message((Sass_Context*)ctx);
    sass_delete_data_context(ctx);
    return "ERROR: " + error;
  }
  sass::string output = sass_context_get_output_string((Sass_Context*)ctx);
  sass_delete_data_context(ctx);
  return output;
}

bool TestRGBWithAlpha() {
  sass::string input = ".test { color: rgb(255, 0, 0, 0.5); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_TRUE(output.find("rgba") != sass::string::npos || output.find("0.5") != sass::string::npos);
  return true;
}

bool TestHSLWithAlpha() {
  sass::string input = ".test { color: hsl(120, 100%, 50%, 0.7); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_TRUE(output.find("rgba") != sass::string::npos || output.find("0.7") != sass::string::npos);
  return true;
}

bool TestHWBFunction() {
  sass::string input = ".test { color: hwb(0, 0%, 0%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHWBWithWhiteness() {
  sass::string input = ".test { color: hwb(0, 50%, 0%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHWBWithBlackness() {
  sass::string input = ".test { color: hwb(0, 0%, 50%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHWBWithAlpha() {
  sass::string input = ".test { color: hwb(180, 20%, 10%, 0.5); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLabFunction() {
  sass::string input = ".test { color: lab(50%, 20, -30); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLabBlack() {
  sass::string input = ".test { color: lab(0%, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLabWhite() {
  sass::string input = ".test { color: lab(100%, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLabWithAlpha() {
  sass::string input = ".test { color: lab(60%, 30, -40, 0.8); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLCHFunction() {
  sass::string input = ".test { color: lch(50%, 50, 40); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLCHBlack() {
  sass::string input = ".test { color: lch(0%, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLCHWhite() {
  sass::string input = ".test { color: lch(100%, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestLCHWithAlpha() {
  sass::string input = ".test { color: lch(70%, 40, 220, 0.9); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLabFunction() {
  sass::string input = ".test { color: oklab(0.6, 0.1, -0.1); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLabBlack() {
  sass::string input = ".test { color: oklab(0, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLabWhite() {
  sass::string input = ".test { color: oklab(1, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLabWithAlpha() {
  sass::string input = ".test { color: oklab(0.7, -0.05, 0.08, 0.75); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLCHFunction() {
  sass::string input = ".test { color: oklch(0.6, 0.15, 50); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLCHBlack() {
  sass::string input = ".test { color: oklch(0, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLCHWhite() {
  sass::string input = ".test { color: oklch(1, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestOKLCHWithAlpha() {
  sass::string input = ".test { color: oklch(0.65, 0.12, 150, 0.85); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestColorFunctionSRGB() {
  sass::string input = ".test { color: color(srgb, 1, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestColorFunctionSRGBWithAlpha() {
  sass::string input = ".test { color: color(srgb, 0.8, 0.2, 0.3, 0.6); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestVariableWithHWB() {
  sass::string input = "$primary: hwb(200, 30%, 20%); .test { color: $primary; }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestVariableWithLab() {
  sass::string input = "$accent: lab(65%, 20, -30); .test { color: $accent; }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestVariableWithOKLCH() {
  sass::string input = "$highlight: oklch(0.7, 0.1, 180); .test { color: $highlight; }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBSpaceSyntax() {
  sass::string input = ".test { color: rgb(100 150 200); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBSpaceWithAlpha() {
  sass::string input = ".test { color: rgb(50 100 150 / 0.5); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBPercent() {
  sass::string input = ".test { color: rgb(50% 75% 100%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBPercentWithAlpha() {
  sass::string input = ".test { color: rgb(25% 50% 75% / 0.8); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBPercentWithAlphaPercent() {
  sass::string input = ".test { color: rgb(100% 0% 0% / 50%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBAllPercent() {
  sass::string input = ".test { color: rgb(80% 60% 40% / 75%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLSpaceSyntax() {
  sass::string input = ".test { color: hsl(240 60% 70%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLSpaceWithAlpha() {
  sass::string input = ".test { color: hsl(180 50% 60% / 0.7); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLWithDegrees() {
  sass::string input = ".test { color: hsl(120deg 100% 50%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLWithAlphaPercent() {
  sass::string input = ".test { color: hsl(0 100% 50% / 50%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLAllPercent() {
  sass::string input = ".test { color: hsl(270 80% 60% / 75%); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBMixedFormats() {
  sass::string input = ".a { c1: rgb(255, 128, 0); c2: rgb(255 128 0); c3: rgba(255, 128, 0, 0.5); c4: rgb(255 128 0 / 0.5); c5: rgb(255, 128, 0, 0.5); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "c1:");
  ASSERT_STR_CONTAINS(output, "c2:");
  ASSERT_STR_CONTAINS(output, "c3:");
  ASSERT_STR_CONTAINS(output, "c4:");
  ASSERT_STR_CONTAINS(output, "c5:");
  return true;
}

bool TestHSLMixedFormats() {
  sass::string input = ".a { c1: hsl(60, 100%, 50%); c2: hsl(60 100% 50%); c3: hsla(60, 100%, 50%, 0.5); c4: hsl(60 100% 50% / 0.5); c5: hsl(60, 100%, 50%, 0.5); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "c1:");
  ASSERT_STR_CONTAINS(output, "c2:");
  ASSERT_STR_CONTAINS(output, "c3:");
  ASSERT_STR_CONTAINS(output, "c4:");
  ASSERT_STR_CONTAINS(output, "c5:");
  return true;
}

bool TestRGBZeroAlpha() {
  sass::string input = ".test { color: rgb(255, 0, 0, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestRGBFullAlpha() {
  sass::string input = ".test { color: rgb(0, 255, 0, 1); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLZeroAlpha() {
  sass::string input = ".test { color: hsl(240, 100%, 50%, 0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
  return true;
}

bool TestHSLFullAlpha() {
  sass::string input = ".test { color: hsl(180, 50%, 50%, 1.0); }";
  sass::string output = compile_scss(input);
  ASSERT_FALSE(output.find("ERROR:") == 0);
  ASSERT_STR_CONTAINS(output, "color:");
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

  TEST(TestRGBWithAlpha);
  TEST(TestHSLWithAlpha);

  TEST(TestHWBFunction);
  TEST(TestHWBWithWhiteness);
  TEST(TestHWBWithBlackness);
  TEST(TestHWBWithAlpha);

  TEST(TestLabFunction);
  TEST(TestLabBlack);
  TEST(TestLabWhite);
  TEST(TestLabWithAlpha);

  TEST(TestLCHFunction);
  TEST(TestLCHBlack);
  TEST(TestLCHWhite);
  TEST(TestLCHWithAlpha);

  TEST(TestOKLabFunction);
  TEST(TestOKLabBlack);
  TEST(TestOKLabWhite);
  TEST(TestOKLabWithAlpha);

  TEST(TestOKLCHFunction);
  TEST(TestOKLCHBlack);
  TEST(TestOKLCHWhite);
  TEST(TestOKLCHWithAlpha);

  TEST(TestColorFunctionSRGB);
  TEST(TestColorFunctionSRGBWithAlpha);

  TEST(TestVariableWithHWB);
  TEST(TestVariableWithLab);
  TEST(TestVariableWithOKLCH);

  TEST(TestRGBSpaceSyntax);
  TEST(TestRGBSpaceWithAlpha);
  TEST(TestRGBPercent);
  TEST(TestRGBPercentWithAlpha);
  TEST(TestRGBPercentWithAlphaPercent);
  TEST(TestRGBAllPercent);

  TEST(TestHSLSpaceSyntax);
  TEST(TestHSLSpaceWithAlpha);
  TEST(TestHSLWithDegrees);
  TEST(TestHSLWithAlphaPercent);
  TEST(TestHSLAllPercent);

  TEST(TestRGBMixedFormats);
  TEST(TestHSLMixedFormats);

  TEST(TestRGBZeroAlpha);
  TEST(TestRGBFullAlpha);
  TEST(TestHSLZeroAlpha);
  TEST(TestHSLFullAlpha);

  std::cerr << argv[0] << ": Passed: " << passed.size()
            << ", failed: " << failed.size()
            << "." << std::endl;
  return failed.size();
}

