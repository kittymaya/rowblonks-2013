#pragma once

#include "RBXDefs.h"

struct lua_State;

namespace RBX
{
	// ===== `RBX::ContentId` member function hooks =====

	void __fastcall ContentId__convertToLegacyContent_hook(RBX::ContentId* _this, void*, vc90::std::string* baseUrl);

	// ===== `RBX::ContentProvider` member function hooks =====

	// SECURITY BYPASS
	const char* __cdecl ContentProvider__verifyScriptSignature_hook(const char* source, bool required);

	// SECURITY BYPASS
	const char* __cdecl ContentProvider__verifyRequestedScriptSignature_hook(const char* source, const std::string& assetId, bool required);

	// ===== `RBX::DataModel` member function hooks =====

	void __fastcall DataModel__startCoreScripts_hook(DataModel* _this, void*, AdornRbxGfx* adorn, bool buildInGameGui);

	// ===== `RBX::ScriptContext` member function hooks =====

	lua_State* __fastcall ScriptContext__openState_hook(ScriptContext* _this);

	// ===== `RBX::Http` member function hooks =====
	
	bool __cdecl Http__trustCheck_hook(const char* url);

	// ===== `RBX::HeartbeatTask` member function hooks =====

	RBX::HeartbeatTask* __fastcall HeartbeatTask__constructor_hook(RBX::HeartbeatTask* _this, void*, int a2, int a3);

	// ===== `RBX::RunService` member function hooks =====

	void __fastcall RunService__step_hook(RBX::RunService* _this, void*, double delta);

	// ===== `RBX::NetworkSettings` member function hooks =====

	void __fastcall NetworkSettings__setDataSendRate_hook(RBX::NetworkSettings* _this, void*, float value);

	void __fastcall NetworkSettings__setPhysicsSendRate_hook(RBX::NetworkSettings* _this, void*, float value);

	void __fastcall NetworkSettings__setReceiveRate_hook(RBX::NetworkSettings* _this, void*, double value);
}
