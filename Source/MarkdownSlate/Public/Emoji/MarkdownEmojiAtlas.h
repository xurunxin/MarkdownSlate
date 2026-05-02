#pragma once

#include "CoreMinimal.h"
#include "Emoji/MarkdownEmojiTypes.h"

struct MARKDOWNSLATE_API FEmojiAtlasEntry
{
	FString Codepoint;
	int32 Page = 0;
	int32 GridCol = -1;
	int32 GridRow = -1;
};

class MARKDOWNSLATE_API FMarkdownEmojiAtlas
{
public:
	FMarkdownEmojiAtlas();
	~FMarkdownEmojiAtlas();

	void SetAtlasConfig(int32 InCellSize, int32 InAtlasSize);
	void SetAtlasTexture(UTexture2D* InTexture);
	void SetAtlasTexture(int32 Page, UTexture2D* InTexture);

	// Map emoji Twemoji codepoint (e.g. "1f600") to atlas grid coordinates
	bool GetGridCoords(const FString& TwemojiCode, int32& OutCol, int32& OutRow) const;

	// Get UV rect (0-1) for the given grid cell
	bool GetUVRect(const FString& TwemojiCode, FVector2D& OutUVMin, FVector2D& OutUVMax) const;

	// Add/register entries
	void AddEntry(const FString& Codepoint, int32 Page, int32 Col, int32 Row);
	void AddEntry(const FString& Codepoint, int32 Col, int32 Row);
	int32 GetEntryCount() const { return Entries.Num(); }

	// Get the brush for an emoji at the given font size
	TSharedPtr<FSlateBrush> GetEmojiBrush(const FString& TwemojiCode, float FontSize);

	// Check if we have a texture
	bool HasAtlasTexture() const { return AtlasTextures.Num() > 0; }

	// Build default grid mapping (sequential codepoints → grid cells)
	void BuildDefaultMapping(const TArray<FString>& SortedCodepoints, int32 CellsPerRow);

	// Load cooked atlas texture asset + built-in mapping.
	bool AutoLoadAtlas(const FString& ContentEmojiPath);

	int32 GetAtlasSize() const { return AtlasSize; }
	int32 GetCellSize() const { return CellSize; }

private:
	void LoadBuiltInMapping();
	bool ResolveAtlasEntry(const FString& TwemojiCode, const FEmojiAtlasEntry*& OutEntry) const;

	TArray<UTexture2D*> AtlasTextures;
	int32 CellSize = 72;
	int32 AtlasSize = 4096;
	int32 PageCount = 1;
	int32 GridCols = 0;
	TMap<FString, FEmojiAtlasEntry> Entries;
	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;
};
