#include "Emoji/MarkdownEmojiAtlas.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "ImageUtils.h"
#include "Styling/SlateBrush.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/FileManager.h"

FMarkdownEmojiAtlas::FMarkdownEmojiAtlas()
{
	GridCols = AtlasSize / CellSize;
}

void FMarkdownEmojiAtlas::SetAtlasConfig(int32 InCellSize, int32 InAtlasSize)
{
	CellSize = InCellSize;
	AtlasSize = InAtlasSize;
	GridCols = AtlasSize / CellSize;
	BrushCache.Empty();
}

void FMarkdownEmojiAtlas::SetAtlasTexture(UTexture2D* InTexture)
{
	AtlasTexture = InTexture;
	BrushCache.Empty();
}

void FMarkdownEmojiAtlas::AddEntry(const FString& Codepoint, int32 Col, int32 Row)
{
	FEmojiAtlasEntry Entry;
	Entry.Codepoint = Codepoint;
	Entry.GridCol = Col;
	Entry.GridRow = Row;
	Entries.Add(Codepoint, Entry);
}

void FMarkdownEmojiAtlas::BuildDefaultMapping(const TArray<FString>& SortedCodepoints, int32 CellsPerRow)
{
	CellsPerRow = CellsPerRow > 0 ? CellsPerRow : GridCols;
	for (int32 i = 0; i < SortedCodepoints.Num(); ++i)
	{
		int32 Col = i % CellsPerRow;
		int32 Row = i / CellsPerRow;
		AddEntry(SortedCodepoints[i], Col, Row);
	}
}

bool FMarkdownEmojiAtlas::GetGridCoords(const FString& TwemojiCode, int32& OutCol, int32& OutRow) const
{
	const FEmojiAtlasEntry* Found = Entries.Find(TwemojiCode);
	if (Found)
	{
		OutCol = Found->GridCol;
		OutRow = Found->GridRow;
		return true;
	}
	return false;
}

bool FMarkdownEmojiAtlas::GetUVRect(const FString& TwemojiCode, FVector2D& OutUVMin, FVector2D& OutUVMax) const
{
	int32 Col, Row;
	if (!GetGridCoords(TwemojiCode, Col, Row)) return false;

	float CellU = (float)CellSize / (float)AtlasSize;
	OutUVMin.X = (float)Col * CellU;
	OutUVMin.Y = (float)Row * CellU;
	OutUVMax.X = OutUVMin.X + CellU;
	OutUVMax.Y = OutUVMin.Y + CellU;
	return true;
}

TSharedPtr<FSlateBrush> FMarkdownEmojiAtlas::GetEmojiBrush(const FString& TwemojiCode, float FontSize)
{
	if (!AtlasTexture) return nullptr;

	TSharedPtr<FSlateBrush>* Cached = BrushCache.Find(TwemojiCode);
	if (Cached && Cached->IsValid()) return *Cached;

	FVector2D UVMin, UVMax;
	if (!GetUVRect(TwemojiCode, UVMin, UVMax)) return nullptr;

	auto Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(AtlasTexture);
	Brush->ImageSize = FVector2D(AtlasSize);
	Brush->DrawAs = ESlateBrushDrawType::Image;

	// Store UV info in a custom metadata or use dynamic brush
	// For now, create brush that renders full atlas; UV cropping done in widget via DrawElement
	BrushCache.Add(TwemojiCode, Brush);
	return Brush;
}

bool FMarkdownEmojiAtlas::LoadMappingFromJsonFile(const FString& JsonFilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
		return false;

	TSharedPtr<FJsonValue> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, Root))
		return false;

	const TSharedPtr<FJsonObject>* RootObj;
	if (!Root->TryGetObject(RootObj)) return false;

	int32 FileCellSize = (*RootObj)->GetIntegerField(TEXT("cell_size"));
	int32 FileAtlasSize = (*RootObj)->GetIntegerField(TEXT("atlas_size"));
	SetAtlasConfig(FileCellSize, FileAtlasSize);

	const TArray<TSharedPtr<FJsonValue>>* EntryList;
	if (!(*RootObj)->TryGetArrayField(TEXT("entries"), EntryList)) return false;

	Entries.Empty();
	for (const auto& EntryVal : *EntryList)
	{
		const TSharedPtr<FJsonObject>* EntryObj;
		if (!EntryVal->TryGetObject(EntryObj)) continue;

		FString Code = (*EntryObj)->GetStringField(TEXT("codepoint"));
		int32 Col = (*EntryObj)->GetIntegerField(TEXT("col"));
		int32 Row = (*EntryObj)->GetIntegerField(TEXT("row"));
		AddEntry(Code, Col, Row);
	}

	return Entries.Num() > 0;
}

bool FMarkdownEmojiAtlas::AutoLoadAtlas(const FString& ContentEmojiPath)
{
	// Build file paths
	FString PluginDir = FPaths::ProjectPluginsDir() / TEXT("MarkdownSlate");
	FString AtlasPngPath  = PluginDir / ContentEmojiPath / TEXT("TwemojiAtlas.png");
	FString AtlasJsonPath = PluginDir / ContentEmojiPath / TEXT("TwemojiAtlas.json");

	// Load JSON mapping first
	if (!LoadMappingFromJsonFile(AtlasJsonPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MarkdownEmojiAtlas] Failed to load mapping: %s"), *AtlasJsonPath);
		return false;
	}

	// Load atlas texture via FImageUtils
	IFileManager& FM = IFileManager::Get();
	if (!FM.FileExists(*AtlasPngPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MarkdownEmojiAtlas] Atlas PNG not found: %s"), *AtlasPngPath);
		return false;
	}

	UTexture2D* LoadedTexture = FImageUtils::ImportFileAsTexture2D(AtlasPngPath);
	if (!LoadedTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MarkdownEmojiAtlas] Failed to import texture: %s"), *AtlasPngPath);
		return false;
	}

	SetAtlasTexture(LoadedTexture);

	UE_LOG(LogTemp, Log, TEXT("[MarkdownEmojiAtlas] Loaded atlas: %d emoji, %dx%d texture"),
		Entries.Num(), AtlasSize, AtlasSize);

	return true;
}
