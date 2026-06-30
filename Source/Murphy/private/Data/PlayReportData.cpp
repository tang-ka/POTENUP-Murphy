// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/PlayReportData.h"

FPlayReportData FPlayReportData::FromAIResult(const FAIResultResponse& InResult)
{
	FPlayReportData OutData;

	// === 1. 점수 및 판정 데이터 ===
	const FAIFinalResult& FR = InResult.final_result;

	OutData.TierName            = FR.tier;
	OutData.TotalScore          = FR.final_score_100;
	OutData.FinalRecommendation = FR.final_recommendation;
	OutData.Rank                = FR.rank;

	// Iron 티어 = FAIL 처리 (DataTable 키 "Iron"과 동일)
	OutData.bIsGameClear = !FR.tier.Equals(TEXT("Iron"), ESearchCase::IgnoreCase);

	// === 2. 세부 점수 ===
	const FAIQuantitativeScores& QS = FR.quantitative_scores;

	OutData.ComprehensionScore  = QS.comprehension;
	OutData.FluencyScore        = QS.fluency;
	OutData.GrammarScore        = QS.grammar_accuracy;
	OutData.VocabularyScore     = QS.vocabulary_range;
	OutData.ClarityScore        = QS.clarity;
	OutData.ProblemSolvingScore = QS.interaction_problem_solving;

	// === 3. 리포트 요약 ===
	const FAIReportSummary& RS = FR.report_summary;

	OutData.OverallSummary  = RS.overall;
	OutData.MainImprovement = RS.main_improvement;
	OutData.BestNode        = RS.best_node;
	OutData.WeakestNode     = RS.weakest_node;

	// === 4. 게임 밖 피드백 ===
	const FAIOutGameFeedback& OGF = InResult.out_game_feedback;

	OutData.OverallSummaryKr     = OGF.overall_summary_kr;
	OutData.NextPracticePromptKr = OGF.personalized_next_step.practice_prompt_kr;
	OutData.NextAnswerExample    = OGF.personalized_next_step.answer_example;

	// === 5. 피드백 카드 배열 변환 ===
	OutData.FeedbackCards.Reserve(OGF.focus_on_form_items.Num());

	for (const FAIFocusOnFormItem& Item : OGF.focus_on_form_items)
	{
		FPlayReportFeedbackCardData Card;
		Card.Title                = Item.title_kr;
		Card.Summary              = Item.rule_summary_kr;
		Card.OriginalUtterances   = Item.original_utterances;
		Card.SuggestedExpressions = Item.suggested_expressions;
		Card.PracticePrompt       = Item.practice_prompt_kr;
		Card.AnswerExample        = Item.answer_example;
		Card.Priority             = Item.priority;

		OutData.FeedbackCards.Add(MoveTemp(Card));
	}

	return OutData;
}
