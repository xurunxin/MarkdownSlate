#include "Misc/AutomationTest.h"
#include "Assets/MarkdownDocumentAsset.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownSlateEditorThemeAssetTest, "MarkdownSlate.Editor.CreateThemeAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownSlateEditorThemeAssetTest::RunTest(const FString& Parameters)
{
	// Verify ThemeAsset class is available
	UClass* ThemeClass = LoadClass<UDataAsset>(nullptr,
		TEXT("/Script/MarkdownSlate.MarkdownThemeAsset"));
	TestNotNull(TEXT("MarkdownThemeAsset class exists"), ThemeClass);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownSlateEditorDocumentAssetTest, "MarkdownSlate.Editor.CreateDocumentAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownSlateEditorDocumentAssetTest::RunTest(const FString& Parameters)
{
	UClass* DocClass = LoadClass<UDataAsset>(nullptr,
		TEXT("/Script/MarkdownSlate.MarkdownDocumentAsset"));
	TestNotNull(TEXT("MarkdownDocumentAsset class exists"), DocClass);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownSlateEditorFactoryTest, "MarkdownSlate.Editor.FactoryExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownSlateEditorFactoryTest::RunTest(const FString& Parameters)
{
	UClass* FactoryClass = LoadClass<UFactory>(nullptr,
		TEXT("/Script/MarkdownSlateEditor.MarkdownDocumentAssetFactory"));
	TestNotNull(TEXT("MarkdownDocumentAssetFactory class exists"), FactoryClass);

	UClass* ThemeFactoryClass = LoadClass<UFactory>(nullptr,
		TEXT("/Script/MarkdownSlateEditor.MarkdownThemeAssetFactory"));
	TestNotNull(TEXT("MarkdownThemeAssetFactory class exists"), ThemeFactoryClass);
	return true;
}

#endif
