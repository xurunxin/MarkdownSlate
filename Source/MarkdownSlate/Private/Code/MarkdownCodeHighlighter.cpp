#include "Code/MarkdownCodeHighlighter.h"

TArray<FMarkdownCodeToken> FMarkdownPlainTextHighlighter::Tokenize(const FString& Code, const FString& Language) const
{
	FMarkdownCodeToken Token;
	Token.Text = Code;
	Token.TokenType = 0;
	return { Token };
}
