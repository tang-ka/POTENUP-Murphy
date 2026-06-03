
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IHttpRequest.h"
#include "AgentNPCBase.generated.h"

class UBoxComponent;
class UAudioComponent;
class UWidgetComponent;

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
	
private:
	// === InteractionBox Overlap Event ===
	UFUNCTION()
	void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// todo: Enum 처리
	// 파싱된 감정 상태를 애니메이션 블루프린트로 전달 
	void UpdateEmotion(int32 EmotionLevel);
	
protected:
	// 누군가 이미 대화 중인지 상태를 저장 (서버 -> 클라 동기화)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="AI|Chat")
	bool bIsTalkingWithPlayer = false;
	
public:
	// === 독점 대화 로직 ===
	UFUNCTION(BlueprintPure, Category="AI|Chat")
	bool CanTalkWithPlayer() const { return !bIsTalkingWithPlayer; }

	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	bool TryStartConversation();

	UFUNCTION(BlueprintCallable, Category="AI|Chat")
	void EndConversation();
	
public:
	// PlayerController가 서버 응답 수신 후 NPC에게 결과를 전달하는 함수 
	void ProcessDialogueResponse(const FString& Dialogue, int32 EmotionLevel, const FString& AudioURL);
	
	// AudioURL로 HTTP GET 요청 시작
	void DownloadAndPlayAudio(const FString& AudioURL);
	
	// HTTP GET 응답 콜백 - WAV 바이너리를 받아 재생
	void OnAudioDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};