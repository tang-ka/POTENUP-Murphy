
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AIDataTypes.generated.h"

class UAnimMontage;
class UTexture2D;

// ===  AI Enums ===

UENUM(BlueprintType)
enum class EAgentEmotion : uint8
{
	Normal,
	Joy,
	Anger,
	Sadness,
	Panic,
	Suspicion,
	Disgust,
	Fear,
	Smirk,
	Surprise,
	Pain,
	Confusion,
	Boredom
};

USTRUCT(BlueprintType)
struct FAI_EmotionData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Emotion", meta=(ToolTip="감정 이름"))
	EAgentEmotion EmotionName = EAgentEmotion::Normal;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Emotion", meta=(ToolTip="얼굴 근육 맵"))
	TMap<FName, float> MuscleValues;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Emotion", meta=(ToolTip="이모지"))
	TSoftObjectPtr<UTexture2D> EmotionTextures;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Emotion", meta=(ToolTip="애니메이션 몽타주"))
	TArray<TSoftObjectPtr<UAnimMontage>> EmotionMontages;
	
};

//  === [Unreal -> AI] Request Structs ===

USTRUCT(BlueprintType)
struct FAI_SessionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="플레이 세션 식별"))
	FString session_id;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="플레이어 식별"))
	FString player_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="챕터 OpenKB/노드 로딩"))
	FString chapter_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="현재 Unreal scene 식별"))
	FString scene_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="현재 대화 노드"))
	FString current_node_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="턴 순서, 로그/리포트용"))
	int32 turn_index = 0;
};

USTRUCT(BlueprintType)
struct FAI_NPCRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC 식별"))
	FString npc_id;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC 역할/tone 결정"))
	FString npc_role;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="플레이어 답변이 어떤 질문에 대한 답인지 판단"))
	FString last_npc_message;
};

USTRUCT(BlueprintType)
struct FAI_AudioMeta
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="wav 검증"))
	FString mime_type = TEXT("audio/wav");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT/audio 처리 참고"))
	int32 sample_rate_hz = 16000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="mono/stereo 정보"))
	int32 channels = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="너무 짧거나 긴 입력 판단"))
	int32 duration_ms = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT 언어 힌트"))
	FString language_hint = TEXT("en-US");
};

USTRUCT(BlueprintType)
struct FAI_PlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="개인화 가능"))
	FString nickname;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="피드백 강도 참고"))
	FString english_confidence;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="난이도/힌트 빈도 결정"))
	FString tier;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="여행 영어 레벨 정책"))
	FString travel_speaking_level;
};

USTRUCT(BlueprintType)
struct FAI_ScenarioState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC/상황 인내도"))
	int32 patience = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="입국 심사 위험도"))
	int32 suspicion = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="재질문/힌트/실패 분기"))
	int32 retry_count = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="힌트 남발 방지"))
	int32 hint_count = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="누적 실패 판단"))
	int32 previous_fail_count = 0;
};

USTRUCT(BlueprintType)
struct FAI_GameState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="소지품 기반 분기/검증"))
	TArray<FString> inventory;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal gameplay flag"))
	TArray<FString> flags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="완료한 행동"))
	TArray<FString> completed_intents;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="현재 UI 목표"))
	FString current_objective;
};

USTRUCT(BlueprintType)
struct FAI_PreviousNodeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString node_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString verdict;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString next_action;
};

USTRUCT(BlueprintType)
struct FAI_InteractionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="상호작용 시작 주체: 'player' 또는 'npc'"))
	FString initiator = TEXT("player");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="상호작용 타입: 'quest', 'ambient', 'tutorial', 'system'"))
	FString interaction_type = TEXT("quest");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="제한 시간 (초), 1 이상이어야 함"))
	int32 time_limit_s = 15;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="첫 만남 여부"))
	bool first_contact = false;

	// --- [안전한 선행 작업] Agent의 Pydantic이 업데이트되지 않아도 무시되어 에러가 나지 않는 새 변수들 ---
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="타임아웃 발생 여부"))
	bool is_timeout = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="시스템 알림 이벤트 (예: scenario_start, scenario_end)"))
	FString system_event;
};

USTRUCT(BlueprintType)
struct FAI_ClientContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="platform 정보"))
	FString platform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="input_device 정보"))
	FString input_device;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="locale 정보"))
	FString locale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="build_version 정보"))
	FString build_version;
};

USTRUCT(BlueprintType)
struct FAIRequestData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="요청 schema 버전 확인"))
	FString contract_version = TEXT("dev_c_unreal_turn.v1");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="한 턴 추적용 id. 디버깅/log 연결 키"))
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="세션, 챕터, 현재 노드 식별"))
	FAI_SessionRequest session;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="직전 NPC 질문/역할 맥락"))
	FAI_NPCRequest npc;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="업로드 wav의 metadata"))
	FAI_AudioMeta audio;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="상호작용 관련 설정 및 시스템 이벤트 알림"))
	FAI_InteractionContext interaction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="난이도, 힌트, 피드백 조절"))
	FAI_PlayerProfile player_profile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="patience/suspicion/retry 등 게임 상태"))
	FAI_ScenarioState scenario_state;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="inventory, flags, objective 등 Unreal 상태"))
	FAI_GameState game_state;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="이전 노드 결과, final report/분기 참고"))
	TArray<FAI_PreviousNodeResult> previous_node_results;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal 측에서 허용하는 다음 노드 guard"))
	TArray<FString> client_allowed_next_nodes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="platform/build/debug 정보"))
	FAI_ClientContext client_context;
};

// === [AI -> Unreal] Response Structs ===

USTRUCT(BlueprintType)
struct FAI_STTResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="사용한 STT 모델명"))
	FString model;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="기본 STT runtime"))
	FString primary_runtime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="fallback runtime"))
	FString fallback_runtime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="실제로 transcript를 만든 runtime"))
	FString runtime_used;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="AI가 이해/평가에 사용한 최종 transcript"))
	FString player_text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT 신뢰도"))
	float confidence = 0.0f; 

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="감지/힌트 언어"))
	FString language_detected;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="다시 말하게 해야 하는지"))
	bool needs_repeat = false;
};


USTRUCT(BlueprintType)
struct FAI_NPCResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="말하는 NPC"))
	FString speaker;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="화면 subtitle/dialogue"))
	FString text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="음성/애니메이션 tone"))
	FString tone;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal animation hint"))
	FString animation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC 음성 wav 접근 경로"))
	FString audio_url;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC 음성 wav 접근 경로"))
	FString emotion;
};

USTRUCT(BlueprintType)
struct FAI_InGameFeedback
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="피드백 UI 표시 여부"))
	bool show = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="피드백 방식"))
	FString feedback_strategy;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="UI 우선순위"))
	FString priority;
};

USTRUCT(BlueprintType)
struct FAI_UIFeedback
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="힌트 UI 표시 여부"))
	bool show_hint = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="한국어 힌트 문구"))
	FString hint_kr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="추천 영어 표현"))
	FString recommended_expression;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FAI_InGameFeedback in_game_feedback;
};

USTRUCT(BlueprintType)
struct FAI_StateDelta
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="patience 변화량"))
	int32 patience_delta = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="suspicion 변화량"))
	int32 suspicion_delta = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="retry_count 변화량"))
	int32 retry_count_delta = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="hint_count 변화량"))
	int32 hint_count_delta = 0;
};

//  억까 물건 데이터용 하위 구조체 생성
USTRUCT(BlueprintType)
struct FAI_RandomCustomsItem_Data
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString item_id;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString suspicion_reason;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FString item_name;
};

USTRUCT(BlueprintType)
struct FAI_CustomsUI_Data
{
	GENERATED_BODY()

	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="배정된 방문 장소 ID (예: LOC_DOWNTOWN_HOTEL)"))
	// FString assigned_visit_location;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="배정된 방문 장소 ID (예: LOC_DOWNTOWN_HOTEL)"))
	FString assigned_visit_location_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="해당 장소를 의심하는 이유 (심사관 대사 생성 참고용)"))
	FString visit_location_suspicion_reason;

	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="랜덤하게 소지한 밀수/의심 물품 ID (예: ITM_SUSPICIOUS_WATCH)"))
	// FString random_customs_item_id;
	
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="해당 물품을 의심하는 이유 (심사관 대사 생성 참고용)"))
	// FString random_customs_item_suspicion_reason;
	
	// ✅ 중요: 변수명을 백엔드 JSON 키와 동일하게 'random_customs_item'으로 맞춥니다!
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FAI_RandomCustomsItem_Data random_customs_item; 
};

USTRUCT(BlueprintType)
struct FAI_EvaluationScores
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="과업 성공 점수"))
	int32 task_success = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="명확성"))
	int32 clarity = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="문법"))
	int32 grammar = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="어휘"))
	int32 vocabulary = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="문제 해결"))
	int32 problem_solving = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="공손함"))
	int32 politeness = 0;
};

USTRUCT(BlueprintType)
struct FAI_Evaluation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="답변 판정"))
	FString verdict;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="세부 점수"))
	FAI_EvaluationScores scores;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="평가 태그"))
	TArray<FString> feedback_tags;
};

USTRUCT(BlueprintType)
struct FAI_ReportItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="최종 리포트용 요약"))
	FString summary;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="개선점"))
	FString improvement;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="예시 답변"))
	FString example_answer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="리포트 태그"))
	TArray<FString> score_tags;
};

USTRUCT(BlueprintType)
struct FAI_Report
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="기록된 학습 오류 수"))
	int32 recorded_error_count = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication")
	FAI_ReportItem report_item;
};

USTRUCT(BlueprintType)
struct FAI_Debug
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT 디버깅"))
	FString stt_model;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT 신뢰도 확인"))
	float stt_confidence = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Understanding Agent 신뢰도"))
	float understanding_confidence = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="계약 버전 추적"))
	TArray<FString> contract_versions;
};

USTRUCT(BlueprintType)
struct FAIResponseData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="응답 schema 버전"))
	FString contract_version;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="요청 - 응답 매칭"))
	FString request_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="세션 매칭"))
	FString session_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="턴 매칭"))
	int32 turn_index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="처리된 현재 노드"))
	FString current_node_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal이 이동할 다음 노드"))
	FString next_node_id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal이 실행할 대화/게임 action"))
	FString next_action;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="STT 결과와 디버그 정보"))
	FAI_STTResponse stt;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="NPC 대사, tone, animation, audio"))
	FAI_NPCResponse npc;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="힌트/피드백 UI 표시 정보"))
	FAI_UIFeedback ui;
	
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="입국심사서 UI 갱신용 데이터"))
	// FAI_CustomsUI_Data customs_data;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="입국심사서 UI 갱신용 데이터"))
	FAI_CustomsUI_Data game_state;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="Unreal state에 적용할 변화량"))
	FAI_StateDelta state_delta;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="플레이어 답변 평가"))
	FAI_Evaluation evaluation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="추후 final report용 기록"))
	FAI_Report report;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AI Communication", meta=(ToolTip="개발/디버깅 정보"))
	FAI_Debug debug;
	
	
};

// === [AI -> Unreal] 최종 결과 조회 응답 구조체 ===

USTRUCT(BlueprintType)
struct MURPHY_API FAIQuantitativeScores
{
	GENERATED_BODY()

	// 전체 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 overall = 0;

	// 이해도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 comprehension = 0;

	// 유창성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 fluency = 0;

	// 문법 정확도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 grammar_accuracy = 0;

	// 어휘 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 vocabulary_range = 0;

	// 명확성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 clarity = 0;

	// 상호작용 문제 해결력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 interaction_problem_solving = 0;

	// 서버 점수 산정 정책
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString scoring_policy;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIReportSummary
{
	GENERATED_BODY()

	// 전체 총평
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString overall;

	// 가장 잘 수행한 노드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString best_node;

	// 가장 약했던 노드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString weakest_node;

	// 핵심 개선 가이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString main_improvement;

	// 복습 대상 표현 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FString> focus_on_form_targets;

	// 점수에 포함된 노드 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 included_node_count = 0;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIFinalResult
{
	GENERATED_BODY()

	// 최종 추천 결과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString final_recommendation;

	// 표시용 랭크 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString rank;

	// Gold/Silver/Bronze 티어
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString tier;

	// 100점 만점 최종 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	int32 final_score_100 = 0;

	// 점수 산정 사유 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FString> reason_tags;

	// 세부 정량 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FAIQuantitativeScores quantitative_scores;

	// 최종 리포트 요약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FAIReportSummary report_summary;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIFocusOnFormItem
{
	GENERATED_BODY()

	// 교정 대상 표현 그룹
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString focus_on_form_target;

	// 카드 제목
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString title_kr;

	// 한글 핵심 규칙 요약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString rule_summary_kr;

	// 유저가 실제로 말한 표현
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FString> original_utterances;

	// 추천 영어 표현
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FString> suggested_expressions;

	// 복습용 한글 프롬프트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString practice_prompt_kr;

	// 복습 모범 답안
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString answer_example;

	// 중요도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString priority;

	// 근거가 된 노드 ID 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FString> source_node_ids;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIPersonalizedNextStep
{
	GENERATED_BODY()

	// 다음 플레이에서 연습할 대상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString target;

	// 다음 플레이 추천 연습 문제
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString practice_prompt_kr;

	// 추천 연습 문제의 모범 답안
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString answer_example;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIOutGameFeedback
{
	GENERATED_BODY()

	// 리포트 모드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString report_mode;

	// 게임 밖 피드백 한글 총평
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString overall_summary_kr;

	// 표현/문법 교정 카드 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	TArray<FAIFocusOnFormItem> focus_on_form_items;

	// 다음 플레이 추천 학습 가이드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FAIPersonalizedNextStep personalized_next_step;
};

USTRUCT(BlueprintType)
struct MURPHY_API FAIResultResponse
{
	GENERATED_BODY()

	// 결과 응답 스키마 버전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString contract_version;

	// 결과를 조회한 AI 세션 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FString session_id;

	// 최종 판정과 점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FAIFinalResult final_result;

	// 게임 밖 학습 피드백
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Murphy|AIResult")
	FAIOutGameFeedback out_game_feedback;
};

