#pragma once

#include "CoreMinimal.h"

struct FMarkdownEmojiAtlasCodepointEntry
{
	const TCHAR* Codepoint;
	int32 Page;
	int32 Col;
	int32 Row;
};

namespace MarkdownEmojiAtlasData
{
	extern const int32 CellSize;
	extern const int32 AtlasSize;
	extern const int32 PageCount;
	extern const int32 EntryCount;
	extern const FMarkdownEmojiAtlasCodepointEntry Entries[];
}
