#include "Emoji/SMarkdownEmojiRun.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "Emoji/MarkdownEmojiScanner.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

void SMarkdownEmojiRun::Construct(const FArguments& InArgs)
{
	const FMarkdownEmojiRun& Run = InArgs._Run;
	float FontSize = (float)InArgs._FontSize;
	const FSlateFontInfo& UseFont = InArgs._FontInfo;
	const FSlateFontInfo& EmojiFontInfo = InArgs._EmojiFontInfo;
	FLinearColor Color = InArgs._TextColor;
	IMarkdownEmojiAssetProvider* Provider = InArgs._EmojiProvider;
	const FMarkdownEmojiConfig& Config = InArgs._Config;

	auto FontHasData = [](const FSlateFontInfo& Font) {
		return Font.FontObject != nullptr ||
			(Font.CompositeFont.IsValid() && Font.CompositeFont->DefaultTypeface.Fonts.Num() > 0);
	};

	auto MakeText = [&](const FSlateFontInfo& SourceFont, bool bSafeEmojiFallback) -> TSharedRef<SWidget> {
		FSlateFontInfo Font = SourceFont;
		Font.Size = FontSize;
		return SNew(STextBlock)
			.Text(FText::FromString(Run.bIsEmoji
				? (bSafeEmojiFallback ? FMarkdownEmojiScanner::MakeSafeTextFallback(Run.EmojiSequence) : Run.EmojiSequence)
				: Run.EmojiSequence))
			.Font(Font)
			.ColorAndOpacity(FSlateColor(Color));
	};

	auto MakeAtlasImage = [&]() -> TSharedPtr<SWidget> {
		if (!Provider)
		{
			return nullptr;
		}

		const float Size = FMath::Max(1.0f, FMath::RoundToFloat(FontSize * Config.EmojiSizeScale));
		const FSlateBrush* Brush = Provider->GetEmojiBrush(Run.TwemojiCode, Size);
		if (!Brush)
		{
			return nullptr;
		}

		TSharedRef<SWidget> ImageWidget = SNew(SImage)
			.Image(Brush)
			.DesiredSizeOverride(FVector2D(Size));
		return ImageWidget;
	};

	if (!Run.bIsEmoji) { ChildSlot[MakeText(UseFont, false)]; return; }

	const bool bHasEmojiFont = FontHasData(EmojiFontInfo);
	const FSlateFontInfo& TextEmojiFont = bHasEmojiFont ? EmojiFontInfo : UseFont;

	if (Config.RenderMode == EMarkdownEmojiRenderMode::TextOnly)
	{
		ChildSlot[MakeText(TextEmojiFont, !bHasEmojiFont)];
		return;
	}

	if (Config.RenderMode == EMarkdownEmojiRenderMode::TwemojiFirst)
	{
		TSharedPtr<SWidget> AtlasImage = MakeAtlasImage();
		if (AtlasImage.IsValid())
		{
			ChildSlot[AtlasImage.ToSharedRef()];
			return;
		}

		if (bHasEmojiFont)
		{
			ChildSlot[MakeText(TextEmojiFont, false)];
			return;
		}
	}
	else if (Config.RenderMode == EMarkdownEmojiRenderMode::PlatformFontFirst)
	{
		if (bHasEmojiFont)
		{
			ChildSlot[MakeText(TextEmojiFont, false)];
			return;
		}

		if (Config.bAllowTwemojiFallback)
		{
			TSharedPtr<SWidget> AtlasImage = MakeAtlasImage();
			if (AtlasImage.IsValid())
			{
				ChildSlot[AtlasImage.ToSharedRef()];
				return;
			}
		}
	}
	else if (Config.bAllowTwemojiFallback)
	{
		TSharedPtr<SWidget> AtlasImage = MakeAtlasImage();
		if (AtlasImage.IsValid())
		{
			ChildSlot[AtlasImage.ToSharedRef()];
			return;
		}
	}

	ChildSlot[MakeText(TextEmojiFont, !bHasEmojiFont)];
}
