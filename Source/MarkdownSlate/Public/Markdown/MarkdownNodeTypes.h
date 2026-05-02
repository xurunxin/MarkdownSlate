#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMarkdownBlockType : uint8
{
	Document,
	Quote,
	UnorderedList,
	OrderedList,
	ListItem,
	HorizontalRule,
	Heading,
	CodeBlock,
	HtmlBlock,
	Paragraph,
	Table,
	TableHead,
	TableBody,
	TableRow,
	TableHeader,
	TableData,
};

UENUM(BlueprintType)
enum class EMarkdownSpanType : uint8
{
	Emphasis,
	Strong,
	Link,
	Image,
	Code,
	Strikethrough,
	Underline,
};

UENUM(BlueprintType)
enum class EMarkdownTextType : uint8
{
	Normal,
	LineBreak,
	SoftBreak,
	Code,
	Html,
};
