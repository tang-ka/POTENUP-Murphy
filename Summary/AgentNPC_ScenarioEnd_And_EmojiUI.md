# Agent NPC 시나리오 종료 로직 및 이모지 UI 버그 수정

## 조건
- **작업 개요**: AI Agent 서버와의 통신을 통해 진행되는 입국 심사 시나리오에서, AI의 응답에 맞춰 적절한 시점에 시나리오를 종료하고 다음 시나리오로 넘어가도록(`ScenarioSubsystem` 연동) 처리했습니다. 또한, 이 과정에서 발생한 이모지 UI(`EmojiComp`)의 표시 버그를 해결하여 NPC 이름이 항상 표시되고, 렌더 스케일이 거리에 따라 조절되며, 상호작용 시 이모지가 정상적으로 켜지고 꺼지도록 수정했습니다.

- **주요 변경 파일**:
  - `Source/Murphy/public/Actors/Characters/AgentNPCBase.h` (수정)
  - `Source/Murphy/private/Actors/Characters/AgentNPCBase.cpp` (수정)

- **변경 상세 내용**:

| 파일명 | 수정 전 (Before) | 수정 후 (After) | 추가 목적 / 의도 (Purpose) |
|---|---|---|---|
| `AgentNPCBase.h` | 1분 타이머 관련 변수만 존재, 시나리오 상태 추적 불가 | `bIsScenarioCompleted` 플래그 추가 | AI 응답으로부터 받은 종료 상태를 오디오 재생 완료 시점까지 지연 캐싱 |
| `AgentNPCBase.cpp` | `ProcessDialogueResponse`에서 대사/이모지 연출만 처리 | `FAIL_END`, `FINAL_DECISION`, `IMM_006_DECLARATION_CHECK` 수신 시 `bIsScenarioCompleted = true` 세팅 | 시나리오 종료를 판단할 수 있는 분기 로직 추가 |
| `AgentNPCBase.cpp` | `OnVoiceFinished`에서 무조건 1분 타이머 시작 | 시나리오 종료 상태일 경우 타이머 시작을 방지하고 `ScenarioSubsystem->EndScenario(true)` 호출 | 음성이 완전히 끝난 자연스러운 시점에 멀티플레이어 시나리오 동기화 진행 |
| `AgentNPCBase.cpp` | 컴포넌트 전체(`EmojiComp`)의 `Visibility`를 끄고 켬 | `EmojiComp`는 항상 `Visible`로 두고, 캐싱된 내부 위젯인 `EmojiUI->SetEmojiVisible()` 활용 | 이름표는 항상 유지하고, 상황에 맞게 이모지/게이지만 켜고 끌 수 있도록 개선 |
| `AgentNPCBase.cpp` | `Tick`에 위젯 스케일 조절 로직 부재 | `EWidgetSpace::Screen`일 때 플레이어 카메라 거리에 비례하여 `EmojiUI->SetRenderScale()` 조절 | 거리가 멀어지면 UI가 화면을 과도하게 가리는 현상 방지 |

- **트러블슈팅**:
  - **이슈**: NPC에 다가가도 이름표나 이모지 UI(`EmojiComp`)가 아예 보이지 않는 버그 발생.
  - **원인**:
    1. 과거 C++ 생성자에서 설정했던 `EmojiComp->SetVisibility(false)` 상태가 블루프린트 파일(CDO)에 직렬화되어 굳어 있었음.
    2. 기존 대화 종료 로직에서 컴포넌트 전체를 꺼버림.
  - **해결**:
    1. C++ 생성자의 `SetVisibility(false)`를 삭제.
    2. `BeginPlay`에서 블루프린트 설정을 덮어쓰도록 `EmojiComp->SetVisibility(true)` 강제 호출 및 `InitWidget()` 방어 코드 추가.
    3. `EmojiUI->SetEmojiVisible()` 함수를 통해 컴포넌트 전체가 아닌 "이모지와 게이지"만 숨기도록 분리 처리.

- **테스트 방법**:
  1. 에디터 빌드 후 레벨에서 NPC에게 접근하여 입국 심사 시작 (NPC의 이름표가 항상 노출되는지 확인)
  2. 오버랩 시 이모지가 정상적으로 팝업되며, 카메라 거리에 따라 `EmojiUI`의 크기가 자연스럽게 스케일링(`SetRenderScale`) 되는지 확인
  3. 시나리오 완료 대사("You have a return ticket...") 음성이 끝난 **직후** 1분 타이머가 돌지 않고 상호작용 박스가 비활성화되며, `ScenarioSubsystem`이 다음 시나리오로 정상 진행되는지 확인.

- **TODO**:
  - AI 백엔드 팀과 조율하여 임시로 적용된 로그 기반 종료 노드 조건(`IMM_006_DECLARATION_CHECK`) 대신, 완벽한 통신 규약 기반의 `FINAL_DECISION` 처리로 통일성 확보.
  - 이모지 파싱 시 현재 0~6 랜덤 난수로 하드코딩되어 있는 부분을 AI `Tone` 키워드 연동 로직으로 복구.
  - `EWidgetSpace::World` 모드 사용 시 활성화할 빌보딩 로직(카메라 바라보기) 검토 및 적용.
