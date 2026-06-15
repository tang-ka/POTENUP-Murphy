
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/PlayerViewComponent.h" // EPlayerViewState, EChatViewMode 사용
#include "Framework/MurphyGameModeBase.h"
#include "MurphyPlayer.generated.h"


class UCameraComponent;
// class UPlayerViewComponent; // PlayerViewComponent.h include로 대체 (enum 전체 정의 필요)
class USpringArmComponent;
class USystemMenuUI;
struct FInputActionValue;

class UInputMappingContext;
class UInputAction;
class UVoiceRecorderComponent;

class AAgentNPCBase;
class UMainHUD;
class AItemBaseActor;

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
	// virtual void Tick(float DeltaSeconds) override; // PlayerViewComponent::TickComponent으로 이전

protected:
#pragma region Components
	// === VoiceRecorder ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VoiceChat")
	TObjectPtr<UVoiceRecorderComponent> VoiceRecorderComp;

	// === Camera ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Murphy|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	// === Util ====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="View")
	TObjectPtr<UPlayerViewComponent> PlayerViewComp;

public:		
	UVoiceRecorderComponent* GetVoiceRecorderComp() const { return VoiceRecorderComp; }
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	UPlayerViewComponent* GetPlayerViewComp() const { return PlayerViewComp; }
#pragma endregion
	
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

	// 대화 시 시점 전환 방식 - 씬/BP 인스턴스별로 에디터에서 바로 전환 테스트 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|View")
	EChatViewMode ChatViewMode = EChatViewMode::ThirdPersonFocus;

	// PlayerViewComp의 시점 전환 완료 콜백 (기존 bPendingEndChat 처리 대체)
	UFUNCTION()
	void HandleViewTransitionComplete(EPlayerViewState ReachedState);

	// 카메라 포커싱
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Murphy|Chat")
	// FRotator CamTargetRot = FRotator(345.0f, 245.0f, 0.0f); // 고정 P=345.0f Y=245.0f R=0.0f -> PlayerViewComp::ThirdPersonFocusCamRot으로 이전
	// void FocusNPC(float DeltaSeconds); // PlayerViewComp::TickThirdPersonFocusAlign으로 이전

	// 회전 완료 전 대화 종료 방지 및 회전 보장 플래그
	// bool bIsAligningWithNPC  = false; // PlayerViewComp::bIsTransitioning으로 이전
	// bool bPendingEndChat = false; // PlayerViewComp::PendingViewState로 이전

	float RecordTime = 0.0f;
	float GetRecordTime() const;
	
public:
	// === Movement Lock ===
	// 비행기 좌석 등에서 이동 입력을 잠그기 위한 플래그
	// 서버에서 호출: 권위 값 세팅 + 소유 클라로 RPC 동기화
	UFUNCTION(BlueprintCallable)
	void SetMovementLocked(bool bLocked);

	UFUNCTION(BlueprintPure)
	bool IsMovementLocked() const
	{
		return bMovementLocked;
	}

	// 소유 클라이언트에 이동 잠금 상태를 동기화하는 RPC
	UFUNCTION(Client, Reliable)
	void Client_SetMovementLocked(bool bLocked);

public:
	// === Input ===
#pragma region InputAction 
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_Interact;	// F키 - 아이템 상호작용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|Input")
	UInputAction* IA_SystemMenu;
#pragma endregion 


	// === Input Action === 
#pragma region Input Action Function
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void RecordStart(const FInputActionValue& Value);
	virtual void RecordEnd(const FInputActionValue& Value);
	virtual void RecordAudioPlay(const FInputActionValue& Value);

#pragma endregion 

	// Toggle Bag, Phone Action
	void ToggleBagPressed();
	void TogglePhonePressed();

	// F키 - 아이템 상호작용
	void InteractPressed();

	// Update Mic UI
	void SetMicUIState(bool bIsRecording);
	
	void SystemMenuPressed();

	/** 상호작용 가능한 객체 탐색용 반경 (Capsule Overlap) 및 시야각 */
	UPROPERTY(EditAnywhere, Category = "Murphy|Item")
	float InteractAngleDeg = 60.0f;

	/** ItemBaseActor에서 가방 위젯에 접근하기 위한 Getter */
	UMainHUD* GetMainHUD() const { return MainHUDInstance; }

protected:
	// === UI ===
	UPROPERTY(EditAnywhere, Category = "Murphy|UI")
	TSubclassOf<UMainHUD> MainHUDClass;
	UPROPERTY()
	TObjectPtr<UMainHUD> MainHUDInstance;
	
	UPROPERTY(EditAnywhere, Category = "Murphy|UI")
	TSubclassOf<USystemMenuUI> SystemMenuClass;
	UPROPERTY()
	TObjectPtr<USystemMenuUI> SystemMenuInstance;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Murphy|flag", meta=(AllowPrivateAccess="true"))
	bool bMovementLocked = false;
};
