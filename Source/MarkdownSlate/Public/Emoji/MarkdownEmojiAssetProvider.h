#pragma once

#include "CoreMinimal.h"
#include "Emoji/MarkdownEmojiTypes.h"

class FMarkdownEmojiAtlas;

class MARKDOWNSLATE_API IMarkdownEmojiAssetProvider
{
public:
	virtual ~IMarkdownEmojiAssetProvider() = default;
	virtual const FSlateBrush* GetEmojiBrush(const FString& TwemojiCode, float FontSize) = 0;
	virtual bool HasEmojiBrush(const FString& TwemojiCode) const = 0;
	virtual bool SupportsAtlasRendering() const { return false; }
};

class MARKDOWNSLATE_API FMarkdownAtlasEmojiProvider : public IMarkdownEmojiAssetProvider
{
public:
	FMarkdownAtlasEmojiProvider();
	explicit FMarkdownAtlasEmojiProvider(const FMarkdownEmojiConfig& InConfig);

	void SetAtlasTexture(UTexture2D* InTexture);
	TSharedPtr<FMarkdownEmojiAtlas> GetAtlas() const { return Atlas; }
	FMarkdownEmojiAtlas* GetAtlasPtr() const { return Atlas.Get(); }

	virtual const FSlateBrush* GetEmojiBrush(const FString& TwemojiCode, float FontSize) override;
	virtual bool HasEmojiBrush(const FString& TwemojiCode) const override;
	virtual bool SupportsAtlasRendering() const override;

private:
	FMarkdownEmojiConfig Config;
	TSharedPtr<FMarkdownEmojiAtlas> Atlas;
};
