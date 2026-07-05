#include "Emoji/MarkdownEmojiAtlas.h"
#include "Emoji/MarkdownEmojiAtlasData.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Texture.h"

FMarkdownEmojiAtlas::FMarkdownEmojiAtlas()
{
	GridCols = AtlasSize / CellSize;
}

FMarkdownEmojiAtlas::~FMarkdownEmojiAtlas()
{
	BrushCache.Empty();
	AtlasTextures.Empty();
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
	SetAtlasTexture(0, InTexture);
}

void FMarkdownEmojiAtlas::SetAtlasTexture(int32 Page, UTexture2D* InTexture)
{
	if (Page < 0)
	{
		return;
	}
	if (AtlasTextures.Num() <= Page)
	{
		AtlasTextures.SetNum(Page + 1);
	}
	BrushCache.Empty();
	if (InTexture)
	{
		InTexture->LODGroup = TEXTUREGROUP_UI;
		InTexture->Filter = TF_Bilinear;
	}
	AtlasTextures[Page] = InTexture;
}

void FMarkdownEmojiAtlas::AddEntry(const FString& Codepoint, int32 Page, int32 Col, int32 Row)
{
	FEmojiAtlasEntry Entry;
	Entry.Codepoint = Codepoint;
	Entry.Page = Page;
	Entry.GridCol = Col;
	Entry.GridRow = Row;
	Entries.Add(Codepoint, Entry);
}

void FMarkdownEmojiAtlas::AddEntry(const FString& Codepoint, int32 Col, int32 Row)
{
	AddEntry(Codepoint, 0, Col, Row);
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
	const FEmojiAtlasEntry* Found = nullptr;
	if (!ResolveAtlasEntry(TwemojiCode, Found))
	{
		return false;
	}

	if (Found)
	{
		OutCol = Found->GridCol;
		OutRow = Found->GridRow;
		return true;
	}
	return false;
}

bool FMarkdownEmojiAtlas::ResolveAtlasEntry(const FString& TwemojiCode, const FEmojiAtlasEntry*& OutEntry) const
{
	OutEntry = Entries.Find(TwemojiCode);
	if (OutEntry)
	{
		return true;
	}

	TArray<FString> Parts;
	TwemojiCode.ParseIntoArray(Parts, TEXT("-"), true);
	if (Parts.Num() <= 0)
	{
		return false;
	}

	TArray<FString> WithoutVariationParts;
	WithoutVariationParts.Reserve(Parts.Num());
	for (const FString& Part : Parts)
	{
		if (!Part.Equals(TEXT("fe0f"), ESearchCase::IgnoreCase))
		{
			WithoutVariationParts.Add(Part);
		}
	}

	const FString WithoutVariation = FString::Join(WithoutVariationParts, TEXT("-"));
	if (!WithoutVariation.Equals(TwemojiCode, ESearchCase::IgnoreCase))
	{
		OutEntry = Entries.Find(WithoutVariation);
		if (OutEntry)
		{
			return true;
		}
	}

	if (Parts.Num() == 1)
	{
		const FString WithVariation = TwemojiCode + TEXT("-fe0f");
		OutEntry = Entries.Find(WithVariation);
		if (OutEntry)
		{
			return true;
		}
	}

	return false;
}

bool FMarkdownEmojiAtlas::GetUVRect(const FString& TwemojiCode, FVector2D& OutUVMin, FVector2D& OutUVMax) const
{
	int32 Col, Row;
	if (!GetGridCoords(TwemojiCode, Col, Row)) return false;

	const float CellU = (float)CellSize / (float)AtlasSize;
	const float HalfTexel = 0.5f / (float)AtlasSize;
	OutUVMin.X = (float)Col * CellU + HalfTexel;
	OutUVMin.Y = (float)Row * CellU + HalfTexel;
	OutUVMax.X = OutUVMin.X + CellU - HalfTexel * 2.0f;
	OutUVMax.Y = OutUVMin.Y + CellU - HalfTexel * 2.0f;
	return true;
}

TSharedPtr<FSlateBrush> FMarkdownEmojiAtlas::GetEmojiBrush(const FString& TwemojiCode, float FontSize)
{
	const FString CacheKey = FString::Printf(TEXT("%s@%.1f"), *TwemojiCode, FontSize);
	TSharedPtr<FSlateBrush>* Cached = BrushCache.Find(CacheKey);
	if (Cached && Cached->IsValid()) return *Cached;

	const FEmojiAtlasEntry* Found = nullptr;
	if (!ResolveAtlasEntry(TwemojiCode, Found) || !Found)
	{
		return nullptr;
	}

	if (!AtlasTextures.IsValidIndex(Found->Page) || !AtlasTextures[Found->Page])
	{
		return nullptr;
	}

	const float CellU = (float)CellSize / (float)AtlasSize;
	const float HalfTexel = 0.5f / (float)AtlasSize;
	const FVector2D UVMin((float)Found->GridCol * CellU + HalfTexel, (float)Found->GridRow * CellU + HalfTexel);
	const FVector2D UVMax(UVMin.X + CellU - HalfTexel * 2.0f, UVMin.Y + CellU - HalfTexel * 2.0f);

	auto Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(AtlasTextures[Found->Page]);
	Brush->ImageSize = FVector2D(FontSize);
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->SetUVRegion(FBox2f(FVector2f(UVMin), FVector2f(UVMax)));

	BrushCache.Add(CacheKey, Brush);
	return Brush;
}

void FMarkdownEmojiAtlas::LoadBuiltInMapping()
{
	SetAtlasConfig(MarkdownEmojiAtlasData::CellSize, MarkdownEmojiAtlasData::AtlasSize);
	PageCount = MarkdownEmojiAtlasData::PageCount;

	Entries.Empty();
	for (int32 i = 0; i < MarkdownEmojiAtlasData::EntryCount; ++i)
	{
		const FMarkdownEmojiAtlasCodepointEntry& Entry = MarkdownEmojiAtlasData::Entries[i];
		AddEntry(Entry.Codepoint, Entry.Page, Entry.Col, Entry.Row);
	}
}

bool FMarkdownEmojiAtlas::AutoLoadAtlas(const FString& ContentEmojiPath)
{
	(void)ContentEmojiPath;
	LoadBuiltInMapping();

	AtlasTextures.Empty();
	for (int32 Page = 0; Page < PageCount; ++Page)
	{
		const FString AssetPath = FString::Printf(TEXT("/MarkdownSlate/Emoji/T_Emoji_%d.T_Emoji_%d"), Page, Page);
		UTexture2D* LoadedTexture = Cast<UTexture2D>(StaticLoadObject(
			UTexture2D::StaticClass(),
			nullptr,
			*AssetPath));
		if (!LoadedTexture)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MarkdownEmojiAtlas] Failed to load texture asset: %s"), *AssetPath);
			continue;
		}
		SetAtlasTexture(Page, LoadedTexture);
	}

	if (AtlasTextures.Num() <= 0)
	{
		UTexture2D* LoadedTexture = Cast<UTexture2D>(StaticLoadObject(
			UTexture2D::StaticClass(),
			nullptr,
			TEXT("/MarkdownSlate/Emoji/T_Emoji.T_Emoji")));
		if (LoadedTexture)
		{
			SetAtlasTexture(0, LoadedTexture);
		}
	}

	if (AtlasTextures.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MarkdownEmojiAtlas] Failed to load any atlas texture asset"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[MarkdownEmojiAtlas] Loaded cooked atlas: %d emoji, %d page(s), %dx%d texture pages"),
		Entries.Num(), AtlasTextures.Num(), AtlasSize, AtlasSize);

	return true;
}
