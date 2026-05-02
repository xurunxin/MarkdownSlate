#include "Misc/AutomationTest.h"
#include "Parser/MarkdownParser.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserHeadingTest, "MarkdownSlate.Parser.Heading", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserHeadingTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	{
		auto Root = Parser.Parse("# Heading 1");
		TestNotNull(TEXT("Root node"), Root.Get());
		TestEqual(TEXT("Document type"), Root->Type, EMarkdownBlockType::Document);
		TestEqual(TEXT("Doc has one child"), Root->Children.Num() > 0, true);

		bool bFoundHeading = false;
		for (const auto& C : Root->Children)
		{
			if (C->IsBlockNode())
			{
				auto H = StaticCastSharedPtr<FMarkdownBlockNode>(C);
				if (H->Type == EMarkdownBlockType::Heading)
				{
					bFoundHeading = true;
					TestEqual(TEXT("H1 level"), H->HeadingLevel, 1);
					break;
				}
			}
		}
		TestTrue(TEXT("Found heading"), bFoundHeading);
	}

	{
		auto Root = Parser.Parse("## Heading 2");
		auto H2 = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
		TestEqual(TEXT("H2 level"), H2->HeadingLevel, 2);
	}

	{
		auto Root = Parser.Parse("###### Heading 6");
		auto H6 = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
		TestEqual(TEXT("H6 level"), H6->HeadingLevel, 6);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserParagraphTest, "MarkdownSlate.Parser.Paragraph", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserParagraphTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("This is a paragraph.");
	TestNotNull(TEXT("Root"), Root.Get());

	bool bFound = false;
	for (const auto& C : Root->Children)
	{
		if (C->IsBlockNode())
		{
			auto B = StaticCastSharedPtr<FMarkdownBlockNode>(C);
			if (B->Type == EMarkdownBlockType::Paragraph)
			{
				bFound = true;
				break;
			}
		}
	}
	TestTrue(TEXT("Found paragraph"), bFound);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserStrongTest, "MarkdownSlate.Parser.Strong", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserStrongTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("This is **bold** text.");
	auto Para = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("Para children count"), Para->Children.Num(), 3);

	bool bFoundStrong = false;
	for (const auto& Child : Para->Children)
	{
		if (Child->IsSpanNode())
		{
			auto Span = StaticCastSharedPtr<FMarkdownSpanNode>(Child);
			if (Span->Type == EMarkdownSpanType::Strong)
			{
				bFoundStrong = true;
				break;
			}
		}
	}
	TestTrue(TEXT("Found strong span"), bFoundStrong);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserEmphasisTest, "MarkdownSlate.Parser.Emphasis", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserEmphasisTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("This is *italic* text.");
	auto Para = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);

	bool bFoundEmphasis = false;
	for (const auto& Child : Para->Children)
	{
		if (Child->IsSpanNode())
		{
			auto Span = StaticCastSharedPtr<FMarkdownSpanNode>(Child);
			if (Span->Type == EMarkdownSpanType::Emphasis)
			{
				bFoundEmphasis = true;
				break;
			}
		}
	}
	TestTrue(TEXT("Found emphasis span"), bFoundEmphasis);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserUnorderedListTest, "MarkdownSlate.Parser.UnorderedList", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserUnorderedListTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("- Item 1\n- Item 2\n- Item 3");

	auto UL = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("UL type"), UL->Type, EMarkdownBlockType::UnorderedList);
	TestEqual(TEXT("UL has 3 items"), UL->Children.Num(), 3);

	for (int32 i = 0; i < UL->Children.Num(); ++i)
	{
		auto LI = StaticCastSharedPtr<FMarkdownBlockNode>(UL->Children[i]);
		TestEqual(FString::Printf(TEXT("Child %d is list item"), i), LI->Type, EMarkdownBlockType::ListItem);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserOrderedListTest, "MarkdownSlate.Parser.OrderedList", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserOrderedListTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("1. First\n2. Second\n3. Third");
	auto OL = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("OL type"), OL->Type, EMarkdownBlockType::OrderedList);
	TestEqual(TEXT("OL has 3 items"), OL->Children.Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserFencedCodeBlockTest, "MarkdownSlate.Parser.CodeBlock", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserFencedCodeBlockTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("```cpp\nint main() { return 0; }\n```");
	auto Code = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("Code type"), Code->Type, EMarkdownBlockType::CodeBlock);
	TestEqual(TEXT("Code lang"), Code->CodeLanguage, TEXT("cpp"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserLinkTest, "MarkdownSlate.Parser.Link", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserLinkTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("[click here](https://example.com)");
	auto Para = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);

	bool bFoundLink = false;
	for (const auto& Child : Para->Children)
	{
		if (Child->IsSpanNode())
		{
			auto Span = StaticCastSharedPtr<FMarkdownSpanNode>(Child);
			if (Span->Type == EMarkdownSpanType::Link)
			{
				bFoundLink = true;
				TestEqual(TEXT("Link href"), Span->Href, TEXT("https://example.com"));
				break;
			}
		}
	}
	TestTrue(TEXT("Found link span"), bFoundLink);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserImageTest, "MarkdownSlate.Parser.Image", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserImageTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("![alt text](https://example.com/img.png)");
	auto Para = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);

	bool bFoundImage = false;
	for (const auto& Child : Para->Children)
	{
		if (Child->IsSpanNode())
		{
			auto Span = StaticCastSharedPtr<FMarkdownSpanNode>(Child);
			if (Span->Type == EMarkdownSpanType::Image)
			{
				bFoundImage = true;
				TestEqual(TEXT("Image src"), Span->ImageSrc, TEXT("https://example.com/img.png"));
				break;
			}
		}
	}
	TestTrue(TEXT("Found image span"), bFoundImage);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
