#include "Factories/MarkdownDocumentAssetFactory.h"
#include "Assets/MarkdownDocumentAsset.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UMarkdownDocumentAssetFactory::UMarkdownDocumentAssetFactory()
{
	Formats.Add(TEXT("md;Markdown Document"));
	Formats.Add(TEXT("markdown;Markdown Document"));
	Formats.Add(TEXT("mdx;Markdown Document (MDX)"));
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	SupportedClass = UMarkdownDocumentAsset::StaticClass();
}

bool UMarkdownDocumentAssetFactory::FactoryCanImport(const FString& Filename)
{
	FString Ext = FPaths::GetExtension(Filename).ToLower();
	return Ext == TEXT("md") || Ext == TEXT("markdown") || Ext == TEXT("mdx");
}

UObject* UMarkdownDocumentAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName,
	EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *Filename))
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load file: %s"), *Filename);
		return nullptr;
	}

	UMarkdownDocumentAsset* Asset = NewObject<UMarkdownDocumentAsset>(InParent, InClass, InName, Flags);
	Asset->MarkdownContent = Content;
	Asset->SourceFilename = FName(*FPaths::GetCleanFilename(Filename));

	return Asset;
}
