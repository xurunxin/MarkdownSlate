#pragma once

#include "CoreMinimal.h"

class SWidget;

class MARKDOWNSLATE_API IMarkdownMathRenderer
{
public:
	virtual ~IMarkdownMathRenderer() = default;
	virtual TSharedRef<SWidget> RenderInlineMath(const FString& Latex) const = 0;
	virtual TSharedRef<SWidget> RenderDisplayMath(const FString& Latex) const = 0;
};

class MARKDOWNSLATE_API FMarkdownMathFallback : public IMarkdownMathRenderer
{
public:
	virtual TSharedRef<SWidget> RenderInlineMath(const FString& Latex) const override;
	virtual TSharedRef<SWidget> RenderDisplayMath(const FString& Latex) const override;
};
