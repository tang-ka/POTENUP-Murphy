// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CinematicOverlayWidget.generated.h"

class UImage;
class UMediaTexture;

/**
 * 풀스크린 커버 위젯.
 * - BlackImage : 검정 레이어 (낮은 ZOrder, FadeColor로 채움)
 * - MediaImage : 미디어 레이어 (높은 ZOrder, MediaTexture 바인딩)
 *
 * WBP_CinematicOverlay 에서 두 Image를 화면 가득 채우고 BindWidget 이름을 맞출 것.
 * MediaImage 가 BlackImage 위에 오도록 배치 (게임 프레임 누출 방지).
 *
 * NOTE: 이 위젯은 World 소속이라 OpenLevel(하드 트래블)을 관통하지 못한다.
 *       맵 전환 커버는 추후 MoviePlayer 레이어로 별도 처리(1차 범위 밖).
 */
UCLASS(Abstract, Blueprintable)
class MURPHY_API UCinematicOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 검정 레이어 알파 (0=투명, 1=불투명). 색은 SetFadeColor로 지정한 값을 사용. */
	void SetBlackOpacity(float Alpha);

	/** 미디어 레이어 알파 (0=투명, 1=불투명). */
	void SetMediaOpacity(float Alpha);

	/** 검정 레이어 + 레터박스 배경색. */
	void SetFadeColor(const FLinearColor& InColor);

	/** MediaImage 브러시에 MediaTexture 바인딩. nullptr이면 미디어 레이어 비움. */
	void SetMediaTexture(UMediaTexture* MediaTexture);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Black;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Media;

private:
	// 검정 레이어에 적용할 색(rgb). 알파는 SetBlackOpacity로 분리 제어.
	FLinearColor FadeColor = FLinearColor::Black;
	float BlackAlpha = 0.f;
};
