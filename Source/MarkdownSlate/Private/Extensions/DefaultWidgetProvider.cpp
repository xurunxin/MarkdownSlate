#include "Extensions/MarkdownExtension.h"
#include "Widgets/Text/STextBlock.h"

class FDefaultWidgetProvider : public IMarkdownWidgetProvider
{
public:
	virtual FName GetExtensionName() const override { return FName(TEXT("DefaultWidgetProvider")); }
	virtual bool SupportsWidget(const FString& WidgetType) const override { return false; }
	virtual TSharedRef<SWidget> CreateWidget(const FString& WidgetType, const TMap<FString, FString>& Attributes) override
	{
		return SNew(STextBlock).Text(FText::FromString(TEXT("[Widget: ") + WidgetType + TEXT("]")));
	}
};
