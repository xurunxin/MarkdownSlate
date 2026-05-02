#include "Math/MarkdownMathRenderer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> FMarkdownMathFallback::RenderInlineMath(const FString& Latex) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(TEXT("$") + Latex + TEXT("$")))
		.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11));
}

TSharedRef<SWidget> FMarkdownMathFallback::RenderDisplayMath(const FString& Latex) const
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.08, 0.08, 0.1))
		.Padding(FMargin(8, 4))
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("$$") + Latex + TEXT("$$")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 13))
		];
}
