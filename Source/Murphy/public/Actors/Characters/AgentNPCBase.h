
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AIDataTypes.h"
#include "AgentNPCBase.generated.h"

class UBoxComponent;
class UAudioComponent;
class UWidgetComponent;
class UScenarioSubsystem;
class UAgentEmojiUI;

UENUM(BlueprintType)
enum class EAgentEmotion : uint8
{
	Normal,
	Smile,
	Suspect,
	Embarrassed,
	Annoyed,
	Angry,
	Furious
};

UCLASS()
class MURPHY_API AAgentNPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAgentNPCBase();
	
	// 독점 대화 로직 관련 복제 프로퍼티 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	// === Components ===
	// [상호작용] 플레이어 접근 인식을 위한 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	TObjectPtr<UBoxComponent> InteractionBox;
	
	// [음성 및 페이셜 트래킹] AI TTS 재생 및 립싱크 데이터 추출의 핵심 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	TObjectPtr<UAudioComponent> VoiceComp;
	
	// [메타휴먼] 메타휴먼 얼굴 파츠를 조립할 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|MetaHuman")
	TObjectPtr<USkeletalMeshComponent> FaceMesh;
	
	// [이모지] Agent의 감정을 더 정확히 표현 할 이모지 위젯 컴포넌트 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|MetaHuman")
	TObjectPtr<UWidgetComponent> EmojiComp;
	UPROPERTY()
	TObjectPtr<UAgentEmojiUI> EmojiUI;
	
	// AI 응답 대기 중 재생할 타이핑 사운드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	TObjectPtr<UAudioComponent> TypingAudioComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Info")
	FString NPCName;
	
	// AI 서버 응답을 대기하며 타이핑 애니메이션을 재생해야 하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bIsWaitingForAIResponse = false;

public:
	// 애니메이션 블루프린트에서 상태를 가져가기 위한 Getter
	UFUNCTION(BlueprintPure, Category = "AI|State")
	bool IsWaitingForAIResponse() const { return bIsWaitingForAIResponse; }
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> PassportSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> TypingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> ForShortAnswerSound;

	void ForShortAnswer();
	
	// todo: DataAsset이나 DataTable로 만들어야함. 아니면 Struct에 Enum을 추가? 
	// 감정별 이모지 텍스처를 매핑해두는 딕셔너리 (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Emoji")
	TMap<EAgentEmotion, TObjectPtr<UTexture2D>> EmotionTextures;
	
private:
	// todo: Enum 처리
	// 파싱된 감정 상태를 애니메이션 블루프린트로 전달 
	void UpdateEmotion(EAgentEmotion EmotionLevel);
	
private:
	// === InteractionBox Overlap Event ===
	UFUNCTION()
	void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	// 누군가 이미 대화 중인지 상태를 저장 (서버 -> 클라 동기화)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="AI|Chat")
	bool bIsTalkingWithPlayer = false;
	
public:
	// === Conversation ===
	UFUNCTION(BlueprintPure, Category="AI|Chat")
	bool CanTalkWithPlayer() const { return !bIsTalkingWithPlayer; }
	
	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	bool TryStartConversation();
	
	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	void EndConversation();

protected:
	// === Timer ===
	bool bIsWaitingForPlayer = false;
	float CurWaitTime = 0.0f;
	const float MaxWaitTime = 60.0f; // 1분 타임아웃

	FTimerHandle VoiceTimerHandle;	// 델리게이트 대신 오디오 길이에 맞춰 직접 호출할 타이머 핸들
	
	// 타이핑 연출 시작 및 종료 내부 함수
	void StartTypingWait();
	void StopTypingWait();
	
public:
	UFUNCTION() // NPC음성 재생 끝났을 때 호출 될 함수
	void OnVoiceFinished();
	
	// 플레이어가 녹음을 완료해 전송했을 때 타이머를 끄기 위해 컨트롤러가 호출할 함수
	void NotifyPlayerSpoke();
	
public:
	// PlayerController가 서버 응답 수신 후 NPC에게 결과를 전달하는 함수 
	void ProcessDialogueResponse(const FAIResponseData& ResponseData);
	
	// AudioURL로 HTTP GET 요청 시작
	void DownloadAndPlayAudio(const FString& AudioURL);
	
	// HTTP GET 응답 콜백 - WAV 바이너리를 받아 재생
	void OnAudioDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};