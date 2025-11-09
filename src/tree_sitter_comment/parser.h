#ifndef TREE_SITTER_COMMENT_PARSER_H
#define TREE_SITTER_COMMENT_PARSER_H

#include <tree_sitter/parser.h>

static bool parse_tagstart(TSLexer* lexer);
static bool parse_tagname(TSLexer* lexer, bool mark_end);
static bool parse_tagtext(TSLexer* lexer);
static bool parse(TSLexer* lexer, const bool* valid_symbols);

#endif /* ifndef TREE_SITTER_COMMENT_PARSER_H */
