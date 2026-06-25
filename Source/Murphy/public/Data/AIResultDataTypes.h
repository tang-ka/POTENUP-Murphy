#pragma once

#include "CoreMinimal.h"
#include "AIResultDataTypes.generated.h"

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
