#include "sass.h"
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

#define ASSERT_CONTAINS(str, substr) \
  if ((str).find(substr) == std::string::npos) { \
    std::cerr << "Expected '" << (str) << "' to contain '" << (substr) << "'" \
              << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

#define ASSERT_NO_ERROR(str) \
  if ((str).find("ERROR:") == 0) { \
    std::cerr << "Compilation error: " << (str) << " at " \
              << __FILE__ << ":" << __LINE__ << std::endl; \
    return false; \
  }

// Helper function to parse and evaluate SCSS code
std::string compile_scss(const std::string& scss_code) {
  std::string input = ".test { color: " + scss_code + "; }";
  Sass_Data_Context* ctx = sass_make_data_context(strdup(input.c_str()));
  sass_compile_data_context(ctx);

  int status = sass_context_get_error_status(sass_data_context_get_context(ctx));
  if (status) {
    const char* error = sass_context_get_error_message(sass_data_context_get_context(ctx));
    sass_delete_data_context(ctx);
    return std::string("ERROR: ") + error;
  }

  const char* output = sass_context_get_output_string(sass_data_context_get_context(ctx));
  std::string result(output);
  sass_delete_data_context(ctx);
  return result;
}

// Test: Parse rgb() with old comma-separated syntax
bool TestRgbOldSyntaxCommas() {
  std::string result = compile_scss("rgb(255, 0, 0)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "red");
  return true;
}

// Test: Parse rgb() with new space-separated syntax
bool TestRgbNewSyntaxSpaces() {
  std::string result = compile_scss("rgb(255 0 0)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "red");
  return true;
}

// Test: Parse rgb() with new syntax and alpha (slash separator)
bool TestRgbNewSyntaxWithAlpha() {
  std::string result = compile_scss("rgb(255 0 0 / 0.5)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Parse rgb() with percentages and spaces
bool TestRgbPercentagesSpaces() {
  std::string result = compile_scss("rgb(100% 0% 0%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "red");
  return true;
}

// Test: Parse rgb() with percentages and alpha
bool TestRgbPercentagesWithAlpha() {
  std::string result = compile_scss("rgb(100% 0% 0% / 0.5)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Parse rgba() with old comma syntax
bool TestRgbaOldSyntax() {
  std::string result = compile_scss("rgba(255, 0, 0, 0.5)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Parse hsl() with old comma-separated syntax
bool TestHslOldSyntaxCommas() {
  std::string result = compile_scss("hsl(0, 100%, 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "red");
  return true;
}

// Test: Parse hsl() with new space-separated syntax
bool TestHslNewSyntaxSpaces() {
  std::string result = compile_scss("hsl(0 100% 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "red");
  return true;
}

// Test: Parse hsl() with new syntax and alpha (slash separator)
bool TestHslNewSyntaxWithAlpha() {
  std::string result = compile_scss("hsl(0 100% 50% / 0.5)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Parse hsl() with degrees notation
bool TestHslWithDegrees() {
  std::string result = compile_scss("hsl(120deg 100% 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "lime");
  return true;
}

// Test: Parse hsl() with unitless hue
bool TestHslUnitlessHue() {
  std::string result = compile_scss("hsl(240 100% 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "blue");
  return true;
}

// Test: Parse hsla() with old comma syntax
bool TestHslaOldSyntax() {
  std::string result = compile_scss("hsla(0, 100%, 50%, 0.5)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Mixed format - rgb with numbers
bool TestRgbMixedFormat() {
  std::string result = compile_scss("rgb(100 150 200)");
  ASSERT_NO_ERROR(result);
  ASSERT_TRUE(result.length() > 0);
  return true;
}

// Test: Mixed format - hsl with different units
bool TestHslMixedFormat() {
  std::string result = compile_scss("hsl(180 50% 60%)");
  ASSERT_NO_ERROR(result);
  ASSERT_TRUE(result.length() > 0);
  return true;
}

// Test: Alpha as percentage in new syntax
bool TestRgbAlphaPercentage() {
  std::string result = compile_scss("rgb(100% 0% 0% / 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Alpha as percentage in hsl new syntax
bool TestHslAlphaPercentage() {
  std::string result = compile_scss("hsl(0 100% 50% / 50%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0.5)");
  return true;
}

// Test: Zero alpha value
bool TestRgbZeroAlpha() {
  std::string result = compile_scss("rgb(255 0 0 / 0)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba(255, 0, 0, 0)");
  return true;
}

// Test: Full alpha value (should optimize to opaque color)
bool TestRgbFullAlpha() {
  std::string result = compile_scss("rgb(0 255 0 / 1)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "lime");
  return true;
}

// Test: Edge case - all zeros
bool TestRgbAllZeros() {
  std::string result = compile_scss("rgb(0 0 0)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "black");
  return true;
}

// Test: Edge case - all max values
bool TestRgbAllMax() {
  std::string result = compile_scss("rgb(255 255 255)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "white");
  return true;
}

// Test: Complex expression in new syntax
bool TestRgbComplexExpression() {
  std::string result = compile_scss("rgb(200 100 50)");
  ASSERT_NO_ERROR(result);
  ASSERT_TRUE(result.length() > 0);
  return true;
}

// Test: HSL with decimal saturation and lightness
bool TestHslDecimalValues() {
  std::string result = compile_scss("hsl(180 50.5% 60.2%)");
  ASSERT_NO_ERROR(result);
  ASSERT_TRUE(result.length() > 0);
  return true;
}

// Test: RGB with percentage alpha
bool TestRgbaPercentAlpha() {
  std::string result = compile_scss("rgb(80% 60% 40% / 75%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba");
  return true;
}

// Test: HSL with percentage alpha
bool TestHslaPercentAlpha() {
  std::string result = compile_scss("hsl(270 80% 60% / 75%)");
  ASSERT_NO_ERROR(result);
  ASSERT_CONTAINS(result, "rgba");
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

  // RGB tests with old syntax
  TEST(TestRgbOldSyntaxCommas);
  TEST(TestRgbaOldSyntax);

  // RGB tests with new syntax
  TEST(TestRgbNewSyntaxSpaces);
  TEST(TestRgbNewSyntaxWithAlpha);
  TEST(TestRgbPercentagesSpaces);
  TEST(TestRgbPercentagesWithAlpha);
  TEST(TestRgbAlphaPercentage);
  TEST(TestRgbZeroAlpha);
  TEST(TestRgbFullAlpha);

  // HSL tests with old syntax
  TEST(TestHslOldSyntaxCommas);
  TEST(TestHslaOldSyntax);

  // HSL tests with new syntax
  TEST(TestHslNewSyntaxSpaces);
  TEST(TestHslNewSyntaxWithAlpha);
  TEST(TestHslWithDegrees);
  TEST(TestHslUnitlessHue);
  TEST(TestHslAlphaPercentage);

  // Mixed format tests
  TEST(TestRgbMixedFormat);
  TEST(TestHslMixedFormat);
  TEST(TestRgbComplexExpression);
  TEST(TestHslDecimalValues);
  TEST(TestRgbaPercentAlpha);
  TEST(TestHslaPercentAlpha);

  // Edge cases
  TEST(TestRgbAllZeros);
  TEST(TestRgbAllMax);

  std::cerr << argv[0] << ": Passed: " << passed.size()
            << ", failed: " << failed.size()
            << "." << std::endl;
  return failed.size();
}

