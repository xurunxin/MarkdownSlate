#include "MarkdownSlate.h"
#include "Emoji/MarkdownEmojiAssetProvider.h"
#include "Emoji/MarkdownEmojiAtlas.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogMarkdownSlate);

void FMarkdownSlateModule::StartupModule()
{
	FMarkdownEmojiConfig EmojiConfig;
	FMarkdownAtlasEmojiProvider EmojiProvider(EmojiConfig);
	if (!EmojiProvider.SupportsAtlasRendering())
	{
		if (!EmojiProvider.GetAtlasPtr()->AutoLoadAtlas(EmojiConfig.TwemojiAssetRoot))
		{
			UE_LOG(LogMarkdownSlate, Error, TEXT("Emoji atlas startup preload failed."));
			return;
		}
	}

	UE_LOG(
		LogMarkdownSlate,
		Display,
		TEXT("Emoji atlas ready at startup: %d entries."),
		EmojiProvider.GetAtlas()->GetEntryCount());
}

void FMarkdownSlateModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FMarkdownSlateModule, MarkdownSlate)
