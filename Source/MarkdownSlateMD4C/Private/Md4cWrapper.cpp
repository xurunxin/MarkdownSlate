#include "Md4cWrapper.h"
#include "md4c.h"

int MarkdownParse(const char* Text, unsigned Size, const MD_PARSER* Parser, void* UserData)
{
	return md_parse(Text, Size, Parser, UserData);
}
