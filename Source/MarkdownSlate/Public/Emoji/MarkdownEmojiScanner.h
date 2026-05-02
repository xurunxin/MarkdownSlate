#pragma once

#include "CoreMinimal.h"
#include "Emoji/MarkdownEmojiTypes.h"

class MARKDOWNSLATE_API FMarkdownEmojiScanner
{
public:
	FMarkdownEmojiScanner();
	explicit FMarkdownEmojiScanner(const FMarkdownEmojiConfig& InConfig);

	void SetConfig(const FMarkdownEmojiConfig& InConfig) { Config = InConfig; }

	// Split a plain text string into runs of text and emoji sequences
	TArray<FMarkdownEmojiRun> ScanText(const FString& Text) const;

	// Check if a codepoint is an emoji-capable character
	static bool IsEmojiCodepoint(uint32 Codepoint);

	// Convert emoji sequence to Twemoji codepoint filename (e.g. "1f600" or "2764-fe0f")
	static FString EmojiToTwemojiCode(const FString& EmojiSequence);

	// Make emoji text safe for plain Slate text fallback when no image provider is available.
	static FString MakeSafeTextFallback(const FString& Text);

private:
	FMarkdownEmojiConfig Config;

	// Get the length in TCHARs of the emoji sequence starting at position
	int32 GetEmojiSequenceLength(const FString& Text, int32 StartIndex) const;

	// Check for variation selector, ZWJ, skin tone, keycap, flag continuation
	static bool IsEmojiModifier(uint32 Codepoint);
	static bool IsVariationSelector(uint32 Codepoint);
	static bool IsSkinTone(uint32 Codepoint);
	static bool IsRegionalIndicator(uint32 Codepoint);
	static bool IsZWJ(uint32 Codepoint);
	static bool IsKeycap(uint32 Codepoint);
};
