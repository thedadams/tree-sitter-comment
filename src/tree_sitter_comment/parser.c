#include "chars.c"

#include "parser.h"
#include "tokens.h"
#include "tree_sitter_comment/chars.h"
#include <stdbool.h>
#include <stdio.h>

/// Parse the name of the tag.
///
/// They can be of the form:
/// - TODO:
/// - TODO: text
/// - TODO(thedadams):
/// - TODO(thedadams): text
/// - TODO (thedadams): text
static bool parse_tagname(TSLexer* lexer, const bool* valid_symbols)
{
  while (is_possible_start_of_tag(lexer->lookahead)) {
    lexer->advance(lexer, false);
  }

  if (!is_upper(lexer->lookahead)) {
    return false;
  }

  int32_t previous = lexer->lookahead;
  lexer->advance(lexer, false);

  while (is_upper(lexer->lookahead)
      || is_digit(lexer->lookahead)
      || is_internal_char(lexer->lookahead)) {
    previous = lexer->lookahead;
    lexer->advance(lexer, false);
  }
  // The tag name ends here.
  // But we keep parsing to see if it's a valid tag name.
  lexer->mark_end(lexer);

  // It can't end with an internal char.
  if (is_internal_char(previous)) {
    return false;
  }

  // For the user component this is `\s*(`.
  // We don't parse that part, we just need to be sure it ends with `:\s`.
  if ((is_space(lexer->lookahead) && !is_newline(lexer->lookahead))
      || lexer->lookahead == '(') {
    // Skip white spaces.
    while (is_space(lexer->lookahead) && !is_newline(lexer->lookahead)) {
      lexer->advance(lexer, false);
    }
    // Checking aperture.
    if (lexer->lookahead != '(') {
      return false;
    }
    lexer->advance(lexer, false);

    // Checking closure.
    int user_length = 0;
    while (lexer->lookahead != ')') {
      if (is_newline(lexer->lookahead)) {
        return false;
      }
      lexer->advance(lexer, false);
      user_length++;
    }
    if (user_length == 0) {
      return false;
    }
    lexer->advance(lexer, false);
  }

  // It should end with `:`...
  if (lexer->lookahead != ':') {
    return false;
  }

  // ... and be followed by one space.
  lexer->advance(lexer, false);
  if (!is_space(lexer->lookahead)) {
    return false;
  }

  lexer->result_symbol = T_TAGNAME;
  return true;
}

static bool parse_tagtext(TSLexer* lexer, const bool* valid_symbols) {
  bool has_text = false;
  while (!is_eof(lexer->lookahead)) {
    while(is_possible_start_of_tag(lexer->lookahead)) {
      lexer->advance(lexer, false);
    }

    if (is_newline(lexer->lookahead)) {
      if (!has_text) {
        return false;
      }

      lexer->result_symbol = T_TAGTEXT;
      return true;
    }

    has_text = true;

    while (!is_newline(lexer->lookahead)) {
      lexer->advance(lexer, false);
    }

    if (!is_eof(lexer->lookahead)) {
      lexer->advance(lexer, false);
    }

    lexer->mark_end(lexer);
  }

  if (!has_text) {
    return false;
  }

  lexer->result_symbol = T_TAGTEXT;
  return true;
}

static bool parse_code_block(TSLexer *lexer, const bool *valid_symbols)
{
  // Count the number of backticks
  uint8_t level = 0;
  while (lexer->lookahead == CHAR_BACK_TICK) {
    lexer->advance(lexer, false);
    level++;
  }
  lexer->mark_end(lexer);

  if (level < 3) {
    return false;
  }

  if (valid_symbols[T_CODE_BLOCK_END]) {
    while (is_space(lexer->lookahead) && !is_newline(lexer->lookahead)) {
      lexer->advance(lexer, false);
    }

    if (is_newline(lexer->lookahead)) {
      lexer->result_symbol = T_CODE_BLOCK_END;
      return true;
    }
  }

  // If this could be the start of a fenced code block, check if the info string contains any backticks.
  if (valid_symbols[T_CODE_BLOCK_START]) {
    // Ensure there is a language string and it is only alphanumeric characters
    if (!is_alpha_numeric(lexer->lookahead)) {
      return false;
    }

    // Ensure there are no backticks in the language string
    while (!is_newline(lexer->lookahead)) {
      if (!is_alpha_numeric(lexer->lookahead)) {
        return false;
      }

      lexer->advance(lexer, false);
    }

    // If it does not then choose to interpret this as the start of a fenced code block.
    lexer->result_symbol = T_CODE_BLOCK_START;
    return true;
  }

  return false;
}

static bool parse(TSLexer* lexer, const bool* valid_symbols)
{
  // If all valid symbols are true, tree-sitter is in correction mode.
  // We don't want to parse anything in that case.
  if (valid_symbols[T_INVALID_TOKEN]) {
    return false;
  }

  // Text is only valid if we are not on a new line. We would have parsed all of the text before this point.
  if (valid_symbols[T_TAGTEXT] && lexer->get_column(lexer) != 0) {
    return parse_tagtext(lexer, valid_symbols);
  }

  if (lexer->get_column(lexer) != 0) {
    return false;
  }

  if (valid_symbols[T_TAGNAME] && (is_upper(lexer->lookahead) || is_possible_start_of_tag(lexer->lookahead))) {
    return parse_tagname(lexer, valid_symbols);
  }

  if ((valid_symbols[T_CODE_BLOCK_START] || valid_symbols[T_CODE_BLOCK_END]) && lexer->lookahead == CHAR_BACK_TICK) {
    return parse_code_block(lexer, valid_symbols);
  }

  return false;
}
