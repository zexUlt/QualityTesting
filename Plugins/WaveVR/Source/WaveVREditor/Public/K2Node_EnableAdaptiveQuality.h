// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "KismetCompiler.h"
#include "K2Node_EnableAdaptiveQuality.generated.h"

/**
 * K2Node which connects WaveVRBlueprintFunctionLibrary::EnableAdaptiveQuality_K2
 */
UCLASS()
class WAVEVREDITOR_API UK2Node_EnableAdaptiveQuality : public UK2Node
{
	GENERATED_BODY()

	UK2Node_EnableAdaptiveQuality(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = AdaptiveQuality)
	bool bHasDefaultModePin;

	UPROPERTY(EditAnywhere, Category = AdaptiveQuality)
	bool bHasFlagsPin;

	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	// End of UObject interface

	//UEdGraphNode implementation
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void PinDefaultValueChanged(UEdGraphPin * Pin) override;
	virtual void PinConnectionListChanged(UEdGraphPin* ChangedPin) override;
	//UEdGraphNode implementation

	//K2Node implementation
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	//K2Node implementation

private:
	void OnPinChanged(UEdGraphPin * Pin);

	UEdGraphPin* GetEnablePin();
	UEdGraphPin* GetModePin();
	UEdGraphPin* GetSendQualityEventPin();
	UEdGraphPin* GetAutoFoveationPin();
};
