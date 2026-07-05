#pragma once

#include "CoreMinimal.h"
#include "MarkdownEmojiTypes.generated.h"

UENUM(BlueprintType)
enum class EMarkdownEmojiRenderMode : uint8
{
	PlatformFontFirst  UMETA(DisplayName = "Platform Font First"),
	TwemojiFirst      UMETA(DisplayName = "Twemoji First"),
	TextOnly           UMETA(DisplayName = "Text Only (Debug)"),
};

struct MARKDOWNSLATE_API FMarkdownEmojiRun
{
	FString EmojiSequence;     // Original emoji text, e.g. "😀" or "👨‍👩‍👧‍👦"
	FString TwemojiCode;       // Twemoji codepoint filename, e.g. "1f600" or "2764-fe0f"
	bool bIsEmoji = false;     // true = emoji run, false = plain text run
};

struct MARKDOWNSLATE_API FMarkdownEmojiConfig
{
	bool bEnableEmojiRendering = true;
	EMarkdownEmojiRenderMode RenderMode = EMarkdownEmojiRenderMode::TwemojiFirst;
	FString TwemojiAssetRoot = TEXT("Content/Emoji");
	float EmojiSizeScale = 1.0f;
	float EmojiBaselineOffset = 0.0f;
	bool bAllowTwemojiFallback = true;
};
