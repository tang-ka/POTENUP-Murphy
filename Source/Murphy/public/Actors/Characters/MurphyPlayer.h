
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MurphyPlayer.generated.h"

class UVoiceRecorderComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;



UCLASS()
class MURPHY_API AMurphyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AMurphyPlayer();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void BeginPlay() override;

protected:
	// === VoiceRecorder ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VoiceChat")
	TObjectPtr<UVoiceRecorderComponent> VoiceRecorderComp;
	
public:		
	UVoiceRecorderComponent* GetVoiceRecorderComp() const { return VoiceRecorderComp; }
	
public:
	// === Input ===
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	// UInputMappingContext* IMC_Murphy;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	// UInputAction* IA_Move;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	// UInputAction* IA_MouseLook;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	// UInputAction* IA_Record;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	// UInputAction* IA_PlayAudio;
	
	// === Input Action === 
	// virtual void Move(const FInputActionValue& Value);
	// virtual void Look(const FInputActionValue& Value);
	// virtual void RecordStart(const FInputActionValue& Value);
	// virtual void RecordEnd(const FInputActionValue& Value);
	// virtual void RecordAudioPlay(const FInputActionValue& Value);
};
