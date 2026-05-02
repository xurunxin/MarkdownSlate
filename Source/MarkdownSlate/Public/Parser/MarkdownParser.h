#pragma once

#include "CoreMinimal.h"
#include "Markdown/MarkdownAst.h"

class MARKDOWNSLATE_API FMarkdownParser
{
public:
	FMarkdownParser();

	TSharedPtr<FMarkdownBlockNode> Parse(const FString& MarkdownText);

	void SetParseFlags(unsigned InFlags) { ParseFlags = InFlags; }
	unsigned GetParseFlags() const { return ParseFlags; }

private:
	unsigned ParseFlags;
};
