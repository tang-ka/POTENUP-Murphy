
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

void AMurphyPlayerController::SetActiveNPC(AAgentNPCBase* NewNPC)
{
	TargetNPC = NewNPC;
}

void AMurphyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Record, ETriggerEvent::Started, this, &AMurphyPlayerController::RecordStart);
		EnhancedInputComponent->BindAction(IA_Record, ETriggerEvent::Completed, this, &AMurphyPlayerController::RecordEnd);
		EnhancedInputComponent->BindAction(IA_PlayAudio, ETriggerEvent::Started, this, &AMurphyPlayerController::RecordAudioPlay);
	}
}

void AMurphyPlayerController::RecordStart(const FInputActionValue& Value)
{
	if (!IsValid(TargetNPC))
	{
		PRINTLOGW_JW(TEXT("[VoiceTest] - NPC가 근처에 없습니다. 녹음을 시작하지 않습니다."));
		return;
	}
	
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		PRINTLOGW_JW(TEXT("[VoiceTest] - Start Recording"));
		MurphyPlayer->GetVoiceRecorderComp()->StartRecording();
	}
}

void AMurphyPlayerController::RecordEnd(const FInputActionValue& Value)
{
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		if (MurphyPlayer->GetVoiceRecorderComp()->IsRecording())
		{
			PRINTLOGW_JW(TEXT("[VoiceTest] - Stop & Save"));
			MurphyPlayer->GetVoiceRecorderComp()->StopRecording(TEXT("TestRecording"), true);
		}
	}
}

void AMurphyPlayerController::RecordAudioPlay(const FInputActionValue& Value)
{
	if (AMurphyPlayer* MurphyPlayer = Cast<AMurphyPlayer>(GetPawn()))
	{
		PRINTLOGW_JW(TEXT("[VoiceTest] - Play"));
		MurphyPlayer->GetVoiceRecorderComp()->PlayRecordedSamples();
	}
}

void AMurphyPlayerController::OnAudioRecordingFinished(const FString& SavedFilePath)
{
	if (!IsValid(TargetNPC)) return;
		
	SendVoiceFileToServer(SavedFilePath);  // multipart 방식 (wav 파일 그대로 넘기기)
	// SendVoiceDataAsJsonBase64(SavedFilePath); // Base64 json 방식 (Base64로 인코딩 후 넘기기)
}

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

