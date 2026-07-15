#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/MurphyPlayer.h"
#include "STTTestPlayer.generated.h"

/**
 * 기존 BP_STTestPlayer 호환용 래퍼 클래스.
 * Realtime STT 기능은 AMurphyPlayer로 통합되었습니다.
 */
UCLASS()
class MURPHY_API ASTTTestPlayer : public AMurphyPlayer
{
	GENERATED_BODY()

public:
	ASTTTestPlayer();
};
