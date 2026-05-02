#pragma once

#include "CoreMinimal.h"
#include "MarkdownRenderNode.h"
#include "Markdown/MarkdownAst.h"

class MARKDOWNSLATE_API FMarkdownRenderBuilder
{
public:
	TSharedPtr<FMarkdownRenderNode> Build(const TSharedPtr<FMarkdownBlockNode>& RootAst);
	void ConvertBlock(const TSharedPtr<FMarkdownBlockNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent);
	void ConvertSpan(const TSharedPtr<FMarkdownSpanNode>& AstNode, const TSharedPtr<FMarkdownRenderNode>& OutParent);
};
