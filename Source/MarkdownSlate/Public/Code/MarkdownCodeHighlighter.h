#pragma once

#include "CoreMinimal.h"

struct MARKDOWNSLATE_API FMarkdownCodeToken { FString Text; int32 TokenType = 0; };

class MARKDOWNSLATE_API IMarkdownCodeHighlighter
{
public:
	virtual ~IMarkdownCodeHighlighter() = default;
	virtual TArray<FMarkdownCodeToken> Tokenize(const FString& Code, const FString& Language) const = 0;
};

class MARKDOWNSLATE_API FMarkdownPlainTextHighlighter : public IMarkdownCodeHighlighter
{
public:
	virtual TArray<FMarkdownCodeToken> Tokenize(const FString& Code, const FString& Language) const override;
};
