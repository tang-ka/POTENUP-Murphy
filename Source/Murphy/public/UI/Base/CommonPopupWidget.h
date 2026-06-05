// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonPopupWidget.generated.h"

class UButton;
class UTextBlock;
class UHorizontalBox;

UENUM(BlueprintType)
enum class EPopupButtons : uint8
{
    None        UMETA(DisplayName = "No Buttons"),   // 버튼 없음
    OkayOnly    UMETA(DisplayName = "Okay Only"),    // 확인 버튼만
    OkayCancel  UMETA(DisplayName = "Okay & Cancel") // 확인 + 취소
};

USTRUCT(BlueprintType)
struct FUIPopupDesc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPopupButtons Buttons = EPopupButtons::OkayOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OkayText = INVTEXT("확인");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText CancelText = INVTEXT("취소");

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeTime = 0.f; // 0 = 수동 닫기, >0 = 자동 소멸

    // 버튼 콜백 (BP 노출 불가, C++ 호출부에서 바인딩)
    FSimpleDelegate OnOkay;
    FSimpleDelegate OnCancel;
};

UCLASS(Abstract)
class MURPHY_API UCommonPopupWidget : public UUserWidget
{
    GENERATED_BODY()
    
protected:
    virtual void NativeDestruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Title;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Txt_Content;

    // 버튼 전체를 담는 컨테이너 (None일 때 통째로 Collapsed)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> Box_Buttons;

#pragma region Okay Button
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Okay;
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_OkayLabel;
#pragma endregion
    
#pragma region Cancel Button
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> Btn_Cancel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Txt_CancelLabel;
#pragma endregion

public:
    void Setup(const FUIPopupDesc& InDesc);

protected:
    UFUNCTION()
    void HandleOkay();

    UFUNCTION()
    void HandleCancel();

    // 버튼 구성에 따라 가시성 토글
    void ApplyButtonLayout(EPopupButtons InButtons);

private:
    FUIPopupDesc Desc;
    FTimerHandle LifeTimeHandle;
};