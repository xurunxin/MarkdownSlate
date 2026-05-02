#include "Image/MarkdownImageLoader.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"
#include "Async/Async.h"

struct FMarkdownImageLoadTask
{
	FString Url;
	FOnMarkdownImageLoaded OnLoaded;
	FOnMarkdownImageFailed OnFailed;
	bool bCancelled = false;
};

void FMarkdownDefaultImageLoader::LoadImage(const FString& Url,
	FOnMarkdownImageLoaded OnLoaded,
	FOnMarkdownImageFailed OnFailed)
{
	auto Task = MakeShared<FMarkdownImageLoadTask>();
	Task->Url = Url;
	Task->OnLoaded = OnLoaded;
	Task->OnFailed = OnFailed;
	PendingTasks.Add(Task);

	TWeakPtr<FMarkdownImageLoadTask> WeakTask = Task;

	Async(EAsyncExecution::ThreadPool, [WeakTask]()
	{
		// Simulate async load — in real impl, load from disk/network
		FPlatformProcess::Sleep(0.05f);

		AsyncTask(ENamedThreads::GameThread, [WeakTask]()
		{
			auto Task = WeakTask.Pin();
			if (!Task.IsValid() || Task->bCancelled) return;

			// Create fallback brush on failure
			if (Task->OnFailed.IsBound())
			{
				Task->OnFailed.Execute();
			}
		});
	});
}

void FMarkdownDefaultImageLoader::CancelAll()
{
	for (auto& Task : PendingTasks)
	{
		if (Task.IsValid())
			Task->bCancelled = true;
	}
	PendingTasks.Empty();
}
