#pragma once

#include "CoreMinimal.h"
#include "MarkdownNodeTypes.h"

struct FMarkdownTextSegment
{
	EMarkdownTextType Type = EMarkdownTextType::Normal;
	FString Content;

	FMarkdownTextSegment() = default;
	FMarkdownTextSegment(EMarkdownTextType InType, const FString& InContent)
		: Type(InType), Content(InContent) {}
};

struct FMarkdownAstNode
{
	TArray<TSharedPtr<FMarkdownAstNode>> Children;
	TWeakPtr<FMarkdownAstNode> Parent;

	virtual ~FMarkdownAstNode() = default;
	virtual bool IsBlockNode() const { return false; }
	virtual bool IsSpanNode() const { return false; }
	virtual bool IsTextNode() const { return false; }
};

struct FMarkdownBlockNode : public FMarkdownAstNode
{
	virtual bool IsBlockNode() const override { return true; }
	EMarkdownBlockType Type = EMarkdownBlockType::Paragraph;

	int32 HeadingLevel = 1;
	bool bIsTightList = true;
	TCHAR ListBulletChar = '-';
	int32 OrderedListStart = 1;
	TCHAR OrderedListDelimiter = '.';
	bool bIsTaskItem = false;
	TCHAR TaskMark = ' ';
	FString CodeLanguage;

	int32 TableColumnCount = 0;
	int32 TableHeadRowCount = 0;
	int32 TableBodyRowCount = 0;

	enum class EAlign : uint8 { Default, Left, Center, Right };
	EAlign ColumnAlign = EAlign::Default;
};

struct FMarkdownSpanNode : public FMarkdownAstNode
{
	virtual bool IsSpanNode() const override { return true; }
	EMarkdownSpanType Type = EMarkdownSpanType::Emphasis;

	FString Href;
	FString Title;
	bool bIsAutolink = false;
	FString ImageSrc;
	FString ImageTitle;

	TArray<FMarkdownTextSegment> TextSegments;
};

struct FMarkdownTextNode : public FMarkdownAstNode
{
	virtual bool IsTextNode() const override { return true; }
	FString Text;
};
