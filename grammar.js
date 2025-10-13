/**
 * @file Grammar for code tags like TODO:, FIXME(user): for the tree-sitter parsing library
 * @author Santos Gallegos <stsewd@proton.me>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const STOP_CHARS = [
  "/",
  "'",
  '"',
  "`",
  "<",
  "(",
  "[",
  "{",
  ".",
  ",",
  ":",
  ";",
  "!",
  "?",
  "\\",
  "}",
  "]",
  ")",
  ">",
  // This must be last, so that it isn't interpreted as a range.
  "-",
];

module.exports = grammar({
  name: "comment",

  externals: ($) => [
    $.name,
    $.invalid_token
  ],

  rules: {
    source: ($) => repeat(
      choice(
        $.tag,
        alias($._text, "text"),
      ),
    ),

    tag: ($) => seq(
      $.name,
      optional($._user),
      ":",
      optional($.text),
    ),

    _user: ($) => seq(
      "(",
      alias(/[^()]+/, $.user),
      ")",
    ),

    text: ($) => token(/[ \t]+[^A-Z\n\r][^\n\r]*/),

    // Text tokens can be a single character, or a sequence of characters that aren't stop characters.
    _text: ($) => choice($._stop_char, notmatching(STOP_CHARS)),
    _stop_char: ($) => choice(...STOP_CHARS),
  },
});

/**
 * Match any characters that aren't whitespace or that aren't in the given list.
 */
function notmatching(chars) {
  chars = escapeRegExp(chars.join(""));
  return new RegExp(`[^\\s${chars}]+`);
}

/**
 * Escape a string for use in a regular expression.
 *
 * Taken from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions#escaping.
 */
function escapeRegExp(string) {
  return string.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}
