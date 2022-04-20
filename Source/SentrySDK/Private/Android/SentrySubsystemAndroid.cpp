// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemAndroid.h"
#include "SentryEventAndroid.h"
#include "SentryUserFeedbackAndroid.h"
#include "SentryUserAndroid.h"

#include "SentryEvent.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"

#include "Callbacks/SentryScopeCallbackAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"

const ANSICHAR* SentrySubsystemAndroid::SentryJavaClassName = "io/sentry/Sentry";
const ANSICHAR* SentrySubsystemAndroid::SentryBridgeJavaClassName = "com/sentry/unreal/SentryBridgeJava";

void SentrySubsystemAndroid::InitWithSettings(const USentrySettings* settings)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis,
		SentryConvertorsAndroid::StringToNative(settings->DsnUrl));
}

void SentrySubsystemAndroid::Close()
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "close", "()V");
}

void SentrySubsystemAndroid::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "addBreadcrumb", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::StringToNative(message),
		SentryConvertorsAndroid::StringToNative(category),
		SentryConvertorsAndroid::StringToNative(type),
		SentryConvertorsAndroid::StringMapToNative(data),
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

USentryId* SentrySubsystemAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level));

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryBridgeJavaClassName, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetNativeObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	jobject id = SentryMethodCallAndroid::CallStaticObjectMethod(SentryBridgeJavaClassName, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetNativeObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

void SentrySubsystemAndroid::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	TSharedPtr<SentryUserFeedbackAndroid> userFeedbackAndroid = StaticCastSharedPtr<SentryUserFeedbackAndroid>(userFeedback->GetNativeImpl());

	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "captureUserFeedback", "(Lio/sentry/UserFeedback;)V",
		userFeedbackAndroid->GetNativeObject());
}

void SentrySubsystemAndroid::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserAndroid> userAndroid = StaticCastSharedPtr<SentryUserAndroid>(user->GetNativeImpl());

	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "setUser", "(Lio/sentry/protocol/User;)V",
		userAndroid->GetNativeObject());
}

void SentrySubsystemAndroid::RemoveUser()
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "setUser", "(Lio/sentry/protocol/User;)V", nullptr);
}

void SentrySubsystemAndroid::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "configureScope", "(J)V",
		(jlong)scopeCallback);
}

void SentrySubsystemAndroid::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "setContext", "(Ljava/lang/String;Ljava/util/HashMap;)V",
		SentryConvertorsAndroid::StringToNative(key), SentryConvertorsAndroid::StringMapToNative(values));
}

void SentrySubsystemAndroid::SetTag(const FString& key, const FString& value)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "setTag", "(Ljava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(key), SentryConvertorsAndroid::StringToNative(value));
}

void SentrySubsystemAndroid::RemoveTag(const FString& key)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryBridgeJavaClassName, "removeTag", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(key));
}
