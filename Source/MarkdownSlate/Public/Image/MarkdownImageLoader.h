#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_OneParam(FOnMarkdownImageLoaded, const TSharedPtr<FSlateBrush>& /*Brush*/);
DECLARE_DELEGATE(FOnMarkdownImageFailed);

class MARKDOWNSLATE_API IMarkdownImageLoader
{
public:
	virtual ~IMarkdownImageLoader() = default;

	virtual void LoadImage(const FString& Url,
		FOnMarkdownImageLoaded OnLoaded,
		FOnMarkdownImageFailed OnFailed = FOnMarkdownImageFailed()) = 0;

	virtual void CancelAll() = 0;
};

class MARKDOWNSLATE_API FMarkdownDefaultImageLoader : public IMarkdownImageLoader
{
public:
	virtual void LoadImage(const FString& Url,
		FOnMarkdownImageLoaded OnLoaded,
		FOnMarkdownImageFailed OnFailed = FOnMarkdownImageFailed()) override;

	virtual void CancelAll() override;

private:
	TArray<TSharedPtr<struct FMarkdownImageLoadTask>> PendingTasks;
};
