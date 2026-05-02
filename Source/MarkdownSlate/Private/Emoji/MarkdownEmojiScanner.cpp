#include "Emoji/MarkdownEmojiScanner.h"

FMarkdownEmojiScanner::FMarkdownEmojiScanner() {}
FMarkdownEmojiScanner::FMarkdownEmojiScanner(const FMarkdownEmojiConfig& InConfig) : Config(InConfig) {}

bool FMarkdownEmojiScanner::IsZWJ(uint32 C) { return C == 0x200D; }
bool FMarkdownEmojiScanner::IsVariationSelector(uint32 C) { return C >= 0xFE00 && C <= 0xFE0F; }
bool FMarkdownEmojiScanner::IsSkinTone(uint32 C) { return C >= 0x1F3FB && C <= 0x1F3FF; }
bool FMarkdownEmojiScanner::IsRegionalIndicator(uint32 C) { return C >= 0x1F1E6 && C <= 0x1F1FF; }
bool FMarkdownEmojiScanner::IsKeycap(uint32 C) { return C == 0x20E3; }

bool FMarkdownEmojiScanner::IsEmojiModifier(uint32 C)
{
	return IsVariationSelector(C) || IsSkinTone(C) || IsZWJ(C) || IsKeycap(C);
}

// Read a full Unicode codepoint (handles UTF-16 surrogate pairs)
static bool IsHighSurrogate(TCHAR C) { return C >= 0xD800 && C <= 0xDBFF; }
static bool IsLowSurrogate(TCHAR C) { return C >= 0xDC00 && C <= 0xDFFF; }

static uint32 ReadCodepoint(const FString& Text, int32& InOutIndex)
{
	if (InOutIndex >= Text.Len()) return 0;
	uint32 C = Text[InOutIndex++];
	if (IsHighSurrogate((TCHAR)C) && InOutIndex < Text.Len())
	{
		uint32 Lo = Text[InOutIndex];
		if (IsLowSurrogate((TCHAR)Lo))
		{
			InOutIndex++;
			return 0x10000 + ((C - 0xD800) << 10) + (Lo - 0xDC00);
		}
	}
	return C;
}

bool FMarkdownEmojiScanner::IsEmojiCodepoint(uint32 C)
{
	return (C >= 0x1F600 && C <= 0x1F64F)   // Emoticons
		|| (C >= 0x1F300 && C <= 0x1F5FF)   // Misc Symbols
		|| (C >= 0x1F680 && C <= 0x1F6FF)   // Transport
		|| (C >= 0x1F900 && C <= 0x1F9FF)   // Supplemental
		|| (C >= 0x1FA00 && C <= 0x1FAFF)   // Extended-A
		|| (C >= 0x2600 && C <= 0x27BF)     // Misc Symbols / Dingbats
		|| (C >= 0x2300 && C <= 0x23FF)     // Misc Technical
		|| (C >= 0x2B00 && C <= 0x2BFF)     // Misc Symbols & Arrows
		|| (C >= 0x1F1E6 && C <= 0x1F1FF)   // Regional Indicators
		|| (C >= 0x1F3FB && C <= 0x1F3FF)   // Skin Tones
		|| C == 0x200D                       // ZWJ
		|| C == 0x20E3                       // Keycap
		|| C == 0x2764 || C == 0x2B50       // Heart, Star
		|| C == 0x00A9 || C == 0x00AE       // Copyright, Registered
		|| (C >= 0x2122 && C <= 0x2139)     // TM
		|| (C >= 0x231A && C <= 0x231B)     // Watch, Hourglass
		|| (C >= 0x2328 && C <= 0x23CF)     // Keyboard
		|| C == 0x24C2
		|| (C >= 0x25AA && C <= 0x25FE)
		|| (C >= 0x2934 && C <= 0x2935)
		|| (C >= 0x2B05 && C <= 0x2B07)
		|| (C >= 0x2B1B && C <= 0x2B1C)
		|| C == 0x2B55 || C == 0x3030 || C == 0x303D
		|| C == 0x3297 || C == 0x3299
		|| (C >= 0x1F000 && C <= 0x1F02F)
		|| (C >= 0x1F0A0 && C <= 0x1F0FF)
		|| (C >= 0x1F100 && C <= 0x1F1FF)
		|| (C >= 0x1F200 && C <= 0x1F251);   // Various
}

int32 FMarkdownEmojiScanner::GetEmojiSequenceLength(const FString& Text, int32 StartIndex) const
{
	int32 Idx = StartIndex;
	uint32 First = ReadCodepoint(Text, Idx);
	if (!IsEmojiCodepoint(First)) return 0;

	int32 Length = Idx - StartIndex;
	bool bFirstIsRI = IsRegionalIndicator(First);

	while (Idx < Text.Len())
	{
		int32 PeekIdx = Idx;
		uint32 Peek = ReadCodepoint(Text, PeekIdx);

		// Skin tone always follows an emoji
		if (IsSkinTone(Peek))
		{
			Length += (PeekIdx - Idx);
			Idx = PeekIdx;
			continue;
		}

		// Variation selector-16 (emoji presentation)
		if (Peek == 0xFE0F)
		{
			Length += (PeekIdx - Idx);
			Idx = PeekIdx;
			continue;
		}

		// Regional indicator pair (flags)
		if (bFirstIsRI && IsRegionalIndicator(Peek))
		{
			Length += (PeekIdx - Idx);
			Idx = PeekIdx;
			break; // Flag = exactly 2 RI
		}

		// Keycap sequence (digit/star + U+FE0F + U+20E3)
		if (IsKeycap(Peek))
		{
			Length += (PeekIdx - Idx);
			Idx = PeekIdx;
			break;
		}

		// ZWJ sequence (emoji + ZWJ + next emoji)
		if (IsZWJ(Peek))
		{
			Length += (PeekIdx - Idx);
			Idx = PeekIdx;
			// Read past ZWJ, expecting another emoji
			if (Idx < Text.Len())
			{
				int32 AfterZWJIdx = Idx;
				uint32 AfterZWJ = ReadCodepoint(Text, Idx);
				if (IsEmojiCodepoint(AfterZWJ))
				{
					Length += (Idx - AfterZWJIdx);
					bFirstIsRI = IsRegionalIndicator(AfterZWJ); // Update for nested flags
					continue;
				}
				else
				{
					Idx = AfterZWJIdx; // Not an emoji, stop
					break;
				}
			}
			break;
		}

		break;
	}

	return Length;
}

TArray<FMarkdownEmojiRun> FMarkdownEmojiScanner::ScanText(const FString& Text) const
{
	TArray<FMarkdownEmojiRun> Runs;
	if (Text.IsEmpty() || !Config.bEnableEmojiRendering) return Runs;

	int32 Idx = 0;
	FString PlainBuf;

	while (Idx < Text.Len())
	{
		int32 EmojiLen = GetEmojiSequenceLength(Text, Idx);
		if (EmojiLen > 0)
		{
			if (!PlainBuf.IsEmpty())
			{
				FMarkdownEmojiRun Run;
				Run.EmojiSequence = PlainBuf;
				Run.bIsEmoji = false;
				Runs.Add(Run);
				PlainBuf.Reset();
			}

			FMarkdownEmojiRun ERun;
			ERun.EmojiSequence = Text.Mid(Idx, EmojiLen);
			ERun.TwemojiCode = EmojiToTwemojiCode(ERun.EmojiSequence);
			ERun.bIsEmoji = true;
			Runs.Add(ERun);
			Idx += EmojiLen;
		}
		else
		{
			PlainBuf += Text[Idx++];
		}
	}

	if (!PlainBuf.IsEmpty())
	{
		FMarkdownEmojiRun Run;
		Run.EmojiSequence = PlainBuf;
		Run.bIsEmoji = false;
		Runs.Add(Run);
	}

	return Runs;
}

FString FMarkdownEmojiScanner::EmojiToTwemojiCode(const FString& EmojiSequence)
{
	TArray<uint32> Codepoints;
	for (int32 i = 0; i < EmojiSequence.Len(); )
	{
		uint32 C = ReadCodepoint(EmojiSequence, i);
		if (C == 0xFE0F) continue;
		Codepoints.Add(C);
	}

	FString Result;
	for (int32 j = 0; j < Codepoints.Num(); ++j)
	{
		if (j > 0) Result += TEXT("-");
		Result += FString::Printf(TEXT("%x"), Codepoints[j]).ToLower();
	}
	return Result;
}
