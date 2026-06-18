
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IHttpRequest.h"
#include "Data/AIDataTypes.h"
#include "Manager/ScenarioSubsystem.h"
#include "AgentNPCBase.generated.h"

class UBoxComponent;
class UAudioComponent;
class UWidgetComponent;
class USoundWave;
class UScenarioSubsystem;
class UAgentEmojiUI;


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

	// 자식 블루프린트에서 추가한 실제 메타휴먼 Face 컴포넌트 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|MetaHuman")
	FName FaceComponentName = TEXT("Face");

	// 이름이 바뀌는 경우를 대비한 Face 컴포넌트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|MetaHuman")
	FName FaceComponentTag = TEXT("Face");
	
	// [이모지] Agent의 감정을 더 정확히 표현 할 이모지 위젯 컴포넌트 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|MetaHuman")
	TObjectPtr<UWidgetComponent> EmojiComp;
	UPROPERTY()
	TObjectPtr<UAgentEmojiUI> EmojiUI;
	
	// AI 응답 대기 중 재생할 타이핑 사운드 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
	TObjectPtr<UAudioComponent> TypingAudioComp;
	
	// AI 서버 응답을 대기하며 타이핑 애니메이션을 재생해야 하는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bIsWaitingForAIResponse = false;

public:
	// 애니메이션 블루프린트에서 상태를 가져가기 위한 Getter
	UFUNCTION(BlueprintPure, Category = "AI|State")
	bool IsWaitingForAIResponse() const { return bIsWaitingForAIResponse; }
	
	void ForShortAnswer();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> PassportSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> TypingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sound")
	TObjectPtr<USoundBase> ForShortAnswerSound;
	
	// 감정별 이모지 텍스처를 매핑해두는 딕셔너리 (블루프린트에서 할당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Emoji")
	TMap<EAgentEmotion, TObjectPtr<UTexture2D>> EmotionTextures;
	
	// 감정별로 재생할 일회성 행동(몽타주)을 매핑해두는 딕셔너리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Animation")
	TMap<EAgentEmotion, TObjectPtr<UAnimMontage>> EmotionMontages;
	
	// 감정에따른 표정 변화
	UFUNCTION(BlueprintImplementableEvent, Category="AI|Emotion")
	void OnFaceEmotionChanged(FName EmotionKeyword);
	
	
protected:
	// ABP와 Emoji에서 사용할 현재 감정 상태 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EAgentEmotion CurrentEmotion = EAgentEmotion::Normal;
	
	// 파싱된 감정 상태를 애니메이션 블루프린트로 전달 
	void UpdateEmotion(EAgentEmotion EmotionLevel);
	
	// 서버에서 온 감정 문자열("Smile" 등)을 EAgentEmotion Enum으로 변환하는 헬퍼 함수
	EAgentEmotion ConvertStringToEmotion(const FString& EmotionString);

	// 자식 블루프린트에 추가된 실제 Face SkeletalMeshComponent를 찾습니다.
	USkeletalMeshComponent* ResolveFaceMeshComponent() const;

	UFUNCTION()
	void OnVoiceEnvelopeValue(const USoundWave* PlayingSoundWave, const float EnvelopeValue);

	void SetFloatPropertyIfExists(UObject* TargetObject, FName PropertyName, float Value) const;
	
private:
	// === InteractionBox Overlap Event ===
	UFUNCTION()
	virtual void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() 
	virtual void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:
	// === NPC Info ==
	// NPC 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Info")
	FName NPCName;
	
	// NPC의 역할 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Info")
	EScenarioType NPCScenarioType = EScenarioType::None;
	
	// NPC가 말을 먼저 거는 사람인지 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Info")
	bool bIsTalkingFirst = true;	
	
	// 누군가 이미 대화 중인지 상태를 저장 (서버 -> 클라 동기화)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="AI|Chat")
	bool bIsTalkingWithPlayer = false;

public:
	// === Conversation ===
	UFUNCTION(BlueprintPure, Category="AI|Chat")
	FString GetNPCName() const { return NPCName.ToString(); }
	
	UFUNCTION(BlueprintPure, Category="AI|Chat")
	bool CanTalkWithPlayer() const { return !bIsTalkingWithPlayer; }
	
	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	bool TryStartConversation();
	
	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	void EndConversation();

protected:
	// === Quest === 
	// Quest ID (NPC_ImmigrationOfficer, NPC_ServiceDesk, NPC_CustomsOfficer ... 상속받아 만들어진 액터에게 부여)
	FName QuestTargetID;
	
	// 시나리오 완료 여부 추적 플래그
	bool bIsScenarioCompleted = false;
	
public:
	UFUNCTION(BlueprintCallable, Category="AI|Info")
	FName GetQuestTargetID() { return QuestTargetID; }
	
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
	// NPC음성 재생 끝났을 때 호출 될 함수
	UFUNCTION() 
	void OnVoiceFinished();
	
	// 플레이어가 녹음을 완료해 전송했을 때 타이머를 끄기 위해 컨트롤러가 호출할 함수
	void NotifyPlayerSpoke();
	
	// PlayerController가 서버 응답 수신 후 NPC에게 결과를 전달하는 함수 
	void ProcessDialogueResponse(const FAIResponseData& ResponseData);
	
	// AudioURL로 HTTP GET 요청 시작
	void DownloadAndPlayAudio(const FString& AudioURL);
	
	// HTTP GET 응답 콜백 - WAV 바이너리를 받아 재생
	void OnAudioDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
	
public:
	// ==========================================
	// [추가] 시선 처리(Look-At IK)를 위한 변수들
	// ==========================================
    
	// 지금 플레이어를 쳐다봐야 하는 상태인지 여부 (스위치 역할)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|IK")
	bool bIsLookingAtPlayer = false;

	// 플레이어의 위치 (주로 얼굴/카메라 좌표)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|IK")
	FVector TargetLookAtLocation = FVector::ZeroVector;
    
	// 플레이어 캐릭터 포인터 캐싱용 (Tick에서 위치를 계속 업데이트하기 위함)
	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractPlayer = nullptr;


	// 애니메이션 블루프린트(ABP)가 실제로 읽어갈 최종 스위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|IK")
	bool bEnableIK = false;
	
};
