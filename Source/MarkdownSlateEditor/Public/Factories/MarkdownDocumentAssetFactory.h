#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MarkdownDocumentAssetFactory.generated.h"

UCLASS()
class UMarkdownDocumentAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMarkdownDocumentAssetFactory();

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	virtual bool FactoryCanImport(const FString& Filename) override;
};
