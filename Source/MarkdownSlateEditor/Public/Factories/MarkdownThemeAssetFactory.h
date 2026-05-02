#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MarkdownThemeAssetFactory.generated.h"

UCLASS()
class UMarkdownThemeAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMarkdownThemeAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, FFeedbackContext* Warn) override;
};
