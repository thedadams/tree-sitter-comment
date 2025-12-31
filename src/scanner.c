#include <tree_sitter/parser.h>

#include "chars.c"
#include "chars.h"

#include "tokens.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

/// Parse the name of the tag.
///
/// They can be of the form:
/// TODO
/// TODO:
/// TODO: text
/// TODO(thedadams)
/// TODO(thedadams):
/// TODO(thedadams): text
/// TODO (thedadams) : text
static bool parse_tagname(TSLexer *lexer, bool mark) {
    if (!is_upper(lexer->lookahead)) {
        return false;
    }

    int32_t previous = lexer->lookahead;
    lexer->advance(lexer, false);

    while (is_upper(lexer->lookahead) || is_digit(lexer->lookahead) || is_internal_char(lexer->lookahead)) {
        previous = lexer->lookahead;
        lexer->advance(lexer, false);
    }
    // The tag name ends here.
    // But we keep parsing to see if it's a valid tag name.
    if (mark) {
        lexer->mark_end(lexer);
    }

    // It can't end with an internal char.
    if (is_internal_char(previous) || is_alpha(lexer->lookahead)) {
        return false;
    }

    // For the user component this is `\s*(`.
    // We don't parse that part, we just need to be sure it ends with `:\s`.
    if ((is_space(lexer->lookahead)) || lexer->lookahead == '(') {
        // Skip white spaces.
        while (is_space(lexer->lookahead)) {
            lexer->advance(lexer, false);
        }
        // Checking aperture.
        if (lexer->lookahead != '(') {
            lexer->result_symbol = T_TAGNAME;
            return true;
        }
        lexer->advance(lexer, false);

        int paren_open = 1;
        // Checking closure.
        while (true) {
            if (is_newline(lexer->lookahead) || lexer->eof(lexer)) {
                return false;
            }

            if (lexer->lookahead == '(') {
                paren_open++;
            } else if (lexer->lookahead == ')') {
                paren_open--;
                if (paren_open == 0) {
                    break;
                }
            }
            lexer->advance(lexer, false);
        }
    }

    if (mark) {
        lexer->result_symbol = T_TAGNAME;
    }
    return true;
}

static bool parse_tagprefix(TSLexer *lexer) {
    while (is_space(lexer->lookahead)) {
        lexer->advance(lexer, false);
    }

    if (!is_possible_start_of_tag(lexer->lookahead)) {
        return false;
    }

    while (is_possible_start_of_tag(lexer->lookahead)) {
        lexer->advance(lexer, false);
    }

    while (is_space(lexer->lookahead)) {
        lexer->advance(lexer, false);
    }

    lexer->mark_end(lexer);

    if (is_newline(lexer->lookahead) || lexer->eof(lexer) || !parse_tagname(lexer, false)) {
        return false;
    }

    lexer->result_symbol = T_TAGPREFIX;
    return true;
}

static bool parse_taguser(TSLexer *lexer) {
    int paren_open = 1;
    while (true) {
        if (is_newline(lexer->lookahead)) {
            return false;
        }

        if (lexer->lookahead == '(') {
            paren_open++;
        } else if (lexer->lookahead == ')') {
            paren_open--;
            if (paren_open == 0) {
                lexer->result_symbol = T_TAGUSER;
                return true;
            }
        }
        lexer->advance(lexer, false);
    }
}

static bool parse_tagtext(TSLexer *lexer) {
    while (is_space(lexer->lookahead)) {
        lexer->advance(lexer, false);
    }

    if (lexer->lookahead == '(') {
        return false;
    }

    bool has_text = false;
    while (!lexer->eof(lexer)) {
        while (is_possible_start_of_tag(lexer->lookahead) || is_space(lexer->lookahead)) {
            has_text = has_text || !is_space(lexer->lookahead);
            lexer->advance(lexer, false);
        }

        if (is_newline(lexer->lookahead) || lexer->eof(lexer) || !has_text && lexer->lookahead == '(') {
            if (!has_text) {
                return false;
            }

            lexer->result_symbol = T_TAGTEXT;
            return true;
        }

        has_text = true;

        while (!is_newline(lexer->lookahead) && !lexer->eof(lexer)) {
            lexer->advance(lexer, false);
        }

        lexer->mark_end(lexer);

        if (!lexer->eof(lexer)) {
            lexer->advance(lexer, false);
        }
    }

    if (!has_text) {
        return false;
    }

    lexer->result_symbol = T_TAGTEXT;
    return true;
}

void *tree_sitter_comment_external_scanner_create() { return NULL; }

void tree_sitter_comment_external_scanner_destroy(void *payload) {}

unsigned tree_sitter_comment_external_scanner_serialize(void *payload, char *buffer) { return 0; }

void tree_sitter_comment_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {}

bool tree_sitter_comment_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
    // If all valid symbols are true, tree-sitter is in correction mode.
    // We don't want to parse anything in that case.
    if (valid_symbols[T_INVALID_TOKEN]) {
        return false;
    }

    if (lexer->get_column(lexer) != 0) {
        if (valid_symbols[T_TAGTEXT]) {
            return parse_tagtext(lexer);
        } else if (valid_symbols[T_TAGUSER]) {
            return parse_taguser(lexer);
        }
    } else if (valid_symbols[T_TAGPREFIX] && (is_possible_start_of_tag(lexer->lookahead)) ||
               is_space(lexer->lookahead)) {
        return parse_tagprefix(lexer);
    }

    // Tag names are only valid if they are at the beginning of the line or after a tag prefix.
    // We know this is after a tag prefix if T_TAGPREFIX is not valid.
    if ((lexer->get_column(lexer) == 0 || !valid_symbols[T_TAGPREFIX]) && valid_symbols[T_TAGNAME]) {
        return parse_tagname(lexer, true);
    }

    return false;
}
