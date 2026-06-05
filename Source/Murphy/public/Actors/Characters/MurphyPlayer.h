
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MurphyPlayer.generated.h"

struct FInputActionValue;

class UInputMappingContext;
class UInputAction;
class UVoiceRecorderComponent;

class AAgentNPCBase;
class UMainHUD;

UENUM(BlueprintType)
enum class EPlayerChatState : uint8
{
	Idle,
	Recording,
	WaitingForAI,
	Talking
};

UCLASS()
class MURPHY_API AMurphyPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	AMurphyPlayer();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	
protected:
	// === VoiceRecorder ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VoiceChat")
	TObjectPtr<UVoiceRecorderComponent> VoiceRecorderComp;

public:		
	UVoiceRecorderComponent* GetVoiceRecorderComp() const { return VoiceRecorderComp; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Chat")
	TObjectPtr<AAgentNPCBase> TargetNPC;
	
public:
	// === Chat State Machine ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Chat")
	EPlayerChatState CurChatState = EPlayerChatState::Idle;
	
	UFUNCTION(BlueprintCallable, Category="Murphy|Chat")
	void SetChatState(EPlayerChatState NewState) { CurChatState = NewState; }
	UFUNCTION(BlueprintCallable, Category="Murphy|Chat")
	EPlayerChatState GetChatState() const { return CurChatState; }
	
	// === Chat Interaction ===
	UFUNCTION(BlueprintCallable, Category = "Murphy|Chat")
	void StartChatWithNPC(AAgentNPCBase* NPC);
	UFUNCTION(BlueprintCallable, Category = "Murphy|Chat")
	void EndChatWithNPC();
	
	// 회전 완료 전 대화 종료 방지 및 회전 보장 플래그
	bool bIsAligningWithNPC  = false;
	bool bPendingEndChat = false;
	
	float RecordTime = 0.0f;
	float GetRecordTime() const;
	
public:
	// === Input ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|Input")
	UInputMappingContext* IMC_Murphy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|Input")
	UInputAction* IA_Move;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|Input")
	UInputAction* IA_MouseLook;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_Record;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_PlayAudio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_ToggleBag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_TogglePhone;
	
	// === Input Action === 
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void RecordStart(const FInputActionValue& Value);
	virtual void RecordEnd(const FInputActionValue& Value);
	virtual void RecordAudioPlay(const FInputActionValue& Value);

	// Toggle Bag, Phone Action
	void ToggleBagPressed();
	void TogglePhonePressed();

protected:
	// === UI ===
	UPROPERTY(EditAnywhere, Category = "Murphy|UI")
	TSubclassOf<UMainHUD> MainHUDClass;
	UPROPERTY()
	TObjectPtr<UMainHUD> MainHUDInstance;
	
};
