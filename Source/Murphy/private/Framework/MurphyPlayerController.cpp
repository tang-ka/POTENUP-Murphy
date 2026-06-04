
#include "Framework/MurphyPlayerController.h"

#include "Actors/Characters/MurphyPlayer.h"
#include "Actors/Characters/AgentNPCBase.h"

#include "VoiceChat/VoiceRecorderComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"

#include "Murphy.h"
#include "Manager/LevelStreamingSubsystem.h"
#include "Manager/NetSubsystem.h"
#include "Manager/ScenarioSubsystem.h"


void AMurphyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AMurphyPlayer* MainPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (IsValid(MainPlayer->GetVoiceRecorderComp()))
		{
			MainPlayer->GetVoiceRecorderComp()->OnRecordingFinished.AddDynamic(this, &AMurphyPlayerController::OnAudioRecordingFinished);
			PRINTLOG_JW(TEXT("OnRecordingFinished 바인딩 완료"));
		}
	}
}

// ==============================================================================
// === 테스트 커맨드 ===
// ==============================================================================
void AMurphyPlayerController::Test_StartScenario(int32 ScenarioIndex)
{
	if (UScenarioSubsystem* ScenarioSubsys = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
	{
		ScenarioSubsys->StartScenario(static_cast<EScenarioType>(ScenarioIndex));
		PRINTLOGW_JW(TEXT("[Test] 시나리오 강제 시작: 인덱스 %d"), ScenarioIndex);
	}
}

void AMurphyPlayerController::Test_EndScenarioAndTravel(FName NextLevelKey)
{
	if (UScenarioSubsystem* ScenarioSubsystem = GetGameInstance()->GetSubsystem<UScenarioSubsystem>())
	{
		ScenarioSubsystem->EndScenario(true);
		PRINTLOGW_JW(TEXT("[Test] 시나리오 성공 처리 완료"));
	}
	
	if (ULevelStreamingSubsystem* LevelSubsystem = GetGameInstance()->GetSubsystem<ULevelStreamingSubsystem>())
	{
		PRINTLOGW_JW(TEXT("[Test] 다음 맵으로 서버 트래블 시도: %s"), *NextLevelKey.ToString());
		LevelSubsystem->TravelAllPlayers(NextLevelKey);
	}
}

void AMurphyPlayerController::Test_SendAIMessage(const FString& Message)
{
	if (UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>())
	{
		// 임시로 빈 델리게이트 전달 (로그 출력 테스트용)
		FOnAIResponseReceived DummyCallback;
		NetSubsystem->SendMessageToAI(Message, DummyCallback);
		PRINTLOGW_JW(TEXT("[Test] NetSubsystem을 통한 더미 메시지 전송 명령: %s"), *Message);
	}
}

void AMurphyPlayerController::Test_SimulateAIResponse(const FString& SimulatedJSONResponse)
{
	
	// 실제 파이썬 서버가 켜져있지 않을 때 NPC의 대화 처리 로직을 강제로 테스트하기 위함
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(SimulatedJSONResponse);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid() && IsValid(TargetNPC))
	{
		FString Dialogue  = JsonObj->GetStringField(TEXT("npc_dialogue"));
		int32 EmotionLevel = JsonObj->GetIntegerField(TEXT("npc_emotion_level"));
		FString AudioURL  = JsonObj->GetStringField(TEXT("npc_audio_url"));
		
		TargetNPC->ProcessDialogueResponse(Dialogue, EmotionLevel, AudioURL);
		
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			MurphyPlayer->EndChatWithNPC();
		}
		PRINTLOGW_JW(TEXT("[Test] 가짜 응답 시뮬레이션 및 NPC 점유 해제 완료!"));
	}
	else
	{
		PRINTLOGE_JW(TEXT("[Test] 시뮬레이션 실패! JSON 문법이 틀렸거나 가까운 곳에 타겟 NPC가 없습니다."));
	}
}

void AMurphyPlayerController::SetActiveNPC(AAgentNPCBase* NewNPC)
{
	TargetNPC = NewNPC;
}

void AMurphyPlayerController::OnAudioRecordingFinished(const FString& SavedFilePath)
{
	if (!IsValid(TargetNPC)) return;
	
	//! Net ~ 넘김
	// SendVoiceFileToServer(SavedFilePath);  // multipart 방식 (wav 파일 그대로 넘기기)
	// SendVoiceDataAsJsonBase64(SavedFilePath); // Base64 JSON 방식 (Base64로 인코딩 후 넘기기)
	
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (MurphyPlayer->GetRecordTime() < 0.5f)
		{			
			PRINTLOGW_JW(TEXT("[Voice Test] 녹음 시간이 너무 짧습니다. AI 서버로 전송하지 않고 기본 응답을 처리합니다."));
			
			// todo : 하드코딩 부분
			FString SimulatedJSONResponse = TEXT("{\"npc_dialogue\":\"잘 못 들었어. 조금만 더 길게 말해줄래?\",\"npc_emotion_level\":1,\"npc_audio_url\":\"\"}");
			Test_SimulateAIResponse(SimulatedJSONResponse);
			return;
		}
	}
	
	if (UNetSubsystem* NetSubsystem = GetGameInstance()->GetSubsystem<UNetSubsystem>())
	{
		FOnAIResponseReceived Callback;
		Callback.BindDynamic(this, &AMurphyPlayerController::OnAIResponseReceived);
		
		PRINTLOGW_JW(TEXT("[Voice Test] NetSubsystem을 통해 서버로 오디오 전송 시작"));
		NetSubsystem->SendVoiceFileToAI(SavedFilePath, Callback);
	}
}

void AMurphyPlayerController::OnAIResponseReceived(const FString& ResponseData)
{
	// 에러 처리: 서버에서 빈 문자열이 오면 에러로 간주하고 대화 상태를 강제로 품
	if (ResponseData.IsEmpty())
	{
		PRINTLOGE_JW(TEXT("[Chat] AI 응답 실패. 대화 상태를 강제 초기화합니다."));
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			MurphyPlayer->EndChatWithNPC();
		}
		
		return;
	}
	
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseData);
	if  (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid() && IsValid(TargetNPC))
	{
		FString Dialogue = JsonObj->GetStringField(TEXT("npc_dialogue"));
		int32 EmotionLevel = JsonObj->GetIntegerField(TEXT("npc_emotion_level"));
		FString AudioURL = JsonObj->GetStringField(TEXT("npc_audio_url"));
		TargetNPC->ProcessDialogueResponse(Dialogue, EmotionLevel, AudioURL);
		
		// AI 응답이 도착해 대화가 끝나면 NPC점유 해제 및 상태 초기화
		if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
		{
			MurphyPlayer->EndChatWithNPC();
		}
	}
}

/*! NetSubsystem으로 넘김 
void AMurphyPlayerController::SendVoiceFileToServer(const FString& WAVFilePath)
{
    TArray<uint8> RawAudioData;
    if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("WAV 파일 로드 실패: %s"), *WAVFilePath);
        return;
    }
	
    // JSON 메타데이터 구성
    TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
    JsonObj->SetStringField(TEXT("session_id"), TEXT("sess_12345"));
    JsonObj->SetStringField(TEXT("user_id"), TEXT("player_01"));
    JsonObj->SetStringField(TEXT("current_scene"), TEXT("immigration"));
    JsonObj->SetStringField(TEXT("npc_current_emotion"), TEXT("Neutral"));
    JsonObj->SetBoolField(TEXT("is_timeout"), false);
    JsonObj->SetNumberField(TEXT("response_time_sec"), 10.0f);
	
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    
	// multipart/form-data 페이로드 조립
    const FString Boundary = TEXT("----UnrealBoundary") + FString::FromInt(FMath::Rand());
    TArray<uint8> Payload;
	auto AppendStr = [&](const FString& Str)
	{
		FTCHARToUTF8 Conv(*Str);
		Payload.Append((uint8*)Conv.Get(), Conv.Length());
	};
    
	AppendStr(FString::Printf(TEXT("\r\n--%s\r\nContent-Disposition: form-data; name=\"metadata\"\r\nContent-Type: application/json\r\n\r\n%s"), *Boundary, *JsonString));
    AppendStr(FString::Printf(TEXT("\r\n--%s\r\nContent-Disposition: form-data; name=\"audio_file\"; filename=\"user_voice.wav\"\r\nContent-Type: audio/wav\r\n\r\n"), *Boundary));
    Payload.Append(RawAudioData);
    AppendStr(FString::Printf(TEXT("\r\n--%s--\r\n"), *Boundary));
	
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("http://127.0.0.1:8001/api/chat"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
    Request->SetContent(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &AMurphyPlayerController::OnResponseReceived);
    Request->ProcessRequest();
}

void AMurphyPlayerController::SendVoiceDataAsJsonBase64(const FString& WAVFilePath)
{
	TArray<uint8> RawAudioData;
	if (!FFileHelper::LoadFileToArray(RawAudioData, *WAVFilePath))
	{
		PRINTLOGE_JW(TEXT("WAV 파일 로드 실패: %s"), *WAVFilePath);
		return;
	}
	
	// JSON 메타데이터 구성
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());
	JsonObj->SetStringField(TEXT("session_id"), TEXT("sess_12345"));
	JsonObj->SetStringField(TEXT("user_id"), TEXT("player_01"));
	JsonObj->SetStringField(TEXT("user_tier"), TEXT("Bronze"));
	JsonObj->SetStringField(TEXT("current_scene"), TEXT("immigration"));
	JsonObj->SetStringField(TEXT("npc_current_emotion"), TEXT("Neutral"));
	JsonObj->SetBoolField(TEXT("is_timeout"), false);
	JsonObj->SetNumberField(TEXT("response_time_sec"), 10.0f);
	
	// WAV 바이너리를 Base64 문자열로 인코딩하여 JSON에 추가
	FString AudioBase64 = FBase64::Encode(RawAudioData);
	JsonObj->SetStringField(TEXT("user_audio_base64"), AudioBase64);
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:8000/api/chat"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &AMurphyPlayerController::OnResponseReceived);
	Request->ProcessRequest();
}

void AMurphyPlayerController::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		PRINTLOGE_JW(TEXT("서버 응답 오류 - 코드: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
		return;
	}
	
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid() && IsValid(TargetNPC))
	{
		FString Dialogue  = JsonObj->GetStringField(TEXT("npc_dialogue"));
		int32 EmotionLevel = JsonObj->GetIntegerField(TEXT("npc_emotion_level"));
		FString AudioURL  = JsonObj->GetStringField(TEXT("npc_audio_url"));
		TargetNPC->ProcessDialogueResponse(Dialogue, EmotionLevel, AudioURL);
	}
}
*/
