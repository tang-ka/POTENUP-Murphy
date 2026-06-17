#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WonFaceAnimInstance.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MURPHY_API UWonFaceAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Emotion")
	TMap<FName, float> TargetEmotionMap;
};
