#include "Render/MarkdownRenderBuilder.h"

static EMarkdownRenderNodeType MapBlockToRenderType(EMarkdownBlockType InType)
{
	switch (InType)
	{
	case EMarkdownBlockType::Paragraph:   return EMarkdownRenderNodeType::Paragraph;
	case EMarkdownBlockType::Heading:     return EMarkdownRenderNodeType::Heading;
	case EMarkdownBlockType::CodeBlock:   return EMarkdownRenderNodeType::CodeBlock;
	case EMarkdownBlockType::UnorderedList: return EMarkdownRenderNodeType::UnorderedList;
	case EMarkdownBlockType::OrderedList: return EMarkdownRenderNodeType::OrderedList;
	case EMarkdownBlockType::ListItem:    return EMarkdownRenderNodeType::ListItem;
	case EMarkdownBlockType::HorizontalRule: return EMarkdownRenderNodeType::HorizontalRule;
	case EMarkdownBlockType::Quote:       return EMarkdownRenderNodeType::Blockquote;
	case EMarkdownBlockType::Table:       return EMarkdownRenderNodeType::Table;
	case EMarkdownBlockType::TableRow:    return EMarkdownRenderNodeType::TableRow;
	case EMarkdownBlockType::TableHeader:
	case EMarkdownBlockType::TableData:   return EMarkdownRenderNodeType::TableCell;
	default: return EMarkdownRenderNodeType::Container;
	}
}

static EMarkdownRenderNodeType MapSpanToRenderType(EMarkdownSpanType InType)
{
	switch (InType)
	{
	case EMarkdownSpanType::Strong:         return EMarkdownRenderNodeType::Strong;
	case EMarkdownSpanType::Emphasis:       return EMarkdownRenderNodeType::Emphasis;
	case EMarkdownSpanType::Link:           return EMarkdownRenderNodeType::Link;
	case EMarkdownSpanType::Image:          return EMarkdownRenderNodeType::Image;
	case EMarkdownSpanType::Code:           return EMarkdownRenderNodeType::CodeInline;
	case EMarkdownSpanType::Strikethrough:  return EMarkdownRenderNodeType::Strikethrough;
	case EMarkdownSpanType::Underline:      return EMarkdownRenderNodeType::Underline;
	default: return EMarkdownRenderNodeType::Container;
	}
}

TSharedPtr<FMarkdownRenderNode> FMarkdownRenderBuilder::Build(const TSharedPtr<FMarkdownBlockNode>& RootAst)
{
	if (!RootAst.IsValid())
	{
		return MakeShared<FMarkdownRenderNode>();
	}

	TSharedPtr<FMarkdownRenderNode> Root = MakeShared<FMarkdownRenderNode>();
	Root->Type = EMarkdownRenderNodeType::Container;

	for (const auto& Child : RootAst->Children)
	{
		if (Child->IsBlockNode())
		{
			ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(Child), Root);
		}
		else if (Child->IsSpanNode())
		{
			ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(Child), Root);
		}
		else if (Child->IsTextNode())
		{
			auto TextNode = StaticCastSharedPtr<FMarkdownTextNode>(Child);
			auto RenderNode = MakeShared<FMarkdownRenderNode>();
			RenderNode->Type = EMarkdownRenderNodeType::PlainText;
			RenderNode->TextContent = FText::FromString(TextNode->Text);
			RenderNode->Parent = Root;
			Root->Children.Add(RenderNode);
		}
	}

	return Root;
}

void FMarkdownRenderBuilder::ConvertBlock(const TSharedPtr<FMarkdownBlockNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent)
{
	if (!AstNode.IsValid()) return;

	auto RenderNode = MakeShared<FMarkdownRenderNode>();
	RenderNode->Type = MapBlockToRenderType(AstNode->Type);
	RenderNode->HeadingLevel = AstNode->HeadingLevel;
	RenderNode->CodeLanguage = AstNode->CodeLanguage;
	RenderNode->OrderedListStart = AstNode->OrderedListStart;
	RenderNode->bIsTightList = AstNode->bIsTightList;
	RenderNode->Parent = OutParent;

	OutParent->Children.Add(RenderNode);

	// Process children in order (text, spans, sub-blocks are interleaved)
	for (const auto& Child : AstNode->Children)
	{
		if (Child->IsTextNode())
		{
			auto TextNode = StaticCastSharedPtr<FMarkdownTextNode>(Child);
			auto TextRender = MakeShared<FMarkdownRenderNode>();
			TextRender->Type = EMarkdownRenderNodeType::PlainText;
			TextRender->TextContent = FText::FromString(TextNode->Text);
			TextRender->Parent = RenderNode;
			RenderNode->Children.Add(TextRender);
		}
		else if (Child->IsSpanNode())
		{
			ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(Child), RenderNode);
		}
		else if (Child->IsBlockNode())
		{
			ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(Child), RenderNode);
		}
	}
}

void FMarkdownRenderBuilder::ConvertSpan(const TSharedPtr<FMarkdownSpanNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent)
{
	if (!AstNode.IsValid()) return;

	auto RenderNode = MakeShared<FMarkdownRenderNode>();
	RenderNode->Type = MapSpanToRenderType(AstNode->Type);
	RenderNode->LinkUrl = AstNode->Href;
	RenderNode->ImageUrl = AstNode->ImageSrc;
	RenderNode->ImageTitle = AstNode->ImageTitle;
	RenderNode->Parent = OutParent;

	OutParent->Children.Add(RenderNode);

	// Process children in order (text nodes are inline content of the span)
	for (const auto& Child : AstNode->Children)
	{
		if (Child->IsTextNode())
		{
			auto TextNode = StaticCastSharedPtr<FMarkdownTextNode>(Child);
			auto TextRender = MakeShared<FMarkdownRenderNode>();
			TextRender->Type = EMarkdownRenderNodeType::PlainText;
			TextRender->TextContent = FText::FromString(TextNode->Text);
			TextRender->Parent = RenderNode;
			RenderNode->Children.Add(TextRender);
		}
		else if (Child->IsSpanNode())
		{
			ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(Child), RenderNode);
		}
		else if (Child->IsBlockNode())
		{
			ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(Child), RenderNode);
		}
	}
}
