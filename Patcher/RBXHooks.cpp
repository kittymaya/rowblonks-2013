#include "pch.h"
#include "RBXHooks.h"
#include "Patches.h"
#include "VC90Defs.h"
#include "LuaApiExtensions.h"
#include "UrlHelper.h"
#include "Config.h"

// ===== `RBX::ContentId` member function hooks =====

RBX::ContentId__convertToLegacyContent_t RBX::ContentId__convertToLegacyContent_orig = 
	reinterpret_cast<RBX::ContentId__convertToLegacyContent_t>(ADDRESS_CONTENTID_CONVERTTOLEGACYCONTENT);

// rebuild asset URLs to use the assetdelivery API
void __fastcall RBX::ContentId__convertToLegacyContent_hook(RBX::ContentId* _this, void*, vc90::std::string* baseUrl)
{
	RBX::ContentId__convertToLegacyContent_orig(_this, baseUrl);

	UrlHelper urlHelper(_this->id);

	if (urlHelper.isAssetUrl())
	{
		std::string query = urlHelper.query;
		std::transform(query.begin(), query.end(), query.begin(), std::tolower);

		static const std::string idParam = "id=";
		size_t idParamPos = query.find(idParam);
		if (idParamPos != std::string::npos)
		{
			size_t idPos = idParamPos + idParam.size();
			size_t idEndPos = query.find('&', idPos);
			if (idEndPos == std::string::npos)
				idEndPos = query.size();

			std::string assetId = query.substr(idPos, idEndPos - idPos);
			if (std::find(Config::assetOverrides.begin(), Config::assetOverrides.end(), assetId) != Config::assetOverrides.end())
			{
				std::string newId = "rbxasset://../extra/assetoverrides/" + assetId;
				(*vc90::std::string__assign_from_cstr)(reinterpret_cast<vc90::std::string*>(&_this->id), newId.c_str());
				return;
			}
		}

		urlHelper.protocol = "https";
		urlHelper.hostname = "assetdelivery.roblox.com";
		urlHelper.path = "v1/asset/";

		(*vc90::std::string__assign_from_cstr)(reinterpret_cast<vc90::std::string*>(&_this->id), urlHelper.buildUrl().c_str());
	}
}

// ===== `RBX::ContentProvider` member function hooks =====

RBX::ContentProvider__verifyScriptSignature_t RBX::ContentProvider__verifyScriptSignature_orig = 
	reinterpret_cast<RBX::ContentProvider__verifyScriptSignature_t>(ADDRESS_VERIFYSCRIPTSIGNATURE);

// SECURITY BYPASS
// never require script signatures (1)
const char* __cdecl RBX::ContentProvider__verifyScriptSignature_hook(const char* source, bool required)
{
	if (source[0] != '%')
		return source;

	const char* end = std::strchr(source + 1, '%');
	if (end == nullptr)
		return source;

	return end + 1;
}

RBX::ContentProvider__verifyRequestedScriptSignature_t RBX::ContentProvider__verifyRequestedScriptSignature_orig = 
	reinterpret_cast<RBX::ContentProvider__verifyRequestedScriptSignature_t>(ADDRESS_VERIFYREQUESTEDSCRIPTSIGNATURE);

// SECURITY BYPASS
// never require script signatures (2)
const char* __cdecl RBX::ContentProvider__verifyRequestedScriptSignature_hook(const char* source, const std::string& assetId, bool required)
{
	return RBX::ContentProvider__verifyRequestedScriptSignature_orig(source, assetId, false);
}

// ===== `RBX::DataModel` member function hooks =====

RBX::DataModel__startCoreScripts_t RBX::DataModel__startCoreScripts_orig =
	reinterpret_cast<RBX::DataModel__startCoreScripts_t>(ADDRESS_DATAMODEL_STARTCORESCRIPTS);

// execute a local Studio.ashx
void __fastcall RBX::DataModel__startCoreScripts_hook(DataModel* _this, void*, AdornRbxGfx* adorn, bool buildInGameGui)
{
	RBX::GuiBuilder__buildGui(reinterpret_cast<RBX::GuiBuilder*>(&_this->guiBuilderStart), adorn, _this->workspace, buildInGameGui);

	char buf[RBX::ProtectedString::size];
	auto source = RBX::ProtectedString__fromTrustedSource(buf, "loadfile('rbxasset://../extra/studio.lua')()");

	auto sc = RBX::DataModel__create__ScriptContext(_this);
	RBX::ScriptContext__executeInNewThread(sc, 4, source, "magic");

	RBX::ProtectedString__destructor(source);
}

// ===== `RBX::ScriptContext` member function hooks =====

RBX::ScriptContext__openState_t RBX::ScriptContext__openState_orig = 
	reinterpret_cast<RBX::ScriptContext__openState_t>(ADDRESS_SCRIPTCONTEXT_OPENSTATE);

// maybe move this later
static void __forceinline fixedOpenProtectedLibrary(lua_State* L, const char* name, lua_CFunction func)
{
	__asm
	{
		push func
		push name
		mov eax, L
		call RBX::ScriptContext__openProtectedLibrary
		add esp, 8
	}
}

// open Lua extensions
lua_State* __fastcall RBX::ScriptContext__openState_hook(ScriptContext* _this)
{
	auto L = RBX::ScriptContext__openState_orig(_this);

	fixedOpenProtectedLibrary(L, "maya", Lua::openApiExtensionsLibrary);

	return L;
}

// ===== `RBX::Http` member function hooks =====

RBX::Http__trustCheck_t RBX::Http__trustCheck_orig =
	reinterpret_cast<RBX::Http__trustCheck_t>(ADDRESS_HTTP_TRUSTCHECK);

// allow content from any url
bool __cdecl RBX::Http__trustCheck_hook(const char* url)
{
	return true;
}

// ===== `RBX::HeartbeatTask` member function hooks =====

RBX::HeartbeatTask__constructor_t RBX::HeartbeatTask__constructor_orig = 
	reinterpret_cast<RBX::HeartbeatTask__constructor_t>(ADDRESS_HEARTBEATTASK_CONSTRUCTOR);

RBX::HeartbeatTask* __fastcall RBX::HeartbeatTask__constructor_hook(RBX::HeartbeatTask* _this, void*, int a2, int a3)
{
	auto result = RBX::HeartbeatTask__constructor_orig(_this, a2, a3);

	if (Config::fpsUnlocked)
		result->fps = Config::desiredFps;

	return result;
}

// ===== `RBX::RunService` member function hooks =====

RBX::RunService__step_t RBX::RunService__step_orig = 
	reinterpret_cast<RBX::RunService__step_t>(ADDRESS_RUNSERVICE_STEP);

const auto fireHeartbeatSignal = 
	reinterpret_cast<void(__thiscall*)(void* _this, double* args)>(ADDRESS_FIRE_HEARTBEAT_SIGNAL);

const auto fireSteppedSignal = 
	reinterpret_cast<void(__thiscall*)(void* _this, double elapsedTime, double delta)>(ADDRESS_FIRE_STEPPED_SIGNAL);

void __fastcall RBX::RunService__step_hook(RBX::RunService* _this, void*, double delta)
{
	double elapsedTime = delta + _this->elapsedTime;
	_this->elapsedTime = elapsedTime;

	double a[2];
	a[0] = elapsedTime;
	a[1] = delta;
	fireHeartbeatSignal(&_this->heartbeatSignalStart, a);

	// this is a hack so that the Stepped event gets fired about 30 times per second
	// some places were relying on this event to update the velocity of parts, causing 
	// them to accelerate faster than they should

	double steppedDelta = elapsedTime - _this->elapsedTimeAtLastStep;
	if (steppedDelta >= 1.0 / 30.0)
	{
		_this->elapsedTimeAtLastStep = elapsedTime;

		fireSteppedSignal(&_this->steppedSignalStart, elapsedTime, steppedDelta);
	}
}
