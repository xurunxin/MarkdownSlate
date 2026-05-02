#include "Render/MarkdownRenderBuilder.h"
#include "Table/MarkdownTableModel.h"

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

static void BuildTable(const TSharedPtr<FMarkdownBlockNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& RenderNode,
	FMarkdownRenderBuilder& Builder)
{
	auto TableModel = MakeShared<FMarkdownTableModel>();
	TArray<FMarkdownTableCell> CurrentRow;

	for (const auto& Section : AstNode->Children)
	{
		if (!Section->IsBlockNode()) continue;
		auto SecBlock = StaticCastSharedPtr<FMarkdownBlockNode>(Section);
		if (SecBlock->Type != EMarkdownBlockType::TableHead && SecBlock->Type != EMarkdownBlockType::TableBody)
			continue;

		if (SecBlock->Type == EMarkdownBlockType::TableHead)
			TableModel->bHasColumnHeader = true;

		for (const auto& RowNode : SecBlock->Children)
		{
			if (!RowNode->IsBlockNode()) continue;
			auto RowBlock = StaticCastSharedPtr<FMarkdownBlockNode>(RowNode);
			if (RowBlock->Type != EMarkdownBlockType::TableRow) continue;

			CurrentRow.Reset();
			for (const auto& CellNode : RowBlock->Children)
			{
				FMarkdownTableCell Cell;
				Cell.bIsHeader = (SecBlock->Type == EMarkdownBlockType::TableHead);
				if (CellNode->IsBlockNode())
				{
					auto CellRender = MakeShared<FMarkdownRenderNode>();
					CellRender->Type = EMarkdownRenderNodeType::Container;
					auto CellBlock = StaticCastSharedPtr<FMarkdownBlockNode>(CellNode);
					for (const auto& InnerChild : CellBlock->Children)
					{
						if (InnerChild->IsTextNode())
						{
							auto TR = MakeShared<FMarkdownRenderNode>();
							TR->Type = EMarkdownRenderNodeType::PlainText;
							TR->TextContent = FText::FromString(
								StaticCastSharedPtr<FMarkdownTextNode>(InnerChild)->Text);
							CellRender->Children.Add(TR);
						}
						else if (InnerChild->IsSpanNode())
							Builder.ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(InnerChild), CellRender);
						else if (InnerChild->IsBlockNode())
							Builder.ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(InnerChild), CellRender);
					}
					Cell.ContentNode = CellRender;
				}
				CurrentRow.Add(Cell);
			}
			TableModel->Rows.Add(CurrentRow);
		}
	}

	if (TableModel->Rows.Num() > 0)
		TableModel->ColumnCount = TableModel->Rows[0].Num();
	RenderNode->TableModel = TableModel;
}

static void ProcessAstChildren(const TSharedPtr<FMarkdownAstNode>& AstNode,
	const TSharedPtr<FMarkdownRenderNode>& RenderNode, FMarkdownRenderBuilder& Builder)
{
	for (const auto& Child : AstNode->Children)
	{
		if (Child->IsTextNode())
		{
			auto TR = MakeShared<FMarkdownRenderNode>();
			TR->Type = EMarkdownRenderNodeType::PlainText;
			TR->TextContent = FText::FromString(StaticCastSharedPtr<FMarkdownTextNode>(Child)->Text);
			TR->Parent = RenderNode; RenderNode->Children.Add(TR);
		}
		else if (Child->IsSpanNode())
			Builder.ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(Child), RenderNode);
		else if (Child->IsBlockNode())
			Builder.ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(Child), RenderNode);
	}
}

TSharedPtr<FMarkdownRenderNode> FMarkdownRenderBuilder::Build(const TSharedPtr<FMarkdownBlockNode>& RootAst)
{
	if (!RootAst.IsValid()) return MakeShared<FMarkdownRenderNode>();
	auto Root = MakeShared<FMarkdownRenderNode>();
	Root->Type = EMarkdownRenderNodeType::Container;
	for (const auto& Child : RootAst->Children)
	{
		if (Child->IsBlockNode())
			ConvertBlock(StaticCastSharedPtr<FMarkdownBlockNode>(Child), Root);
		else if (Child->IsSpanNode())
			ConvertSpan(StaticCastSharedPtr<FMarkdownSpanNode>(Child), Root);
		else if (Child->IsTextNode())
		{
			auto TR = MakeShared<FMarkdownRenderNode>();
			TR->Type = EMarkdownRenderNodeType::PlainText;
			TR->TextContent = FText::FromString(StaticCastSharedPtr<FMarkdownTextNode>(Child)->Text);
			TR->Parent = Root; Root->Children.Add(TR);
		}
	}
	return Root;
}

void FMarkdownRenderBuilder::ConvertBlock(const TSharedPtr<FMarkdownBlockNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent)
{
	if (!AstNode.IsValid()) return;
	auto RN = MakeShared<FMarkdownRenderNode>();
	RN->Type = MapBlockToRenderType(AstNode->Type);
	RN->HeadingLevel = AstNode->HeadingLevel;
	RN->CodeLanguage = AstNode->CodeLanguage;
	RN->OrderedListStart = AstNode->OrderedListStart;
	RN->bIsTightList = AstNode->bIsTightList;
	RN->bIsTaskItem = AstNode->bIsTaskItem;
	RN->TaskMark = AstNode->TaskMark;
	RN->Parent = OutParent;
	OutParent->Children.Add(RN);

	if (AstNode->Type == EMarkdownBlockType::Table)
		BuildTable(AstNode, RN, *this);
	else
		ProcessAstChildren(AstNode, RN, *this);
}

void FMarkdownRenderBuilder::ConvertSpan(const TSharedPtr<FMarkdownSpanNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent)
{
	if (!AstNode.IsValid()) return;
	auto RN = MakeShared<FMarkdownRenderNode>();
	RN->Type = MapSpanToRenderType(AstNode->Type);
	RN->LinkUrl = AstNode->Href;
	RN->ImageUrl = AstNode->ImageSrc;
	RN->ImageTitle = AstNode->ImageTitle;
	RN->Parent = OutParent;
	OutParent->Children.Add(RN);
	ProcessAstChildren(AstNode, RN, *this);
}
