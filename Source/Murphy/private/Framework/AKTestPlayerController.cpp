
#include "Framework/AKTestPlayerController.h"

#include "Actors/AgentNPCBase.h"
#include "VoiceChat/VoiceChatActor.h"
#include "VoiceChat/VoiceRecorderComponent.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "Murphy.h"
#include "Misc/FileHelper.h"

void AAKTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 빙의된 Pawn에서 VoiceRecorderComponent를 찾아 델리게이트 등록....
	if (AVoiceChatActor* VoicePawn = Cast<AVoiceChatActor>(GetPawn()))
	{	
		// todo: [탕카] GetVoiceRecorderComp() 같은 getter 함수 추가
		if (IsValid(VoicePawn->GetVoiceRecorderComp()))
		{
			VoicePawn->GetVoiceRecorderComp()->OnRecordingFinished.AddDynamic(this, &AAKTestPlayerController::OnAudioRecordingFinished);
			PRINTLOG_JW(TEXT("OnRecordingFinished 바인딩 완료"));
		}
	}
}

void AAKTestPlayerController::SetActiveNPC(AAgentNPCBase* NewNPC)
{
    TargetNPC = NewNPC;
}

void AAKTestPlayerController::OnAudioRecordingFinished(const FString& SavedFilePath)
{
    if (!IsValid(TargetNPC)) return;
	
	SendVoiceFileToServer(SavedFilePath);
}

void AAKTestPlayerController::SendVoiceFileToServer(const FString& WAVFilePath)
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
    Request->SetURL(TEXT("http://127.0.0.1:8000/api/chat"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
    Request->SetContent(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &AAKTestPlayerController::OnResponseReceived);
    Request->ProcessRequest();
}

void AAKTestPlayerController::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		PRINTLOG_JW(TEXT("서버 응답 오류 - 코드: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
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
