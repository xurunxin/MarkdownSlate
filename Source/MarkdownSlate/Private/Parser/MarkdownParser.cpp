#include "Parser/MarkdownParser.h"
#include "md4c.h"
#include "Md4cWrapper.h"

static FString ConvertUTF8ToString(const MD_CHAR* Text, MD_SIZE Size)
{
	if (!Text || Size == 0) return FString();
	FUTF8ToTCHAR Conv(Text, Size);
	return FString(Conv.Length(), Conv.Get());
}

struct FParsingState
{
	TSharedPtr<FMarkdownBlockNode> RootNode;
	TArray<TSharedPtr<FMarkdownAstNode>> NodeStack;
	TArray<FString> TextBufferStack;
	bool bAborted = false;
};

static TSharedPtr<FMarkdownBlockNode> GetCurrentBlock(FParsingState* State)
{
	for (int32 i = State->NodeStack.Num() - 1; i >= 0; --i)
	{
		if (State->NodeStack[i]->IsBlockNode())
		{
			return StaticCastSharedPtr<FMarkdownBlockNode>(State->NodeStack[i]);
		}
	}
	return nullptr;
}

static TSharedPtr<FMarkdownSpanNode> GetCurrentSpan(FParsingState* State)
{
	if (State->NodeStack.Num() > 0)
	{
		auto Top = State->NodeStack.Last();
		if (Top->IsSpanNode())
		{
			return StaticCastSharedPtr<FMarkdownSpanNode>(Top);
		}
	}
	return nullptr;
}

static FString& GetCurrentTextBuffer(FParsingState* State)
{
	if (State->TextBufferStack.Num() == 0)
	{
		State->TextBufferStack.Add(FString());
	}
	return State->TextBufferStack.Last();
}

static void FlushTextBuffer(FParsingState* State)
{
	FString& Buffer = GetCurrentTextBuffer(State);
	if (!Buffer.IsEmpty())
	{
		auto Current = State->NodeStack.Num() > 0 ? State->NodeStack.Last() : TSharedPtr<FMarkdownAstNode>();
		if (Current.IsValid())
		{
			auto TextNode = MakeShared<FMarkdownTextNode>();
			TextNode->Text = MoveTemp(Buffer);
			Current->Children.Add(TextNode);
		}
		Buffer.Empty();
	}
}

static int EnterBlock(MD_BLOCKTYPE Type, void* Detail, void* UserData)
{
	FParsingState* State = static_cast<FParsingState*>(UserData);
	if (!State || State->bAborted) return 0;

	FlushTextBuffer(State);

	auto Block = MakeShared<FMarkdownBlockNode>();

	switch (Type)
	{
	case MD_BLOCK_DOC:       Block->Type = EMarkdownBlockType::Document; break;
	case MD_BLOCK_QUOTE:     Block->Type = EMarkdownBlockType::Quote; break;
	case MD_BLOCK_UL:        Block->Type = EMarkdownBlockType::UnorderedList; break;
	case MD_BLOCK_OL:        Block->Type = EMarkdownBlockType::OrderedList; break;
	case MD_BLOCK_LI:        Block->Type = EMarkdownBlockType::ListItem; break;
	case MD_BLOCK_HR:        Block->Type = EMarkdownBlockType::HorizontalRule; break;
	case MD_BLOCK_H:         Block->Type = EMarkdownBlockType::Heading; break;
	case MD_BLOCK_CODE:      Block->Type = EMarkdownBlockType::CodeBlock; break;
	case MD_BLOCK_HTML:      Block->Type = EMarkdownBlockType::HtmlBlock; break;
	case MD_BLOCK_P:         Block->Type = EMarkdownBlockType::Paragraph; break;
	case MD_BLOCK_TABLE:     Block->Type = EMarkdownBlockType::Table; break;
	case MD_BLOCK_THEAD:     Block->Type = EMarkdownBlockType::TableHead; break;
	case MD_BLOCK_TBODY:     Block->Type = EMarkdownBlockType::TableBody; break;
	case MD_BLOCK_TR:        Block->Type = EMarkdownBlockType::TableRow; break;
	case MD_BLOCK_TH:        Block->Type = EMarkdownBlockType::TableHeader; break;
	case MD_BLOCK_TD:        Block->Type = EMarkdownBlockType::TableData; break;
	default: return 0;
	}

	if (Type == MD_BLOCK_UL && Detail)
	{
		auto* D = static_cast<MD_BLOCK_UL_DETAIL*>(Detail);
		Block->bIsTightList = D->is_tight != 0;
		Block->ListBulletChar = D->mark;
	}
	else if (Type == MD_BLOCK_OL && Detail)
	{
		auto* D = static_cast<MD_BLOCK_OL_DETAIL*>(Detail);
		Block->OrderedListStart = D->start;
		Block->bIsTightList = D->is_tight != 0;
		Block->OrderedListDelimiter = D->mark_delimiter;
	}
	else if (Type == MD_BLOCK_LI && Detail)
	{
		auto* D = static_cast<MD_BLOCK_LI_DETAIL*>(Detail);
		Block->bIsTaskItem = D->is_task != 0;
		Block->TaskMark = D->task_mark;
	}
	else if (Type == MD_BLOCK_H && Detail)
	{
		auto* D = static_cast<MD_BLOCK_H_DETAIL*>(Detail);
		Block->HeadingLevel = D->level;
	}
	else if (Type == MD_BLOCK_CODE && Detail)
	{
		auto* D = static_cast<MD_BLOCK_CODE_DETAIL*>(Detail);
		if (D->lang.text && D->lang.size > 0)
		{
			Block->CodeLanguage = ConvertUTF8ToString(D->lang.text, D->lang.size);
		}
	}

	auto Parent = State->NodeStack.Num() > 0 ? State->NodeStack.Last() : TSharedPtr<FMarkdownAstNode>();
	if (Parent.IsValid())
	{
		Parent->Children.Add(Block);
	}
	else
	{
		State->RootNode = Block;
	}

	State->NodeStack.Add(Block);
	State->TextBufferStack.Add(FString());

	return 0;
}

static int LeaveBlock(MD_BLOCKTYPE Type, void* Detail, void* UserData)
{
	FParsingState* State = static_cast<FParsingState*>(UserData);
	if (!State || State->bAborted) return 0;

	FlushTextBuffer(State);

	if (State->TextBufferStack.Num() > 0)
	{
		State->TextBufferStack.Pop();
	}

	if (State->NodeStack.Num() > 0)
	{
		State->NodeStack.Pop();
	}

	return 0;
}

static int EnterSpan(MD_SPANTYPE Type, void* Detail, void* UserData)
{
	FParsingState* State = static_cast<FParsingState*>(UserData);
	if (!State || State->bAborted) return 0;

	FlushTextBuffer(State);

	auto Span = MakeShared<FMarkdownSpanNode>();

	switch (Type)
	{
	case MD_SPAN_EM:              Span->Type = EMarkdownSpanType::Emphasis; break;
	case MD_SPAN_STRONG:          Span->Type = EMarkdownSpanType::Strong; break;
	case MD_SPAN_A:               Span->Type = EMarkdownSpanType::Link; break;
	case MD_SPAN_IMG:             Span->Type = EMarkdownSpanType::Image; break;
	case MD_SPAN_CODE:            Span->Type = EMarkdownSpanType::Code; break;
	case MD_SPAN_DEL:             Span->Type = EMarkdownSpanType::Strikethrough; break;
	case MD_SPAN_U:               Span->Type = EMarkdownSpanType::Underline; break;
	default: return 0;
	}

	if (Type == MD_SPAN_A && Detail)
	{
		auto* D = static_cast<MD_SPAN_A_DETAIL*>(Detail);
		if (D->href.text && D->href.size > 0)
		{
			Span->Href = ConvertUTF8ToString(D->href.text, D->href.size);
		}
		if (D->title.text && D->title.size > 0)
		{
			Span->Title = ConvertUTF8ToString(D->title.text, D->title.size);
		}
		Span->bIsAutolink = D->is_autolink != 0;
	}
	else if (Type == MD_SPAN_IMG && Detail)
	{
		auto* D = static_cast<MD_SPAN_IMG_DETAIL*>(Detail);
		if (D->src.text && D->src.size > 0)
		{
			Span->ImageSrc = ConvertUTF8ToString(D->src.text, D->src.size);
		}
		if (D->title.text && D->title.size > 0)
		{
			Span->ImageTitle = ConvertUTF8ToString(D->title.text, D->title.size);
		}
	}

	auto Parent = State->NodeStack.Num() > 0 ? State->NodeStack.Last() : TSharedPtr<FMarkdownAstNode>();
	if (Parent.IsValid())
	{
		Parent->Children.Add(Span);
	}

	State->NodeStack.Add(Span);
	State->TextBufferStack.Add(FString());

	return 0;
}

static int LeaveSpan(MD_SPANTYPE Type, void* Detail, void* UserData)
{
	FParsingState* State = static_cast<FParsingState*>(UserData);
	if (!State || State->bAborted) return 0;

	FlushTextBuffer(State);

	if (State->TextBufferStack.Num() > 0)
	{
		State->TextBufferStack.Pop();
	}

	if (State->NodeStack.Num() > 0)
	{
		State->NodeStack.Pop();
	}

	return 0;
}

static int TextCallback(MD_TEXTTYPE Type, const MD_CHAR* Text, MD_SIZE Size, void* UserData)
{
	FParsingState* State = static_cast<FParsingState*>(UserData);
	if (!State || State->bAborted) return 0;

	FUTF8ToTCHAR UTF8Converter(Text, Size);
	FString TextStr(UTF8Converter.Length(), UTF8Converter.Get());

	switch (Type)
	{
	case MD_TEXT_NORMAL:
	case MD_TEXT_NULLCHAR:
	case MD_TEXT_ENTITY:
	case MD_TEXT_CODE:
		GetCurrentTextBuffer(State) += TextStr;
		break;
	case MD_TEXT_BR:
	case MD_TEXT_SOFTBR:
		FlushTextBuffer(State);
		{
			auto LineBreakNode = MakeShared<FMarkdownTextNode>();
			LineBreakNode->Text = TEXT("\n");
			auto Current = State->NodeStack.Num() > 0 ? State->NodeStack.Last() : TSharedPtr<FMarkdownAstNode>();
			if (Current.IsValid())
			{
				Current->Children.Add(LineBreakNode);
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

FMarkdownParser::FMarkdownParser()
	: ParseFlags(MD_DIALECT_GITHUB)
{
}

TSharedPtr<FMarkdownBlockNode> FMarkdownParser::Parse(const FString& MarkdownText)
{
	if (MarkdownText.IsEmpty())
	{
		auto Doc = MakeShared<FMarkdownBlockNode>();
		Doc->Type = EMarkdownBlockType::Document;
		return Doc;
	}

	FTCHARToUTF8 Utf8(*MarkdownText);
	int32 Utf8Len = Utf8.Length();

	MD_PARSER ParserDef;
	FMemory::Memzero(&ParserDef, sizeof(ParserDef));

	ParserDef.abi_version = 0;
	ParserDef.flags = ParseFlags;
	ParserDef.enter_block = EnterBlock;
	ParserDef.leave_block = LeaveBlock;
	ParserDef.enter_span = EnterSpan;
	ParserDef.leave_span = LeaveSpan;
	ParserDef.text = TextCallback;
	ParserDef.debug_log = nullptr;
	ParserDef.syntax = nullptr;

	FParsingState State;
	int Ret = MarkdownParse(Utf8.Get(), Utf8Len, &ParserDef, &State);

	if (Ret != 0 && !State.RootNode.IsValid())
	{
		auto Doc = MakeShared<FMarkdownBlockNode>();
		Doc->Type = EMarkdownBlockType::Document;
		return Doc;
	}

	return State.RootNode.IsValid() ? State.RootNode : MakeShared<FMarkdownBlockNode>();
}
