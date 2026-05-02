#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"

bool FMarkdownAtlasEmojiProvider::SupportsAtlasRendering() const
{
	return Atlas.IsValid() && Atlas->HasAtlasTexture();
}

FMarkdownAtlasEmojiProvider::FMarkdownAtlasEmojiProvider()
	: Atlas(MakeShared<FMarkdownEmojiAtlas>())
{
}

FMarkdownAtlasEmojiProvider::FMarkdownAtlasEmojiProvider(const FMarkdownEmojiConfig& InConfig)
	: Config(InConfig)
	, Atlas(MakeShared<FMarkdownEmojiAtlas>())
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
