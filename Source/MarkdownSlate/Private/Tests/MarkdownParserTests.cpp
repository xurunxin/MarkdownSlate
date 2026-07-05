#include "Misc/AutomationTest.h"
#include "Emoji/MarkdownEmojiScanner.h"
#include "Parser/MarkdownParser.h"
#include "Render/MarkdownRenderBuilder.h"
#include "Streaming/MarkdownStreamingBuffer.h"
#include "Widgets/MarkdownWidget.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownStreamingBufferStableBoundaryTest, "MarkdownSlate.StreamingBuffer.StableBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownStreamingBufferStableBoundaryTest::RunTest(const FString& Parameters)
{
	FMarkdownStreamingBuffer Buffer;

	Buffer.Append(TEXT("First line"));
	TestEqual(TEXT("No newline remains pending"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Pending holds incomplete line"), Buffer.GetPendingText(), FString(TEXT("First line")));

	Buffer.Append(TEXT("\nSecond"));
	TestEqual(TEXT("Newline commits stable text"), Buffer.GetStableText(), FString(TEXT("First line\n")));
	TestEqual(TEXT("Pending holds suffix"), Buffer.GetPendingText(), FString(TEXT("Second")));

	Buffer.Reset();
	TestEqual(TEXT("Reset clears stable text"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Reset clears pending text"), Buffer.GetPendingText(), FString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownWidgetStreamingApiTest, "MarkdownSlate.Widget.StreamingApi", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownWidgetStreamingApiTest::RunTest(const FString& Parameters)
{
	UMarkdownWidget* Widget = NewObject<UMarkdownWidget>();
	TestNotNull(TEXT("Widget"), Widget);

	Widget->BeginStreamingMarkdown();
	Widget->AppendMarkdownChunk(TEXT("Hello "));
	Widget->AppendMarkdownChunk(TEXT("world\n"));
	Widget->EndStreamingMarkdown();

	TestEqual(TEXT("Streaming preserves public markdown text"), Widget->MarkdownText, FString(TEXT("Hello world\n")));
	return true;
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserTaskListMetadataTest, "MarkdownSlate.Parser.TaskListMetadata", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserTaskListMetadataTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse("- [ ] Todo\n- [x] Done");
	TestNotNull(TEXT("Root"), Root.Get());
	TestTrue(TEXT("Root has list"), Root->Children.Num() > 0);

	auto List = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("List type"), List->Type, EMarkdownBlockType::UnorderedList);
	TestEqual(TEXT("List has two items"), List->Children.Num(), 2);

	auto Todo = StaticCastSharedPtr<FMarkdownBlockNode>(List->Children[0]);
	auto Done = StaticCastSharedPtr<FMarkdownBlockNode>(List->Children[1]);

	TestTrue(TEXT("Todo is task item"), Todo->bIsTaskItem);
	TestEqual(TEXT("Todo task mark"), Todo->TaskMark, TCHAR(' '));
	TestTrue(TEXT("Done is task item"), Done->bIsTaskItem);
	TestEqual(TEXT("Done task mark"), Done->TaskMark, TCHAR('x'));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserHeadingStrongWithoutTrailingNewlineTest, "MarkdownSlate.Parser.HeadingStrongWithoutTrailingNewline", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserHeadingStrongWithoutTrailingNewlineTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	auto Root = Parser.Parse(TEXT("## **Completed Scope**"));
	TestNotNull(TEXT("Root"), Root.Get());
	TestTrue(TEXT("Root has heading"), Root->Children.Num() > 0);

	auto Heading = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("Heading type"), Heading->Type, EMarkdownBlockType::Heading);
	TestEqual(TEXT("Heading level"), Heading->HeadingLevel, 2);

	bool bFoundStrong = false;
	for (const auto& Child : Heading->Children)
	{
		if (Child->IsSpanNode())
		{
			auto Span = StaticCastSharedPtr<FMarkdownSpanNode>(Child);
			if (Span->Type == EMarkdownSpanType::Strong)
			{
				bFoundStrong = Span->Children.Num() > 0;
				break;
			}
		}
	}

	TestTrue(TEXT("Heading strong span is parsed"), bFoundStrong);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserTableAfterInlineHeadingTest, "MarkdownSlate.Parser.TableAfterInlineHeading", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserTableAfterInlineHeadingTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	const FString Markdown = TEXT("# Report\n")
		TEXT("Done\n")
		TEXT("## Completed **Scope**\n")
		TEXT("| # | Module | Status |\n")
		TEXT("|---|---|---|\n")
		TEXT("| 1 | Cache | done |");

	auto Root = Parser.Parse(Markdown);
	TestNotNull(TEXT("Root"), Root.Get());
	TestEqual(TEXT("Root block count"), Root->Children.Num(), 4);

	auto Heading = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[2]);
	TestEqual(TEXT("Third block is heading"), Heading->Type, EMarkdownBlockType::Heading);
	TestEqual(TEXT("Heading level"), Heading->HeadingLevel, 2);

	auto Table = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[3]);
	TestEqual(TEXT("Fourth block is table"), Table->Type, EMarkdownBlockType::Table);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownParserTableAfterParagraphWithoutBlankLineTest, "MarkdownSlate.Parser.TableAfterParagraphWithoutBlankLine", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownParserTableAfterParagraphWithoutBlankLineTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;

	const FString Markdown = TEXT("Summary line\n")
		TEXT("| # | Module | Status |\n")
		TEXT("|---|---|---|\n")
		TEXT("| 1 | Cache | done |");

	auto Root = Parser.Parse(Markdown);
	TestNotNull(TEXT("Root"), Root.Get());
	TestEqual(TEXT("Root block count"), Root->Children.Num(), 2);
	if (Root->Children.Num() < 2)
	{
		return false;
	}

	auto Paragraph = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[0]);
	TestEqual(TEXT("First block is paragraph"), Paragraph->Type, EMarkdownBlockType::Paragraph);

	auto Table = StaticCastSharedPtr<FMarkdownBlockNode>(Root->Children[1]);
	TestEqual(TEXT("Second block is table"), Table->Type, EMarkdownBlockType::Table);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiSafeTextFallbackTest, "MarkdownSlate.Emoji.SafeTextFallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiSafeTextFallbackTest::RunTest(const FString& Parameters)
{
	FString Text;
	Text.AppendChar(TCHAR(0x2764));
	Text.AppendChar(TCHAR(0xFE0F));
	Text += TEXT(" ");
	Text.AppendChar(TCHAR(0xD83E));
	Text.AppendChar(TCHAR(0xDE7A));

	const FString Fallback = FMarkdownEmojiScanner::MakeSafeTextFallback(Text);

	TestTrue(TEXT("Fallback keeps BMP symbol text"), Fallback.Contains(FString::Chr(TCHAR(0x2764))));
	TestFalse(TEXT("Fallback removes variation selector"), Fallback.Contains(FString::Chr(TCHAR(0xFE0F))));
	TestFalse(TEXT("Fallback removes high surrogate"), Fallback.Contains(FString::Chr(TCHAR(0xD83E))));
	TestFalse(TEXT("Fallback removes low surrogate"), Fallback.Contains(FString::Chr(TCHAR(0xDE7A))));
	TestTrue(TEXT("Fallback replaces non-BMP emoji"), Fallback.Contains(TEXT("[emoji]")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiTwemojiCodeTest, "MarkdownSlate.Emoji.TwemojiCode", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiTwemojiCodeTest::RunTest(const FString& Parameters)
{
	FString Heart;
	Heart.AppendChar(TCHAR(0x2764));
	Heart.AppendChar(TCHAR(0xFE0F));
	TestEqual(TEXT("Heart keeps variation selector"), FMarkdownEmojiScanner::EmojiToTwemojiCode(Heart), TEXT("2764-fe0f"));

	FString Grinning;
	Grinning.AppendChar(TCHAR(0xD83D));
	Grinning.AppendChar(TCHAR(0xDE00));
	TestEqual(TEXT("Grinning face code"), FMarkdownEmojiScanner::EmojiToTwemojiCode(Grinning), TEXT("1f600"));

	FString Family;
	Family.AppendChar(TCHAR(0xD83D)); Family.AppendChar(TCHAR(0xDC68));
	Family.AppendChar(TCHAR(0x200D));
	Family.AppendChar(TCHAR(0xD83D)); Family.AppendChar(TCHAR(0xDC69));
	Family.AppendChar(TCHAR(0x200D));
	Family.AppendChar(TCHAR(0xD83D)); Family.AppendChar(TCHAR(0xDC67));
	Family.AppendChar(TCHAR(0x200D));
	Family.AppendChar(TCHAR(0xD83D)); Family.AppendChar(TCHAR(0xDC66));
	TestEqual(TEXT("Family ZWJ sequence code"), FMarkdownEmojiScanner::EmojiToTwemojiCode(Family), TEXT("1f468-200d-1f469-200d-1f467-200d-1f466"));

	FString ThumbsUpMedium;
	ThumbsUpMedium.AppendChar(TCHAR(0xD83D)); ThumbsUpMedium.AppendChar(TCHAR(0xDC4D));
	ThumbsUpMedium.AppendChar(TCHAR(0xD83C)); ThumbsUpMedium.AppendChar(TCHAR(0xDFFD));
	TestEqual(TEXT("Thumbs up skin tone code"), FMarkdownEmojiScanner::EmojiToTwemojiCode(ThumbsUpMedium), TEXT("1f44d-1f3fd"));

	FString Hospital;
	Hospital.AppendChar(TCHAR(0xD83C)); Hospital.AppendChar(TCHAR(0xDFE5));
	TestEqual(TEXT("Hospital code"), FMarkdownEmojiScanner::EmojiToTwemojiCode(Hospital), TEXT("1f3e5"));

	FString Stethoscope;
	Stethoscope.AppendChar(TCHAR(0xD83E)); Stethoscope.AppendChar(TCHAR(0xDE7A));
	TestEqual(TEXT("Stethoscope code"), FMarkdownEmojiScanner::EmojiToTwemojiCode(Stethoscope), TEXT("1fa7a"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownRenderBuilderNestedInlineTest, "MarkdownSlate.RenderBuilder.NestedInline", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownRenderBuilderNestedInlineTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;
	FMarkdownRenderBuilder Builder;

	auto Root = Parser.Parse(TEXT("混排：**重要 ❤️** [链接 😀](https://example.com)"));
	auto RenderRoot = Builder.Build(Root);
	TestNotNull(TEXT("Render root"), RenderRoot.Get());
	TestTrue(TEXT("Render root has paragraph"), RenderRoot->Children.Num() > 0);

	auto Paragraph = RenderRoot->Children[0];
	TestEqual(TEXT("Paragraph type"), Paragraph->Type, EMarkdownRenderNodeType::Paragraph);

	bool bFoundStrongWithText = false;
	bool bFoundLinkWithText = false;
	for (const auto& Child : Paragraph->Children)
	{
		if (Child->Type == EMarkdownRenderNodeType::Strong)
		{
			bFoundStrongWithText = Child->Children.Num() > 0 &&
				Child->Children[0]->Type == EMarkdownRenderNodeType::PlainText &&
				Child->Children[0]->TextContent.ToString().Contains(TEXT("重要"));
		}
		else if (Child->Type == EMarkdownRenderNodeType::Link)
		{
			bFoundLinkWithText = Child->LinkUrl == TEXT("https://example.com") &&
				Child->Children.Num() > 0 &&
				Child->Children[0]->Type == EMarkdownRenderNodeType::PlainText &&
				Child->Children[0]->TextContent.ToString().Contains(TEXT("链接"));
		}
	}

	TestTrue(TEXT("Strong inline preserves child text"), bFoundStrongWithText);
	TestTrue(TEXT("Link inline preserves child text"), bFoundLinkWithText);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
