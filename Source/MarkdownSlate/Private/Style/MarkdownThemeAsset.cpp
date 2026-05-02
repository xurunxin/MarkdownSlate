#include "Style/MarkdownThemeAsset.h"

#if WITH_EDITOR
void UMarkdownThemeAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	++Generation;
}
#endif
