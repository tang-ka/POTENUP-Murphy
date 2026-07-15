#pragma once

#include "CoreMinimal.h"
#include "Framework/MurphyPlayerController.h"
#include "STTPlayerController.generated.h"

/**
 * 기존 BP_STTTestPC 호환용 래퍼 클래스.
 * Realtime STT 요청/응답 처리는 AMurphyPlayerController로 통합되었습니다.
 */
UCLASS()
class MURPHY_API ASTTPlayerController : public AMurphyPlayerController
{
	GENERATED_BODY()

public:
	ASTTPlayerController();
};
