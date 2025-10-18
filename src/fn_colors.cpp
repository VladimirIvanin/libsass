// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include <iomanip>
#include "ast.hpp"
#include "fn_utils.hpp"
#include "fn_colors.hpp"
#include "color_spaces.hpp"
#include "util.hpp"
#include "util_string.hpp"

namespace Sass {

  namespace Functions {

    // Check if argument is a "special number" according to CSS Color Level 4
    // Special numbers include: calc(), var(), env(), attr(), clamp(), min(), max()
    // These are CSS functions that may return a number and should be passed through as-is
    bool string_argument(AST_Node_Obj obj) {
      String_Constant* s = Cast<String_Constant>(obj);
      if (s == nullptr) return false;
      const sass::string& str = s->value();
      
      // Check for CSS function prefixes (case-insensitive check would be better, but this works)
      return starts_with(str, "calc(") ||
             starts_with(str, "var(") ||
             starts_with(str, "env(") ||
             starts_with(str, "attr(") ||
             starts_with(str, "clamp(") ||
             starts_with(str, "min(") ||
             starts_with(str, "max(") ||
             // Case variations
             starts_with(str, "CALC(") ||
             starts_with(str, "VAR(") ||
             starts_with(str, "ENV(") ||
             starts_with(str, "ATTR(") ||
             starts_with(str, "CLAMP(") ||
             starts_with(str, "MIN(") ||
             starts_with(str, "MAX(");
    }

    void hsla_alpha_percent_deprecation(const SourceSpan& pstate, const sass::string val)
    {

      sass::string msg("Passing a percentage as the alpha value to hsla() will be interpreted");
      sass::string tail("differently in future versions of Sass. For now, use " + val + " instead.");

      deprecated(msg, tail, false, pstate);

    }

    // CSS Colors Level 4: rgb() now supports optional 4th parameter for alpha
    Signature rgb_sig = "rgb($red, $green, $blue, $alpha: 1)";
    BUILT_IN(rgb)
    {
      // Check for string arguments (CSS custom properties, calc, var, etc.)
      bool has_string = string_argument(env["$red"]) ||
                       string_argument(env["$green"]) ||
                       string_argument(env["$blue"]);

      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);

      if (has_string || alpha_is_string) {
        // Output CSS syntax
        sass::string result = "rgb("
          + env["$red"]->to_string()
          + (has_string ? ", " : " ")
          + env["$green"]->to_string()
          + (has_string ? ", " : " ")
          + env["$blue"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += (has_string ? ", " : " / ") + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      // Numeric values
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;
      return SASS_MEMORY_NEW(Color_RGBA,
                             pstate,
                             COLOR_NUM("$red"),
                             COLOR_NUM("$green"),
                             COLOR_NUM("$blue"),
                             alpha);
    }

    Signature rgba_4_sig = "rgba($red, $green, $blue, $alpha)";
    BUILT_IN(rgba_4)
    {
      if (
        string_argument(env["$red"]) ||
        string_argument(env["$green"]) ||
        string_argument(env["$blue"]) ||
        string_argument(env["$alpha"])
      ) {
        return SASS_MEMORY_NEW(String_Constant, pstate, "rgba("
                                                        + env["$red"]->to_string()
                                                        + ", "
                                                        + env["$green"]->to_string()
                                                        + ", "
                                                        + env["$blue"]->to_string()
                                                        + ", "
                                                        + env["$alpha"]->to_string()
                                                        + ")"
        );
      }

      return SASS_MEMORY_NEW(Color_RGBA,
                             pstate,
                             COLOR_NUM("$red"),
                             COLOR_NUM("$green"),
                             COLOR_NUM("$blue"),
                             ALPHA_NUM("$alpha"));
    }

    Signature rgba_2_sig = "rgba($color, $alpha)";
    BUILT_IN(rgba_2)
    {
      if (
        string_argument(env["$color"])
      ) {
        return SASS_MEMORY_NEW(String_Constant, pstate, "rgba("
                                                        + env["$color"]->to_string()
                                                        + ", "
                                                        + env["$alpha"]->to_string()
                                                        + ")"
        );
      }

      Color_RGBA_Obj c_arg = ARG("$color", Color)->toRGBA();

      if (
        string_argument(env["$alpha"])
      ) {
        sass::ostream strm;
        strm << "rgba("
                 << (int)c_arg->r() << ", "
                 << (int)c_arg->g() << ", "
                 << (int)c_arg->b() << ", "
                 << env["$alpha"]->to_string()
             << ")";
        return SASS_MEMORY_NEW(String_Constant, pstate, strm.str());
      }

      Color_RGBA_Obj new_c = SASS_MEMORY_COPY(c_arg);
      new_c->a(ALPHA_NUM("$alpha"));
      new_c->disp("");
      return new_c.detach();
    }

    ////////////////
    // RGB FUNCTIONS
    ////////////////

    Signature red_sig = "red($color)";
    BUILT_IN(red)
    {
      Color_RGBA_Obj color = ARG("$color", Color)->toRGBA();
      return SASS_MEMORY_NEW(Number, pstate, color->r());
    }

    Signature green_sig = "green($color)";
    BUILT_IN(green)
    {
      Color_RGBA_Obj color = ARG("$color", Color)->toRGBA();
      return SASS_MEMORY_NEW(Number, pstate, color->g());
    }

    Signature blue_sig = "blue($color)";
    BUILT_IN(blue)
    {
      Color_RGBA_Obj color = ARG("$color", Color)->toRGBA();
      return SASS_MEMORY_NEW(Number, pstate, color->b());
    }

    Color_RGBA* colormix(Context& ctx, SourceSpan& pstate, Color* color1, Color* color2, double weight) {
      Color_RGBA_Obj c1 = color1->toRGBA();
      Color_RGBA_Obj c2 = color2->toRGBA();
      double p = weight/100;
      double w = 2*p - 1;
      double a = c1->a() - c2->a();

      double w1 = (((w * a == -1) ? w : (w + a)/(1 + w*a)) + 1)/2.0;
      double w2 = 1 - w1;

      return SASS_MEMORY_NEW(Color_RGBA,
                             pstate,
                             Sass::round(w1*c1->r() + w2*c2->r(), ctx.c_options.precision),
                             Sass::round(w1*c1->g() + w2*c2->g(), ctx.c_options.precision),
                             Sass::round(w1*c1->b() + w2*c2->b(), ctx.c_options.precision),
                             c1->a()*p + c2->a()*(1-p));
    }

    Signature mix_sig = "mix($color1, $color2, $weight: 50%)";
    BUILT_IN(mix)
    {
      Color_Obj  color1 = ARG("$color1", Color);
      Color_Obj  color2 = ARG("$color2", Color);
      double weight = DARG_U_PRCT("$weight");
      return colormix(ctx, pstate, color1, color2, weight);

    }

    ////////////////
    // HSL FUNCTIONS
    ////////////////

    // CSS Colors Level 4: hsl() now supports optional 4th parameter for alpha
    Signature hsl_sig = "hsl($hue, $saturation, $lightness, $alpha: 1)";
    BUILT_IN(hsl)
    {
      // Check for string arguments (CSS custom properties, calc, var, etc.)
      bool has_string = string_argument(env["$hue"]) ||
                       string_argument(env["$saturation"]) ||
                       string_argument(env["$lightness"]);

      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);

      if (has_string || alpha_is_string) {
        // Output CSS syntax
        sass::string result = "hsl("
          + env["$hue"]->to_string()
          + (has_string ? ", " : " ")
          + env["$saturation"]->to_string()
          + (has_string ? ", " : " ")
          + env["$lightness"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += (has_string ? ", " : " / ") + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      // Numeric values
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;
      return SASS_MEMORY_NEW(Color_HSLA,
        pstate,
        ARGVAL("$hue"),
        ARGVAL("$saturation"),
        ARGVAL("$lightness"),
        alpha);
    }

    Signature hsla_sig = "hsla($hue, $saturation, $lightness, $alpha)";
    BUILT_IN(hsla)
    {
      if (
        string_argument(env["$hue"]) ||
        string_argument(env["$saturation"]) ||
        string_argument(env["$lightness"]) ||
        string_argument(env["$alpha"])
      ) {
        return SASS_MEMORY_NEW(String_Constant, pstate, "hsla("
                                                        + env["$hue"]->to_string()
                                                        + ", "
                                                        + env["$saturation"]->to_string()
                                                        + ", "
                                                        + env["$lightness"]->to_string()
                                                        + ", "
                                                        + env["$alpha"]->to_string()
                                                        + ")"
        );
      }

      Number* alpha = ARG("$alpha", Number);
      if (alpha && alpha->unit() == "%") {
        Number_Obj val = SASS_MEMORY_COPY(alpha);
        val->numerators.clear(); // convert
        val->value(val->value() / 100.0);
        sass::string nr(val->to_string(ctx.c_options));
        hsla_alpha_percent_deprecation(pstate, nr);
      }

      return SASS_MEMORY_NEW(Color_HSLA,
        pstate,
        ARGVAL("$hue"),
        ARGVAL("$saturation"),
        ARGVAL("$lightness"),
        ARGVAL("$alpha"));

    }

    /////////////////////////////////////////////////////////////////////////
    // Query functions
    /////////////////////////////////////////////////////////////////////////

    Signature hue_sig = "hue($color)";
    BUILT_IN(hue)
    {
      Color_HSLA_Obj col = ARG("$color", Color)->toHSLA();
      return SASS_MEMORY_NEW(Number, pstate, col->h(), "deg");
    }

    Signature saturation_sig = "saturation($color)";
    BUILT_IN(saturation)
    {
      Color_HSLA_Obj col = ARG("$color", Color)->toHSLA();
      return SASS_MEMORY_NEW(Number, pstate, col->s(), "%");
    }

    Signature lightness_sig = "lightness($color)";
    BUILT_IN(lightness)
    {
      Color_HSLA_Obj col = ARG("$color", Color)->toHSLA();
      return SASS_MEMORY_NEW(Number, pstate, col->l(), "%");
    }

    /////////////////////////////////////////////////////////////////////////
    // HSL manipulation functions
    /////////////////////////////////////////////////////////////////////////

    Signature adjust_hue_sig = "adjust-hue($color, $degrees)";
    BUILT_IN(adjust_hue)
    {
      Color* col = ARG("$color", Color);
      double degrees = ARGVAL("$degrees");
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->h(absmod(copy->h() + degrees, 360.0));
      return copy.detach();
    }

    Signature lighten_sig = "lighten($color, $amount)";
    BUILT_IN(lighten)
    {
      Color* col = ARG("$color", Color);
      double amount = DARG_U_PRCT("$amount");
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->l(clip(copy->l() + amount, 0.0, 100.0));
      return copy.detach();

    }

    Signature darken_sig = "darken($color, $amount)";
    BUILT_IN(darken)
    {
      Color* col = ARG("$color", Color);
      double amount = DARG_U_PRCT("$amount");
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->l(clip(copy->l() - amount, 0.0, 100.0));
      return copy.detach();
    }

    Signature saturate_sig = "saturate($color, $amount: false)";
    BUILT_IN(saturate)
    {
      // CSS3 filter function overload: pass literal through directly
      if (!Cast<Number>(env["$amount"])) {
        return SASS_MEMORY_NEW(String_Quoted, pstate, "saturate(" + env["$color"]->to_string(ctx.c_options) + ")");
      }

      Color* col = ARG("$color", Color);
      double amount = DARG_U_PRCT("$amount");
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->s(clip(copy->s() + amount, 0.0, 100.0));
      return copy.detach();
    }

    Signature desaturate_sig = "desaturate($color, $amount)";
    BUILT_IN(desaturate)
    {
      Color* col = ARG("$color", Color);
      double amount = DARG_U_PRCT("$amount");
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->s(clip(copy->s() - amount, 0.0, 100.0));
      return copy.detach();
    }

    Signature grayscale_sig = "grayscale($color)";
    BUILT_IN(grayscale)
    {
      // CSS3 filter function overload: pass literal through directly
      Number* amount = Cast<Number>(env["$color"]);
      if (amount) {
        return SASS_MEMORY_NEW(String_Quoted, pstate, "grayscale(" + amount->to_string(ctx.c_options) + ")");
      }

      Color* col = ARG("$color", Color);
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->s(0.0); // just reset saturation
      return copy.detach();
    }

    /////////////////////////////////////////////////////////////////////////
    // Misc manipulation functions
    /////////////////////////////////////////////////////////////////////////

    Signature complement_sig = "complement($color)";
    BUILT_IN(complement)
    {
      Color* col = ARG("$color", Color);
      Color_HSLA_Obj copy = col->copyAsHSLA();
      copy->h(absmod(copy->h() - 180.0, 360.0));
      return copy.detach();
    }

    Signature invert_sig = "invert($color, $weight: 100%)";
    BUILT_IN(invert)
    {
      // CSS3 filter function overload: pass literal through directly
      Number* amount = Cast<Number>(env["$color"]);
      double weight = DARG_U_PRCT("$weight");
      if (amount) {
        // TODO: does not throw on 100% manually passed as value
        if (weight < 100.0) {
          error("Only one argument may be passed to the plain-CSS invert() function.", pstate, traces);
        }
        return SASS_MEMORY_NEW(String_Quoted, pstate, "invert(" + amount->to_string(ctx.c_options) + ")");
      }

      Color* col = ARG("$color", Color);
      Color_RGBA_Obj inv = col->copyAsRGBA();
      inv->r(clip(255.0 - inv->r(), 0.0, 255.0));
      inv->g(clip(255.0 - inv->g(), 0.0, 255.0));
      inv->b(clip(255.0 - inv->b(), 0.0, 255.0));
      return colormix(ctx, pstate, inv, col, weight);
    }

    /////////////////////////////////////////////////////////////////////////
    // Opacity functions
    /////////////////////////////////////////////////////////////////////////

    Signature alpha_sig = "alpha($color)";
    Signature opacity_sig = "opacity($color)";
    BUILT_IN(alpha)
    {
      String_Constant* ie_kwd = Cast<String_Constant>(env["$color"]);
      if (ie_kwd) {
        return SASS_MEMORY_NEW(String_Quoted, pstate, "alpha(" + ie_kwd->value() + ")");
      }

      // CSS3 filter function overload: pass literal through directly
      Number* amount = Cast<Number>(env["$color"]);
      if (amount) {
        return SASS_MEMORY_NEW(String_Quoted, pstate, "opacity(" + amount->to_string(ctx.c_options) + ")");
      }

      return SASS_MEMORY_NEW(Number, pstate, ARG("$color", Color)->a());
    }

    Signature opacify_sig = "opacify($color, $amount)";
    Signature fade_in_sig = "fade-in($color, $amount)";
    BUILT_IN(opacify)
    {
      Color* col = ARG("$color", Color);
      double amount = DARG_U_FACT("$amount");
      Color_Obj copy = SASS_MEMORY_COPY(col);
      copy->a(clip(col->a() + amount, 0.0, 1.0));
      return copy.detach();
    }

    Signature transparentize_sig = "transparentize($color, $amount)";
    Signature fade_out_sig = "fade-out($color, $amount)";
    BUILT_IN(transparentize)
    {
      Color* col = ARG("$color", Color);
      double amount = DARG_U_FACT("$amount");
      Color_Obj copy = SASS_MEMORY_COPY(col);
      copy->a(std::max(col->a() - amount, 0.0));
      return copy.detach();
    }

    ////////////////////////
    // OTHER COLOR FUNCTIONS
    ////////////////////////

    Signature adjust_color_sig = "adjust-color($color, $red: false, $green: false, $blue: false, $hue: false, $saturation: false, $lightness: false, $alpha: false)";
    BUILT_IN(adjust_color)
    {
      Color* col = ARG("$color", Color);
      Number* r = Cast<Number>(env["$red"]);
      Number* g = Cast<Number>(env["$green"]);
      Number* b = Cast<Number>(env["$blue"]);
      Number* h = Cast<Number>(env["$hue"]);
      Number* s = Cast<Number>(env["$saturation"]);
      Number* l = Cast<Number>(env["$lightness"]);
      Number* a = Cast<Number>(env["$alpha"]);

      bool rgb = r || g || b;
      bool hsl = h || s || l;

      if (rgb && hsl) {
        error("Cannot specify HSL and RGB values for a color at the same time for `adjust-color'", pstate, traces);
      }
      else if (rgb) {
        Color_RGBA_Obj c = col->copyAsRGBA();
        if (r) c->r(c->r() + DARG_R_BYTE("$red"));
        if (g) c->g(c->g() + DARG_R_BYTE("$green"));
        if (b) c->b(c->b() + DARG_R_BYTE("$blue"));
        if (a) c->a(c->a() + DARG_R_FACT("$alpha"));
        return c.detach();
      }
      else if (hsl) {
        Color_HSLA_Obj c = col->copyAsHSLA();
        if (h) c->h(c->h() + absmod(h->value(), 360.0));
        if (s) c->s(c->s() + DARG_R_PRCT("$saturation"));
        if (l) c->l(c->l() + DARG_R_PRCT("$lightness"));
        if (a) c->a(c->a() + DARG_R_FACT("$alpha"));
        return c.detach();
      }
      else if (a) {
        Color_Obj c = SASS_MEMORY_COPY(col);
        c->a(c->a() + DARG_R_FACT("$alpha"));
        c->a(clip(c->a(), 0.0, 1.0));
        return c.detach();
      }
      error("not enough arguments for `adjust-color'", pstate, traces);
      // unreachable
      return col;
    }

    Signature scale_color_sig = "scale-color($color, $red: false, $green: false, $blue: false, $hue: false, $saturation: false, $lightness: false, $alpha: false)";
    BUILT_IN(scale_color)
    {
      Color* col = ARG("$color", Color);
      Number* r = Cast<Number>(env["$red"]);
      Number* g = Cast<Number>(env["$green"]);
      Number* b = Cast<Number>(env["$blue"]);
      Number* h = Cast<Number>(env["$hue"]);
      Number* s = Cast<Number>(env["$saturation"]);
      Number* l = Cast<Number>(env["$lightness"]);
      Number* a = Cast<Number>(env["$alpha"]);

      bool rgb = r || g || b;
      bool hsl = h || s || l;

      if (rgb && hsl) {
        error("Cannot specify HSL and RGB values for a color at the same time for `scale-color'", pstate, traces);
      }
      else if (rgb) {
        Color_RGBA_Obj c = col->copyAsRGBA();
        double rscale = (r ? DARG_R_PRCT("$red") : 0.0) / 100.0;
        double gscale = (g ? DARG_R_PRCT("$green") : 0.0) / 100.0;
        double bscale = (b ? DARG_R_PRCT("$blue") : 0.0) / 100.0;
        double ascale = (a ? DARG_R_PRCT("$alpha") : 0.0) / 100.0;
        if (rscale) c->r(c->r() + rscale * (rscale > 0.0 ? 255.0 - c->r() : c->r()));
        if (gscale) c->g(c->g() + gscale * (gscale > 0.0 ? 255.0 - c->g() : c->g()));
        if (bscale) c->b(c->b() + bscale * (bscale > 0.0 ? 255.0 - c->b() : c->b()));
        if (ascale) c->a(c->a() + ascale * (ascale > 0.0 ? 1.0 - c->a() : c->a()));
        return c.detach();
      }
      else if (hsl) {
        Color_HSLA_Obj c = col->copyAsHSLA();
        double hscale = (h ? DARG_R_PRCT("$hue") : 0.0) / 100.0;
        double sscale = (s ? DARG_R_PRCT("$saturation") : 0.0) / 100.0;
        double lscale = (l ? DARG_R_PRCT("$lightness") : 0.0) / 100.0;
        double ascale = (a ? DARG_R_PRCT("$alpha") : 0.0) / 100.0;
        if (hscale) c->h(c->h() + hscale * (hscale > 0.0 ? 360.0 - c->h() : c->h()));
        if (sscale) c->s(c->s() + sscale * (sscale > 0.0 ? 100.0 - c->s() : c->s()));
        if (lscale) c->l(c->l() + lscale * (lscale > 0.0 ? 100.0 - c->l() : c->l()));
        if (ascale) c->a(c->a() + ascale * (ascale > 0.0 ? 1.0 - c->a() : c->a()));
        return c.detach();
      }
      else if (a) {
        Color_Obj c = SASS_MEMORY_COPY(col);
        double ascale = DARG_R_PRCT("$alpha") / 100.0;
        c->a(c->a() + ascale * (ascale > 0.0 ? 1.0 - c->a() : c->a()));
        c->a(clip(c->a(), 0.0, 1.0));
        return c.detach();
      }
      error("not enough arguments for `scale-color'", pstate, traces);
      // unreachable
      return col;
    }

    Signature change_color_sig = "change-color($color, $red: false, $green: false, $blue: false, $hue: false, $saturation: false, $lightness: false, $alpha: false)";
    BUILT_IN(change_color)
    {
      Color* col = ARG("$color", Color);
      Number* r = Cast<Number>(env["$red"]);
      Number* g = Cast<Number>(env["$green"]);
      Number* b = Cast<Number>(env["$blue"]);
      Number* h = Cast<Number>(env["$hue"]);
      Number* s = Cast<Number>(env["$saturation"]);
      Number* l = Cast<Number>(env["$lightness"]);
      Number* a = Cast<Number>(env["$alpha"]);

      bool rgb = r || g || b;
      bool hsl = h || s || l;

      if (rgb && hsl) {
        error("Cannot specify HSL and RGB values for a color at the same time for `change-color'", pstate, traces);
      }
      else if (rgb) {
        Color_RGBA_Obj c = col->copyAsRGBA();
        if (r) c->r(DARG_U_BYTE("$red"));
        if (g) c->g(DARG_U_BYTE("$green"));
        if (b) c->b(DARG_U_BYTE("$blue"));
        if (a) c->a(DARG_U_FACT("$alpha"));
        return c.detach();
      }
      else if (hsl) {
        Color_HSLA_Obj c = col->copyAsHSLA();
        if (h) c->h(absmod(h->value(), 360.0));
        if (s) c->s(DARG_U_PRCT("$saturation"));
        if (l) c->l(DARG_U_PRCT("$lightness"));
        if (a) c->a(DARG_U_FACT("$alpha"));
        return c.detach();
      }
      else if (a) {
        Color_Obj c = SASS_MEMORY_COPY(col);
        c->a(clip(DARG_U_FACT("$alpha"), 0.0, 1.0));
        return c.detach();
      }
      error("not enough arguments for `change-color'", pstate, traces);
      // unreachable
      return col;
    }

    Signature ie_hex_str_sig = "ie-hex-str($color)";
    BUILT_IN(ie_hex_str)
    {
      Color* col = ARG("$color", Color);
      Color_RGBA_Obj c = col->toRGBA();
      double r = clip(c->r(), 0.0, 255.0);
      double g = clip(c->g(), 0.0, 255.0);
      double b = clip(c->b(), 0.0, 255.0);
      double a = clip(c->a(), 0.0, 1.0) * 255.0;

      sass::ostream ss;
      ss << '#' << std::setw(2) << std::setfill('0');
      ss << std::hex << std::setw(2) << static_cast<unsigned long>(Sass::round(a, ctx.c_options.precision));
      ss << std::hex << std::setw(2) << static_cast<unsigned long>(Sass::round(r, ctx.c_options.precision));
      ss << std::hex << std::setw(2) << static_cast<unsigned long>(Sass::round(g, ctx.c_options.precision));
      ss << std::hex << std::setw(2) << static_cast<unsigned long>(Sass::round(b, ctx.c_options.precision));

      sass::string result = ss.str();
      Util::ascii_str_toupper(&result);
      return SASS_MEMORY_NEW(String_Quoted, pstate, result);
    }

    /////////////////////////////////////////////////////////////////////////
    // CSS Colors Level 4 functions
    /////////////////////////////////////////////////////////////////////////

    Signature hwb_sig = "hwb($hue, $whiteness, $blackness, $alpha: 1)";
    BUILT_IN(hwb)
    {
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      // Check for string arguments (CSS custom properties, calc, var, etc.)
      if (
        string_argument(env["$hue"]) ||
        string_argument(env["$whiteness"]) ||
        string_argument(env["$blackness"]) ||
        alpha_is_string
      ) {
        sass::string result = "hwb("
          + env["$hue"]->to_string()
          + " "
          + env["$whiteness"]->to_string()
          + " "
          + env["$blackness"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += " / " + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      double hue = ARGVAL("$hue");
      double whiteness = DARG_U_PRCT("$whiteness");
      double blackness = DARG_U_PRCT("$blackness");
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

      HWB hwb_color(hue, whiteness, blackness, alpha);
      return hwb_color.toRGBA();
    }

    Signature lab_sig = "lab($lightness, $a, $b, $alpha: 1)";
    BUILT_IN(lab)
    {
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      if (
        string_argument(env["$lightness"]) ||
        string_argument(env["$a"]) ||
        string_argument(env["$b"]) ||
        alpha_is_string
      ) {
        sass::string result = "lab("
          + env["$lightness"]->to_string()
          + " "
          + env["$a"]->to_string()
          + " "
          + env["$b"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += " / " + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      double lightness = DARG_U_PRCT("$lightness");
      double a = ARGVAL("$a");
      double b = ARGVAL("$b");
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

      Lab lab_color(lightness, a, b, alpha);
      return lab_color.toRGBA();
    }

    Signature lch_sig = "lch($lightness, $chroma, $hue, $alpha: 1)";
    BUILT_IN(lch)
    {
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      if (
        string_argument(env["$lightness"]) ||
        string_argument(env["$chroma"]) ||
        string_argument(env["$hue"]) ||
        alpha_is_string
      ) {
        sass::string result = "lch("
          + env["$lightness"]->to_string()
          + " "
          + env["$chroma"]->to_string()
          + " "
          + env["$hue"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += " / " + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      double lightness = DARG_U_PRCT("$lightness");
      double chroma = ARGVAL("$chroma");
      double hue = ARGVAL("$hue");
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

      LCH lch_color(lightness, chroma, hue, alpha);
      return lch_color.toRGBA();
    }

    Signature oklab_sig = "oklab($lightness, $a, $b, $alpha: 1)";
    BUILT_IN(oklab)
    {
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      if (
        string_argument(env["$lightness"]) ||
        string_argument(env["$a"]) ||
        string_argument(env["$b"]) ||
        alpha_is_string
      ) {
        sass::string result = "oklab("
          + env["$lightness"]->to_string()
          + " "
          + env["$a"]->to_string()
          + " "
          + env["$b"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += " / " + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      // OKLab uses 0-1 range for lightness, not percentage
      double lightness = DARG_U_FACT("$lightness");
      double a = ARGVAL("$a");
      double b = ARGVAL("$b");
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

      OKLab oklab_color(lightness, a, b, alpha);
      return oklab_color.toRGBA();
    }

    Signature oklch_sig = "oklch($lightness, $chroma, $hue, $alpha: 1)";
    BUILT_IN(oklch)
    {
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      if (
        string_argument(env["$lightness"]) ||
        string_argument(env["$chroma"]) ||
        string_argument(env["$hue"]) ||
        alpha_is_string
      ) {
        sass::string result = "oklch("
          + env["$lightness"]->to_string()
          + " "
          + env["$chroma"]->to_string()
          + " "
          + env["$hue"]->to_string();

        // Add alpha if present and not default 1
        if (has_alpha_arg) {
          Number* alpha_num = Cast<Number>(env["$alpha"]);
          if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
            result += " / " + env["$alpha"]->to_string();
          }
        }
        result += ")";

        return SASS_MEMORY_NEW(String_Constant, pstate, result);
      }

      // OKLCH uses 0-1 range for lightness, not percentage
      double lightness = DARG_U_FACT("$lightness");
      double chroma = ARGVAL("$chroma");
      double hue = ARGVAL("$hue");
      double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

      OKLCH oklch_color(lightness, chroma, hue, alpha);
      return oklch_color.toRGBA();
    }

    Signature color_sig = "color($space, $channel1, $channel2, $channel3, $alpha: 1)";
    BUILT_IN(color)
    {
      // Get the color space name
      String_Constant* space_str = Cast<String_Constant>(env["$space"]);
      if (!space_str) {
        error("$space: " + env["$space"]->to_string() + " is not a string.", pstate, traces);
      }

      sass::string space = space_str->value();
      
      bool has_alpha_arg = env.has("$alpha");
      bool alpha_is_string = has_alpha_arg && string_argument(env["$alpha"]);
      
      // Check if any channel contains special numbers (calc, var, etc.)
      bool has_special_number = string_argument(env["$channel1"]) ||
                                 string_argument(env["$channel2"]) ||
                                 string_argument(env["$channel3"]) ||
                                 alpha_is_string;

      // For now, support the most common color spaces
      // Full implementation would support: srgb, srgb-linear, display-p3, a98-rgb, prophoto-rgb, rec2020, xyz, xyz-d50, xyz-d65

      if ((space == "srgb" || space == "rgb") && !has_special_number) {
        // sRGB is the same as legacy RGB - only compute if no special numbers
        double r = ARGVAL("$channel1");
        double g = ARGVAL("$channel2");
        double b = ARGVAL("$channel3");
        double alpha = has_alpha_arg ? ALPHA_NUM("$alpha") : 1.0;

        return SASS_MEMORY_NEW(Color_RGBA, pstate, r * 255.0, g * 255.0, b * 255.0, alpha);
      }

      // For other color spaces or when special numbers are present, output as CSS string for browser support
      sass::string result = "color(" + space + " "
        + env["$channel1"]->to_string() + " "
        + env["$channel2"]->to_string() + " "
        + env["$channel3"]->to_string();

      // Add alpha if present and not default 1
      if (has_alpha_arg) {
        Number* alpha_num = Cast<Number>(env["$alpha"]);
        if (alpha_is_string || !alpha_num || alpha_num->value() != 1.0) {
          result += " / " + env["$alpha"]->to_string();
        }
      }
      result += ")";

      return SASS_MEMORY_NEW(String_Constant, pstate, result);
    }

  }

}
