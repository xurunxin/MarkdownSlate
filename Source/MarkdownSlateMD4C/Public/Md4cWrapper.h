#pragma once

#include "CoreMinimal.h"

struct MD_PARSER;

MARKDOWNSLATEMD4C_API int MarkdownParse(const char* Text, unsigned Size, const MD_PARSER* Parser, void* UserData);
