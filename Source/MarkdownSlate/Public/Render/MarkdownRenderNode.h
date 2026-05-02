#pragma once

#include "CoreMinimal.h"
#include "Markdown/MarkdownNodeTypes.h"

struct FMarkdownTableModel;

enum class EMarkdownRenderNodeType : uint8
{
	Container,
	PlainText,
	Heading,
	Paragraph,
	Strong,
	Emphasis,
	Link,
	Image,
	CodeInline,
	CodeBlock,
	UnorderedList,
	OrderedList,
	ListItem,
	HorizontalRule,
	Blockquote,
	Strikethrough,
	Underline,
	Table,
	TableRow,
	TableCell,
};

struct FMarkdownRenderNode
{
	EMarkdownRenderNodeType Type = EMarkdownRenderNodeType::Paragraph;
	TArray<TSharedPtr<FMarkdownRenderNode>> Children;
	TSharedPtr<FMarkdownRenderNode> Parent;

	FText TextContent;
	FString LinkUrl;
	FString ImageUrl;
	FString ImageTitle;
	int32 HeadingLevel = 1;
	FString CodeLanguage;
	int32 OrderedListStart = 1;
	bool bIsTightList = true;
	bool bIsTaskItem = false;
	TCHAR TaskMark = ' ';

	TSharedPtr<FMarkdownTableModel> TableModel;
};
