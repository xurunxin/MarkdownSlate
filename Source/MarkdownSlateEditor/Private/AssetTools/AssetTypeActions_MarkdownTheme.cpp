#include "AssetTypeActions_MarkdownTheme.h"
#include "Style/MarkdownThemeAsset.h"

FAssetTypeActions_MarkdownTheme::FAssetTypeActions_MarkdownTheme() {}

FText FAssetTypeActions_MarkdownTheme::GetName() const { return NSLOCTEXT("MarkdownSlate", "MarkdownTheme", "Markdown Theme"); }
FColor FAssetTypeActions_MarkdownTheme::GetTypeColor() const { return FColor(220, 180, 100); }
UClass* FAssetTypeActions_MarkdownTheme::GetSupportedClass() const { return UMarkdownThemeAsset::StaticClass(); }
uint32 FAssetTypeActions_MarkdownTheme::GetCategories() { return EAssetTypeCategories::Misc; }
