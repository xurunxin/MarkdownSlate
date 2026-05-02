#pragma once

#include "CoreMinimal.h"

class SWidget;

class MARKDOWNSLATE_API IMarkdownExtension
{
public:
	virtual ~IMarkdownExtension() = default;
	virtual FName GetExtensionName() const = 0;
};

class MARKDOWNSLATE_API IMarkdownWidgetProvider : public IMarkdownExtension
{
public:
	virtual bool SupportsWidget(const FString& WidgetType) const = 0;
	virtual TSharedRef<SWidget> CreateWidget(const FString& WidgetType, const TMap<FString, FString>& Attributes) = 0;
};
