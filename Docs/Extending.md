# Extending MarkdownSlate

## Custom Image Loader

Implement `IMarkdownImageLoader` and assign to `FMarkdownSlateThemeConfig::EmojiProvider`.

```cpp
class FMyImageLoader : public IMarkdownImageLoader
{
    virtual void LoadImage(const FString& Url, FOnMarkdownImageLoaded OnLoaded, FOnMarkdownImageFailed OnFailed) override;
    virtual void CancelAll() override;
};
```

## Custom Code Highlighter

Implement `IMarkdownCodeHighlighter` for syntax highlighting:

```cpp
class FMyHighlighter : public IMarkdownCodeHighlighter
{
    virtual TArray<FMarkdownCodeToken> Tokenize(const FString& Code, const FString& Language) const override;
};
```

## Custom Emoji Provider

Use `FMarkdownAtlasEmojiProvider` with a custom atlas texture:

```cpp
auto Atlas = MakeShared<FMarkdownEmojiAtlas>();
Atlas->SetAtlasTexture(MyTexture);
Atlas->AddEntry("1f600", Col, Row); // Add emoji codepoint mapping
Provider->SetAtlasTexture(Atlas->GetAtlasTexture());
```

## Widget Embed Extension

Implement `IMarkdownWidgetProvider` to handle custom `:widget[Type]` syntax:

```cpp
class FMyWidgetProvider : public IMarkdownWidgetProvider
{
    virtual bool SupportsWidget(const FString& WidgetType) const override;
    virtual TSharedRef<SWidget> CreateWidget(const FString& WidgetType, const TMap<FString, FString>& Attributes) override;
};
```
