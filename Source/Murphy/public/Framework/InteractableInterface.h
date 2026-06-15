#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class AMurphyPlayer;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 액터들이 구현해야 할 인터페이스
 */
class MURPHY_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 플레이어가 F키로 상호작용할 때 호출됩니다.
	 * @param Player - 상호작용을 시도한 플레이어
	 */
	virtual void Interact(AMurphyPlayer* Player) = 0;
};
