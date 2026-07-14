#include "MarkdownSlate.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Modules/ModuleManager.h"

void FMarkdownSlateModule::StartupModule()
{
	FMarkdownEmojiConfig EmojiConfig;
	FMarkdownAtlasEmojiProvider EmojiProvider(EmojiConfig);
	if (!EmojiProvider.SupportsAtlasRendering())
	{
		EmojiProvider.GetAtlasPtr()->AutoLoadAtlas(EmojiConfig.TwemojiAssetRoot);
	}
}

void FMarkdownSlateModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FMarkdownSlateModule, MarkdownSlate)
