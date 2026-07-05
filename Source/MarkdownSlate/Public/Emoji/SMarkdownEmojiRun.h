#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "Emoji/MarkdownEmojiTypes.h"

class IMarkdownEmojiAssetProvider;

class MARKDOWNSLATE_API SMarkdownEmojiRun : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMarkdownEmojiRun)
		: _Run()
		, _FontSize(12)
		, _FontInfo()
		, _EmojiFontInfo()
		, _TextColor(FLinearColor::White)
		, _EmojiProvider(nullptr)
		, _Config()
	{}
		SLATE_ARGUMENT(FMarkdownEmojiRun, Run)
		SLATE_ARGUMENT(int32, FontSize)
		SLATE_ARGUMENT(FSlateFontInfo, FontInfo)
		SLATE_ARGUMENT(FSlateFontInfo, EmojiFontInfo)
		SLATE_ARGUMENT(FLinearColor, TextColor)
		SLATE_ARGUMENT(IMarkdownEmojiAssetProvider*, EmojiProvider)
		SLATE_ARGUMENT(FMarkdownEmojiConfig, Config)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
