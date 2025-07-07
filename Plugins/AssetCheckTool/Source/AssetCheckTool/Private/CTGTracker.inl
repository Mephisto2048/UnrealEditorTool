#pragma once
#ifndef _CTG_TRACKER_INL_
#define _CTG_TRACKER_INL_

#include <Modules/ModuleManager.h>
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <GenericPlatform/GenericPlatformHttp.h>
#include <GenericPlatform/GenericPlatformMisc.h>
#include <Kismet/KismetSystemLibrary.h>


namespace CTG {
	static inline void SendTrackingMessage(FString const& Category, FString const& Action, int Value = 0)
	{
		static constexpr auto* RequestTemplate = TEXT("?attaid=05e00064128&token=6753676592&v=1&cid=Unreal&t=event&ec={category}&ea={action}&el={label}&ev={value}&_dc={randint}");
		FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
		auto HttpRequest = HttpModule.Get().CreateRequest();

		FString CurrentUser = UKismetSystemLibrary::GetPlatformUserName();

		FStringFormatNamedArguments RequestArgs;
		RequestArgs.Add("category", FGenericPlatformHttp::UrlEncode(Category));
		RequestArgs.Add("action", FGenericPlatformHttp::UrlEncode(Action));
		RequestArgs.Add("label", CurrentUser);
		RequestArgs.Add("value", Value);
		int randint = FMath::RandRange(0, 99999);
		RequestArgs.Add("randint", randint);

		FString RequestContent = FString::Format(
			RequestTemplate,
			RequestArgs
		);

		FString Url = TEXT("https://h.trace.qq.com/kv");
		Url += RequestContent;
		HttpRequest->SetURL(Url);
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->OnProcessRequestComplete().BindLambda(
			[](FHttpRequestPtr Req, FHttpResponsePtr HttpResponse, bool bSucceeded) {
				if (HttpResponse) {
					UE_LOG(LogTemp, Verbose, TEXT("Track sent, with response = %d, succeeded = %d"), HttpResponse->GetResponseCode(), bSucceeded);
				}
				else {
					UE_LOG(LogTemp, Verbose, TEXT("Track sent, succeeded = %d"), bSucceeded);
				}
			}
		);
		HttpRequest->ProcessRequest();
	}
}

namespace TACraft {
	static inline void SendTrackingMessageV2(FStringView ProductName, FStringView ProductVersion, FStringView ProjectName, FStringView StudioName, bool FilterInternalUsers = true)
	{
		static constexpr auto* RequestTemplate = TEXT("?"
			"attaid=08b00079699&"
			"token=7134218135&"
			"c=1&"
			"prd={ProductName}&"
			"v={ProductVersion}&"
			"plt={Platform}&"
			"pltv={PlatformVersion}&"
			"un={UserName}&"
			"prj={ProjectName}&"
			"s={StudioName}&"
			"mac={MacAddr}"
		);

		const static FString InternalUsers[] = {
		  TEXT("kelvinjrcai"),
		  TEXT("carterwu"),
		  TEXT("harlanqiu"),
		  TEXT("honghan"),
		  TEXT("ifyu"),
		  TEXT("jarvisli"),
		  TEXT("jocelynhao"),
		  TEXT("viceyliang"),
		  TEXT("whitingyan"),
		  TEXT("yulingji"),
		  TEXT("yuyuzhao"),
		  TEXT("zhimingma")
		};

		const static FString CurrentUser = UKismetSystemLibrary::GetPlatformUserName();
		const static FString Platform = TEXT("Unreal");
		const static FString PlatformVer = FString::Printf(TEXT("%d.%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION, ENGINE_PATCH_VERSION);
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			const static FString MacAddress = FGenericPlatformMisc::GetMacAddressString();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

			if (FilterInternalUsers) {
				for (auto&& Name : InternalUsers)
					if (Name == CurrentUser)
						return;
			}

		FStringFormatNamedArguments RequestArgs;
		RequestArgs.Add("ProductName", FGenericPlatformHttp::UrlEncode(ProductName));
		RequestArgs.Add("ProductVersion", FGenericPlatformHttp::UrlEncode(ProductVersion));
		RequestArgs.Add("Platform", Platform);
		RequestArgs.Add("PlatformVersion", PlatformVer);
		RequestArgs.Add("UserName", FGenericPlatformHttp::UrlEncode(CurrentUser));
		RequestArgs.Add("ProjectName", FGenericPlatformHttp::UrlEncode(ProjectName));
		RequestArgs.Add("StudioName", FGenericPlatformHttp::UrlEncode(StudioName));
		RequestArgs.Add("MacAddr", FGenericPlatformHttp::UrlEncode(MacAddress));

		FString RequestContent = FString::Format(
			RequestTemplate,
			RequestArgs
		);

		FString Url = TEXT("https://h.trace.qq.com/kv");
		Url += RequestContent;

		FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
		auto HttpRequest = HttpModule.Get().CreateRequest();
		HttpRequest->SetURL(Url);
		HttpRequest->SetTimeout(0.1f);
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->OnProcessRequestComplete().BindLambda(
			[](FHttpRequestPtr Req, FHttpResponsePtr HttpResponse, bool bSucceeded) {
				if (HttpResponse) {
					UE_LOG(LogTemp, Verbose, TEXT("Track sent, with response = %d, succeeded = %d"), HttpResponse->GetResponseCode(), bSucceeded);
				}
				else {
					UE_LOG(LogTemp, Verbose, TEXT("Track sent, succeeded = %d"), bSucceeded);
				}
			}
		);
		HttpRequest->ProcessRequest();
	}
}

#endif // _CTG_TRACKER_INL_
