#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

namespace
{
	TSharedPtr<FMarkdownEmojiAtlas> AcquireSharedEmojiAtlas(const FString& AssetRoot)
	{
		static FCriticalSection AtlasCacheMutex;
		static TMap<FString, TSharedPtr<FMarkdownEmojiAtlas>> AtlasCache;

		FScopeLock Lock(&AtlasCacheMutex);
		if (const TSharedPtr<FMarkdownEmojiAtlas>* ExistingAtlas = AtlasCache.Find(AssetRoot))
		{
			return *ExistingAtlas;
		}

		TSharedPtr<FMarkdownEmojiAtlas> NewAtlas = MakeShared<FMarkdownEmojiAtlas>();
		AtlasCache.Add(AssetRoot, NewAtlas);
		return NewAtlas;
	}
}

bool FMarkdownAtlasEmojiProvider::SupportsAtlasRendering() const
{
	return Atlas.IsValid() && Atlas->HasAtlasTexture();
}

FMarkdownAtlasEmojiProvider::FMarkdownAtlasEmojiProvider()
	: Atlas(AcquireSharedEmojiAtlas(Config.TwemojiAssetRoot))
{
}

FMarkdownAtlasEmojiProvider::FMarkdownAtlasEmojiProvider(const FMarkdownEmojiConfig& InConfig)
	: Config(InConfig)
	, Atlas(AcquireSharedEmojiAtlas(Config.TwemojiAssetRoot))
{
}

void FMarkdownAtlasEmojiProvider::SetAtlasTexture(UTexture2D* InTexture)
{
	Atlas->SetAtlasTexture(InTexture);
}

const FSlateBrush* FMarkdownAtlasEmojiProvider::GetEmojiBrush(const FString& TwemojiCode, float FontSize)
{
	TSharedPtr<FSlateBrush> Brush = Atlas->GetEmojiBrush(TwemojiCode, FontSize);
	if (Brush.IsValid()) return Brush.Get();
	return nullptr;
}

bool FMarkdownAtlasEmojiProvider::HasEmojiBrush(const FString& TwemojiCode) const
{
	int32 Col, Row;
	return Atlas->GetGridCoords(TwemojiCode, Col, Row);
}
