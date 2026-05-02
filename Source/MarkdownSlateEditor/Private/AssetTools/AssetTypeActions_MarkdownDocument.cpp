#include "AssetTypeActions_MarkdownDocument.h"
#include "Assets/MarkdownDocumentAsset.h"

FAssetTypeActions_MarkdownDocument::FAssetTypeActions_MarkdownDocument() {}

FText FAssetTypeActions_MarkdownDocument::GetName() const { return NSLOCTEXT("MarkdownSlate", "MarkdownDoc", "Markdown Document"); }
FColor FAssetTypeActions_MarkdownDocument::GetTypeColor() const { return FColor(100, 160, 220); }
UClass* FAssetTypeActions_MarkdownDocument::GetSupportedClass() const { return UMarkdownDocumentAsset::StaticClass(); }
uint32 FAssetTypeActions_MarkdownDocument::GetCategories() { return EAssetTypeCategories::Misc; }
