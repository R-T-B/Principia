﻿using KSP.Localization;
using System.Text.RegularExpressions;

namespace principia {
namespace ksp_plugin_adapter {

internal static class L10N {
  private const string english_us_ = "en-us";

  public static bool IsCJKV(string text) {
    return Regex.IsMatch(
        text,
        @"^[\p{IsCJKUnifiedIdeographs}\p{IsCJKUnifiedIdeographsExtensionA}]*$");
  }

  public static string ZWSPToHyphenBetweenNonCJK(string text) {
    return Regex.Replace(
        text,
        @"([^\p{IsCJKUnifiedIdeographs}\p{IsCJKUnifiedIdeographsExtensionA}])" +
        "\u200B" +
        @"([^\p{IsCJKUnifiedIdeographs}\p{IsCJKUnifiedIdeographsExtensionA}])",
        "$1-$2");
  }

  public static string Standalone(string name) {
    return Localizer.Format("#Principia_GrammaticalForm_Standalone", name);
  }

  public static string NameWithoutArticle(this CelestialBody body) {
    // This will need to be adjusted when we add support for other languages
    // with articles.
    return (Localizer.CurrentLanguage == english_us_ &&
            body.displayName.StartsWith("The ")) ? body.name : body.displayName;
  }

  public static string NameWithArticle(this CelestialBody body) {
    // TODO(egg): should we apped ^d instead of prepending "the"?  Certainly
    // we would need to do that for other languages.
    return (Localizer.CurrentLanguage == english_us_ &&
            body.displayName.StartsWith("The ")) ? "the " + body.name
                                                 : body.displayName;
  }

  public static string FormatOrNull(string template, params object[] args) {
    // Optional translations include the language name so that they do not fall
    // back to English.
    string qualified_template = $"{template}.{Localizer.CurrentLanguage}";
    if (!Localizer.Tags.ContainsKey(qualified_template)) {
      return null;
    }
    return Localizer.Format(qualified_template, args);
  }
}

}
}
