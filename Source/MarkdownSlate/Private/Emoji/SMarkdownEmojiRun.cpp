#include "Emoji/SMarkdownEmojiRun.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

void SMarkdownEmojiRun::Construct(const FArguments& InArgs)
{
	const FMarkdownEmojiRun& Run = InArgs._Run;
	float FontSize = (float)InArgs._FontSize;
	const FSlateFontInfo& UseFont = InArgs._FontInfo;
	FLinearColor Color = InArgs._TextColor;
	IMarkdownEmojiAssetProvider* Provider = InArgs._EmojiProvider;
	const FMarkdownEmojiConfig& Config = InArgs._Config;

	auto MakeText = [&]() -> TSharedRef<SWidget> {
		FSlateFontInfo Font = UseFont;
		Font.Size = FontSize;
		return SNew(STextBlock)
			.Text(FText::FromString(Run.EmojiSequence))
			.Font(Font)
			.ColorAndOpacity(FSlateColor(Color));
	};

	if (!Run.bIsEmoji) { ChildSlot[MakeText()]; return; }

	if (Provider && Provider->SupportsAtlasRendering())
	{
		const FSlateBrush* Brush = Provider->GetEmojiBrush(Run.TwemojiCode, FontSize);
		if (Brush) {
			float S = FontSize * Config.EmojiSizeScale;
			ChildSlot[SNew(SImage).Image(Brush).DesiredSizeOverride(FVector2D(S))]; return;
		}
	}

	if (Config.RenderMode == EMarkdownEmojiRenderMode::TwemojiFirst && Provider)
	{
		const FSlateBrush* Brush = Provider->GetEmojiBrush(Run.TwemojiCode, FontSize);
		if (Brush) {
			float S = FontSize * Config.EmojiSizeScale;
			ChildSlot[SNew(SImage).Image(Brush).DesiredSizeOverride(FVector2D(S))]; return;
		}
	}

	ChildSlot[MakeText()];
}
