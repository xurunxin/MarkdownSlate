#include "Misc/AutomationTest.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "Emoji/MarkdownEmojiScanner.h"
#include "Emoji/SMarkdownEmojiRun.h"
#include "Parser/MarkdownParser.h"
#include "Render/MarkdownRenderBuilder.h"
#include "Slate/MarkdownSlateRenderer.h"
#include "Slate/SMarkdownView.h"
#include "Streaming/MarkdownStreamingBuffer.h"
#include "Widgets/MarkdownStreamingPerfTestWidget.h"
#include "Widgets/MarkdownWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Tests/AutomationCommon.h"

#if WITH_AUTOMATION_TESTS

static UWorld* FindAutomationWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World())
		{
			return Context.World();
		}
	}

	return nullptr;
}

static UMarkdownStreamingPerfTestWidget* CreateMarkdownPerfTestWidget()
{
	UClass* WidgetClass = LoadClass<UMarkdownStreamingPerfTestWidget>(nullptr, TEXT("/Game/MarkdownSlate/Tests/WBP_MarkdownStreamingPerf_MCP.WBP_MarkdownStreamingPerf_MCP_C"));
	UWorld* World = FindAutomationWorld();
	if (!WidgetClass || !World)
	{
		return nullptr;
	}

	return CreateWidget<UMarkdownStreamingPerfTestWidget>(World, WidgetClass);
}

class FWaitForMarkdownDelayedStreamingCommand final : public IAutomationLatentCommand
{
public:
	FWaitForMarkdownDelayedStreamingCommand(FAutomationTestBase* InTest, UMarkdownStreamingPerfTestWidget* InWidget, int32 InExpectedChars, double InStartedAt)
		: Test(InTest)
		, Widget(InWidget)
		, ExpectedChars(InExpectedChars)
		, StartedAt(InStartedAt)
	{
	}

	virtual bool Update() override
	{
		if (!Test)
		{
			return true;
		}

		if (!Widget.IsValid())
		{
			Test->AddError(TEXT("Delayed streaming widget became invalid"));
			return true;
		}

		if (!Widget->IsDelayedTokenStreamingActive())
		{
			const FMarkdownStreamingPerfResult Result = Widget->GetLastDelayedStreamingResult();
			Test->TestTrue(TEXT("Delayed token streaming completed successfully"), Result.bSuccess);
			Test->TestEqual(TEXT("Delayed token streaming produced expected character count"), Result.TotalChars, ExpectedChars);
			Test->TestTrue(TEXT("Delayed token streaming elapsed across timer ticks"), Result.ElapsedMs >= 20.0);
			return true;
		}

		if ((FPlatformTime::Seconds() - StartedAt) > 5.0)
		{
			Test->AddError(TEXT("Timed out waiting for delayed token streaming"));
			Widget->StopDelayedTokenStreamingTest();
			return true;
		}

		return false;
	}

private:
	FAutomationTestBase* Test = nullptr;
	TWeakObjectPtr<UMarkdownStreamingPerfTestWidget> Widget;
	int32 ExpectedChars = 0;
	double StartedAt = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownStreamingBufferStableBoundaryTest, "MarkdownSlate.StreamingBuffer.StableBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownStreamingBufferStableBoundaryTest::RunTest(const FString& Parameters)
{
	FMarkdownStreamingBuffer Buffer;

	Buffer.Append(TEXT("First line"));
	TestEqual(TEXT("No newline remains pending"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Pending holds incomplete line"), Buffer.GetPendingText(), FString(TEXT("First line")));

	Buffer.Append(TEXT("\nSecond"));
	TestEqual(TEXT("Single newline keeps current block pending"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Pending holds current block"), Buffer.GetPendingText(), FString(TEXT("First line\nSecond")));

	Buffer.Append(TEXT("\n\nThird"));
	TestEqual(TEXT("Blank line commits stable block"), Buffer.GetStableText(), FString(TEXT("First line\nSecond\n\n")));
	TestEqual(TEXT("Pending holds suffix after blank line"), Buffer.GetPendingText(), FString(TEXT("Third")));

	Buffer.Reset();
	TestEqual(TEXT("Reset clears stable text"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Reset clears pending text"), Buffer.GetPendingText(), FString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownStreamingBufferTableBoundaryTest, "MarkdownSlate.StreamingBuffer.TableBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownStreamingBufferTableBoundaryTest::RunTest(const FString& Parameters)
{
	FMarkdownStreamingBuffer Buffer;

	Buffer.Append(TEXT("| Item | Value |\n"));
	TestEqual(TEXT("Table header remains pending"), Buffer.GetStableText(), FString());
	TestEqual(TEXT("Pending has table header"), Buffer.GetPendingText(), FString(TEXT("| Item | Value |\n")));

	Buffer.Append(TEXT("| --- | ---: |\n"));
	TestEqual(TEXT("Table delimiter remains pending"), Buffer.GetStableText(), FString());

	Buffer.Append(TEXT("| Row | 1 |\n"));
	TestEqual(TEXT("Table row remains pending until block boundary"), Buffer.GetStableText(), FString());

	Buffer.Append(TEXT("\nNext"));
	TestEqual(TEXT("Blank line commits whole table"), Buffer.GetStableText(), FString(TEXT("| Item | Value |\n| --- | ---: |\n| Row | 1 |\n\n")));
	TestEqual(TEXT("Pending starts next block"), Buffer.GetPendingText(), FString(TEXT("Next")));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownStreamingPendingInlineFallbackTest, "MarkdownSlate.Streaming.PendingInlineFallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownStreamingPendingInlineFallbackTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Unclosed strong text renders pending as plain text"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("**diagnosis")));
	TestTrue(TEXT("Closed single-line strong text remains plain while pending"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("**diagnosis**")));
	TestTrue(TEXT("Unclosed code span renders pending as plain text"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("`code")));
	TestTrue(TEXT("Closed single-line code span remains plain while pending"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("`code`")));
	TestTrue(TEXT("Unclosed link renders pending as plain text"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("[reference](https://example.com")));
	TestTrue(TEXT("Single-line heading remains plain while pending"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("# Assessment")));
	TestTrue(TEXT("Single-line unordered list remains plain while pending"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("- item")));
	TestTrue(TEXT("Single-line ordered list remains plain while pending"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("1. item")));
	TestTrue(TEXT("Multi-line unordered list remains plain until block boundary"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("- first\n- second")));
	TestTrue(TEXT("Multi-line ordered list remains plain until block boundary"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("1. first\n2. second")));
	TestTrue(TEXT("Task list remains plain until block boundary"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("- [ ] first\n- [x] second")));
	TestFalse(TEXT("Table pending remains markdown-renderable"), MarkdownSlate::ShouldRenderPendingStreamingTextAsPlainText(TEXT("| A | B |\n| --- | --- |\n")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownWidgetStreamingBlueprintPerfTest, "MarkdownSlate.Widget.StreamingBlueprintPerf", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownWidgetStreamingBlueprintPerfTest::RunTest(const FString& Parameters)
{
	UClass* WidgetClass = LoadClass<UMarkdownStreamingPerfTestWidget>(nullptr, TEXT("/Game/MarkdownSlate/Tests/WBP_MarkdownStreamingPerf_MCP.WBP_MarkdownStreamingPerf_MCP_C"));
	TestNotNull(TEXT("MCP-created performance widget blueprint class"), WidgetClass);
	if (!WidgetClass)
	{
		return false;
	}

	UWorld* World = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World())
			{
				World = Context.World();
				break;
			}
		}
	}

	TestNotNull(TEXT("World for creating performance widget"), World);
	if (!World)
	{
		return false;
	}

	UMarkdownStreamingPerfTestWidget* Widget = CreateWidget<UMarkdownStreamingPerfTestWidget>(World, WidgetClass);
	TestNotNull(TEXT("Performance widget instance"), Widget);
	if (!Widget)
	{
		return false;
	}

	const FMarkdownStreamingPerfResult Streaming = Widget->RunStreamingPerformanceTest(120, 192, true);
	const FMarkdownStreamingPerfResult FullReset = Widget->RunStreamingPerformanceTest(120, 192, false);

	TestTrue(TEXT("Streaming blueprint path succeeds"), Streaming.bSuccess);
	TestTrue(TEXT("Full-reset comparison path succeeds"), FullReset.bSuccess);
	TestTrue(TEXT("Streaming remains faster than full reset for chatbot-style chunks"), Streaming.ElapsedMs <= FullReset.ElapsedMs);

	AddInfo(FString::Printf(TEXT("Streaming perf: chars=%d chunks=%d streaming=%.3fms avg=%.3fms full-reset=%.3fms"),
		Streaming.TotalChars,
		Streaming.ChunkCount,
		Streaming.ElapsedMs,
		Streaming.AverageChunkMs,
		FullReset.ElapsedMs));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownWidgetDelayedTokenStreamingTest, "MarkdownSlate.Widget.DelayedTokenStreaming", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownWidgetDelayedTokenStreamingTest::RunTest(const FString& Parameters)
{
	UMarkdownStreamingPerfTestWidget* Widget = CreateMarkdownPerfTestWidget();
	TestNotNull(TEXT("Performance widget instance"), Widget);
	if (!Widget)
	{
		return false;
	}

	const int32 ExpectedChars = Widget->GetPerfMarkdownText(2).Len();
	const FMarkdownStreamingPerfResult Started = Widget->StartDelayedTokenStreamingTest(2, 64, 0.01f, false);
	TestTrue(TEXT("Delayed token streaming starts"), Started.bSuccess);
	TestTrue(TEXT("Delayed token streaming reports active state"), Widget->IsDelayedTokenStreamingActive());
	TestEqual(TEXT("Delayed token streaming uses requested token size"), Started.ChunkCount, FMath::DivideAndRoundUp(ExpectedChars, 64));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForMarkdownDelayedStreamingCommand(this, Widget, ExpectedChars, FPlatformTime::Seconds()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownWidgetEmojiMixedBlueprintTest, "MarkdownSlate.Widget.EmojiMixedBlueprint", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownWidgetEmojiMixedBlueprintTest::RunTest(const FString& Parameters)
{
	UMarkdownStreamingPerfTestWidget* Widget = CreateMarkdownPerfTestWidget();
	TestNotNull(TEXT("Performance widget instance"), Widget);
	if (!Widget)
	{
		return false;
	}

	const FString EmojiMarkdown = Widget->GetEmojiMixedMarkdownText();
	TestTrue(TEXT("Emoji mixed markdown includes face emoji"), EmojiMarkdown.Contains(TEXT("😀")));
	TestTrue(TEXT("Emoji mixed markdown includes ZWJ medical worker emoji"), EmojiMarkdown.Contains(TEXT("🧑‍⚕️")));
	TestTrue(TEXT("Emoji mixed markdown includes flag emoji"), EmojiMarkdown.Contains(TEXT("🇨🇳")));
	TestTrue(TEXT("Emoji mixed markdown includes skin tone emoji"), EmojiMarkdown.Contains(TEXT("👍🏽")));

	const FMarkdownStreamingPerfResult Result = Widget->ShowEmojiMixedTextTest(true, 24);
	TestTrue(TEXT("Emoji mixed blueprint display succeeds"), Result.bSuccess);
	TestEqual(TEXT("Emoji mixed blueprint display preserves text length"), Result.TotalChars, EmojiMarkdown.Len());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiDefaultFontTest, "MarkdownSlate.Emoji.DefaultFont", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiDefaultFontTest::RunTest(const FString& Parameters)
{
	const FMarkdownSlateThemeConfig Config = FMarkdownSlateThemeConfig::Default();
	TestNotNull(TEXT("Default emoji font object is loaded"), Config.EmojiFont.FontObject.Get());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiDefaultAtlasFirstTest, "MarkdownSlate.Emoji.DefaultAtlasFirst", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiDefaultAtlasFirstTest::RunTest(const FString& Parameters)
{
	const FMarkdownSlateThemeConfig Config = FMarkdownSlateThemeConfig::Default();
	TestEqual(TEXT("Default emoji rendering prefers atlas over platform font"), Config.EmojiRenderMode, EMarkdownEmojiRenderMode::TwemojiFirst);

	FMarkdownEmojiAtlas Atlas;
	TestTrue(TEXT("Built-in emoji atlas loads"), Atlas.AutoLoadAtlas(Config.TwemojiAssetRoot));
	TestTrue(TEXT("Atlas has grinning face brush"), Atlas.GetEmojiBrush(TEXT("1f600"), 18.0f).IsValid());
	TestTrue(TEXT("Atlas resolves heart variation brush"), Atlas.GetEmojiBrush(TEXT("2764-fe0f"), 18.0f).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiPlatformFontFirstTest, "MarkdownSlate.Emoji.PlatformFontFirst", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiPlatformFontFirstTest::RunTest(const FString& Parameters)
{
	FMarkdownSlateThemeConfig ThemeConfig = FMarkdownSlateThemeConfig::Default();
	FMarkdownAtlasEmojiProvider Provider;
	TestTrue(TEXT("Built-in emoji provider loads atlas"), Provider.GetAtlasPtr()->AutoLoadAtlas(ThemeConfig.TwemojiAssetRoot));

	FMarkdownEmojiRun Run;
	Run.EmojiSequence = TEXT("馃榾");
	Run.TwemojiCode = TEXT("1f600");
	Run.bIsEmoji = true;

	FMarkdownEmojiConfig Config;
	Config.RenderMode = EMarkdownEmojiRenderMode::PlatformFontFirst;
	Config.bAllowTwemojiFallback = true;
	Config.EmojiSizeScale = ThemeConfig.EmojiSizeScale;

	const TSharedRef<SMarkdownEmojiRun> Widget = SNew(SMarkdownEmojiRun)
		.Run(Run)
		.FontSize(24)
		.FontInfo(ThemeConfig.DefaultFont)
		.EmojiFontInfo(ThemeConfig.EmojiFont)
		.TextColor(FLinearColor::White)
		.EmojiProvider(&Provider)
		.Config(Config);

	const FChildren* Children = Widget->GetChildren();
	TestEqual(TEXT("Emoji run has one child"), Children->Num(), 1);
	if (Children->Num() > 0)
	{
        TestEqual(TEXT("Platform font mode keeps the configured emoji font"), Children->GetChildAt(0)->GetTypeAsString(), FString(TEXT("STextBlock")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownEmojiAtlasUVInsetTest, "MarkdownSlate.Emoji.AtlasUVInset", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownEmojiAtlasUVInsetTest::RunTest(const FString& Parameters)
{
	FMarkdownEmojiAtlas Atlas;
	Atlas.SetAtlasConfig(72, 4096);
	Atlas.AddEntry(TEXT("1f004"), 0, 0, 0);

	FVector2D UVMin;
	FVector2D UVMax;
	TestTrue(TEXT("Known first-cell emoji has UVs"), Atlas.GetUVRect(TEXT("1f004"), UVMin, UVMax));
	TestTrue(TEXT("UV min is inset from cell edge"), UVMin.X > 0.0f || UVMin.Y > 0.0f);
	TestTrue(TEXT("UV max remains within atlas"), UVMax.X < 1.0f && UVMax.Y < 1.0f);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMarkdownPlainInlineLayoutWrapTest,
	"MarkdownSlate.Renderer.PlainInlineLayoutWrap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::NonNullRHI)

bool FMarkdownPlainInlineLayoutWrapTest::RunTest(const FString& Parameters)
{
	const FString Markdown = TEXT("A long ordinary clinical explanation should wrap through Slate layout while preserving every original character and without inserting hard line breaks into the Markdown model.");
	TestFalse(TEXT("Plain Markdown control contains no source newline"), Markdown.Contains(TEXT("\n")));

	FMarkdownParser Parser;
	FMarkdownRenderBuilder Builder;
	const TSharedPtr<FMarkdownBlockNode> AstRoot = Parser.Parse(Markdown);
	const TSharedPtr<FMarkdownRenderNode> RenderRoot = Builder.Build(AstRoot);
	if (!TestNotNull(TEXT("Plain Markdown render root builds"), RenderRoot.Get()) ||
		!TestTrue(TEXT("Plain Markdown render root contains a paragraph"), RenderRoot->Children.Num() > 0))
	{
		return false;
	}

	FMarkdownSlateThemeConfig Theme = FMarkdownSlateThemeConfig::Default();
	Theme.WrapTextWidth = 160.0f;
	const TSharedRef<SWidget> ParagraphWidget = FMarkdownSlateRenderer::RenderNode(RenderRoot->Children[0], Theme);
	FWidgetRenderer Renderer(false, true);
	ParagraphWidget->SlatePrepass(1.0f);
	UTextureRenderTarget2D* RenderTarget = Renderer.DrawWidget(ParagraphWidget, FVector2D(180.0f, 400.0f));
	ParagraphWidget->SlatePrepass(1.0f);

	TestNotNull(TEXT("Plain inline wrap render target creates"), RenderTarget);
	const FVector2D DesiredSize = ParagraphWidget->GetDesiredSize();
	const FVector2D CachedSize = ParagraphWidget->GetTickSpaceGeometry().GetLocalSize();
	AddInfo(FString::Printf(TEXT("Plain inline wrap geometry desired=(%.2f,%.2f) cached=(%.2f,%.2f)"), DesiredSize.X, DesiredSize.Y, CachedSize.X, CachedSize.Y));
	TestTrue(TEXT("Plain inline desired width respects the positive theme wrap width"), DesiredSize.X <= Theme.WrapTextWidth + 1.0f);
	TestTrue(TEXT("Plain inline cached width stays inside the render allocation"), CachedSize.X <= 180.0f + 1.0f);
	TestTrue(TEXT("Plain inline desired layout grows vertically after wrapping"), DesiredSize.Y >= 60.0f);
	TestFalse(TEXT("Renderer layout wrapping does not mutate source Markdown"), Markdown.Contains(TEXT("\n")));

	Theme.WrapTextWidth = 0.0f;
	const TSharedRef<SWidget> NonPositiveWrapWidget = FMarkdownSlateRenderer::RenderNode(RenderRoot->Children[0], Theme);
	NonPositiveWrapWidget->SlatePrepass(1.0f);
	const FVector2D NonPositiveDesiredSize = NonPositiveWrapWidget->GetDesiredSize();
	TestTrue(TEXT("Non-positive wrap width retains a safe positive desired width"), NonPositiveDesiredSize.X > 0.0f);
	TestTrue(TEXT("Non-positive wrap width retains a safe positive desired height"), NonPositiveDesiredSize.Y > 0.0f);
	TestFalse(TEXT("Non-positive wrap width also preserves the source Markdown"), Markdown.Contains(TEXT("\n")));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarkdownRendererInlineNoWrapTest, "MarkdownSlate.Renderer.InlineNoWrap", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)
bool FMarkdownRendererInlineNoWrapTest::RunTest(const FString& Parameters)
{
	FMarkdownParser Parser;
	FMarkdownRenderBuilder Builder;

	const auto Ast = Parser.Parse(TEXT("Before **bold** [link](https://example.com) ~~strike~~ `code` after"));
	const auto Root = Builder.Build(Ast);
	TestTrue(TEXT("Root contains one paragraph"), Root.IsValid() && Root->Children.Num() == 1);
	if (!Root.IsValid() || Root->Children.Num() != 1)
	{
		return false;
	}

	const TSharedRef<SWidget> Rendered = FMarkdownSlateRenderer::RenderNode(Root->Children[0], FMarkdownSlateThemeConfig::Default());
	TestEqual(TEXT("Inline fragments use a non-wrapping horizontal layout"), Rendered->GetTypeAsString(), FString(TEXT("SHorizontalBox")));
	return true;
}

#endif // WITH_AUTOMATION_TESTS
