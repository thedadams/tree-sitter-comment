package tree_sitter_comment_test

import (
	"testing"

	tree_sitter_comment "github.com/thedadams/tree-sitter-comment/bindings/go"
	tree_sitter "github.com/tree-sitter/go-tree-sitter"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_comment.Language())
	if language == nil {
		t.Errorf("Error loading Comment grammar")
	}
}
