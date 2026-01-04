#pragma once

#include "VC90Defs.h"
#include "Patches.h"
#include "LuaDefs.h"

namespace RBX
{
	namespace Network
	{
#ifdef PATCHER_PLAYER
		// HOOKED (player)
		typedef void(__cdecl* initWithPlayerSecurity_t)();
		extern initWithPlayerSecurity_t initWithPlayerSecurity_orig;

		const auto initWithoutSecurity = reinterpret_cast<void(__cdecl*)()>(ADDRESS_NETWORK_INITWITHOUTSECURITY);

		// ===== `ClientReplicator` class =====

		class ClientReplicator;

		// HOOKED (player)
		typedef bool(__thiscall* ClientReplicator__canUseProtocolVersion_t)(ClientReplicator* _this, int version);
		extern ClientReplicator__canUseProtocolVersion_t ClientReplicator__canUseProtocolVersion_orig;

		// ===== `Replicator::RockyItem` class =====

		class Replicator__RockyItem;

		// HOOKED (player)
		typedef bool(__thiscall* Replicator__RockyItem__write_t)(Replicator__RockyItem* _this, void* bitstream);
		extern Replicator__RockyItem__write_t Replicator__RockyItem__write_orig;
#endif
	}

	namespace Reflection
	{
		// ===== `Descriptor` class =====

		class Descriptor;
	}

	// ===== `ContentId` class =====

	class ContentId
	{
	public:
		std::string id;
	};

	// HOOKED
	typedef void(__thiscall* ContentId__convertToLegacyContent_t)(ContentId* _this, vc90::std::string* baseUrl);
	extern ContentId__convertToLegacyContent_t ContentId__convertToLegacyContent_orig;

	// ===== `ContentProvider` class =====

	class ContentProvider;

	// static method
	// HOOKED
	typedef const char* (__cdecl* ContentProvider__verifyScriptSignature_t)(const char* source, bool required);
	extern ContentProvider__verifyScriptSignature_t ContentProvider__verifyScriptSignature_orig;

	// static method
	// HOOKED
	typedef const char* (__cdecl* ContentProvider__verifyRequestedScriptSignature_t)(const char* source, const std::string& assetId, bool required);
	extern ContentProvider__verifyRequestedScriptSignature_t ContentProvider__verifyRequestedScriptSignature_orig;

	// ===== `Workspace` class =====

	class Workspace;

	// ===== `AdornRbxGfx` class =====

	class AdornRbxGfx;

	// ===== `GuiBuilder` class =====

	class GuiBuilder;

	const auto GuiBuilder__buildGui = 
		reinterpret_cast<void(__thiscall*)(GuiBuilder* _this, AdornRbxGfx* adorn, Workspace* workspace, bool buildInGameGui)>(ADDRESS_GUIBUILDER_BUILDGUI);

	// ===== `Players` class =====

	class Players;

	const auto Players__gameChat =
		reinterpret_cast<void(__thiscall*)(Players* _this, const std::string& message)>(ADDRESS_PLAYERS_GAMECHAT);
	
	// ===== `ProtectedString` class =====

	class ProtectedString
	{
	public:
		static constexpr size_t size = 32;
	};

	const auto ProtectedString__fromTrustedSource =
		reinterpret_cast<ProtectedString* (__cdecl*)(void* buf, const std::string& str)>(ADDRESS_PROTECTEDSTRING_FROMTRUSTEDSOURCE);

	const auto ProtectedString__destructor =
		reinterpret_cast<void(__thiscall*)(ProtectedString* _this)>(ADDRESS_PROTECTEDSTRING_DESTRUCTOR);

	// ===== `ScriptContext` class =====

	class ScriptContext;

	const auto ScriptContext__executeInNewThread =
		reinterpret_cast<void(__thiscall*)(ScriptContext* _this, int identity, ProtectedString* source, const char* name)>(ADDRESS_SCRIPTCONTEXT_EXECUTEINNEWTHREAD);

	// HOOKED
	typedef lua_State*(__thiscall* ScriptContext__openState_t)(ScriptContext* _this);
	extern ScriptContext__openState_t ScriptContext__openState_orig;

	// this function was optimized at compile time... lua_State* should be in EAX before calling
	const auto ScriptContext__openProtectedLibrary = 
		reinterpret_cast<void(__cdecl*)(const char* name, lua_CFunction func)>(ADDRESS_SCRIPTCONTEXT_OPENPROTECTEDLIBRARY);

	// this function was also optimized at compile time... the stack should be grown by an additional 0x68 bytes
	// to store the function's local variables before calling
	const auto ScriptContext__addScript =
		reinterpret_cast<void(__thiscall*)(ScriptContext* _this, void*)>(ADDRESS_SCRIPTCONTEXT_ADDSCRIPT);

	// ===== `DataModel` class =====

	class DataModel
	{
	private:
		char padding1[2944];
	public:
		Workspace* workspace;
	private:
		char padding2[564];
	public:
		void* guiBuilderStart;
	};

	// HOOKED
	typedef void(__thiscall* DataModel__startCoreScripts_t)(DataModel* _this, AdornRbxGfx* adorn, bool buildInGameGui);
	extern DataModel__startCoreScripts_t DataModel__startCoreScripts_orig;

	const auto DataModel__create__ScriptContext = 
		reinterpret_cast<ScriptContext*(__thiscall*)(DataModel* _this)>(ADDRESS_DM_CREATE_SCRIPTCONTEXT);

	const auto DataModel__find__Players =
		reinterpret_cast<Players*(__thiscall*)(DataModel* _this)>(ADDRESS_DM_FIND_PLAYERS);

	// ===== `CoreScript` class =====

	class CoreScript
	{
	public:
		static constexpr size_t size = 160;
	};

	const auto CoreScript__constructor =
		reinterpret_cast<CoreScript*(__thiscall*)(CoreScript* _this, const std::string& source)>(ADDRESS_CORESCRIPT_CONSTRUCTOR);

	// ===== `Script` class =====

	class Script
	{
	public:
		static constexpr size_t size = 196;
	};

	const auto Script__constructor =
		reinterpret_cast<Script*(__thiscall*)(Script* _this)>(ADDRESS_SCRIPT_CONSTRUCTOR);

	const auto Script__setDisabled =
		reinterpret_cast<void(__thiscall*)(Script* _this, bool disabled)>(ADDRESS_SCRIPT_SETDISABLED);

	const auto Script__setSource =
		reinterpret_cast<void(__thiscall*)(Script* _this, ProtectedString* source)>(ADDRESS_SCRIPT_SETSOURCE);

	// ===== `Instance` class =====

	class Instance
	{
	private:
		char padding1[56];
	public:
		Instance* parent;
	};

	const auto Instance__setRobloxLocked = 
		reinterpret_cast<void(__thiscall*)(Instance* _this, bool robloxLocked)>(ADDRESS_INSTANCE_SETROBLOXLOCKED);

	const auto Instance__setName =
		reinterpret_cast<void(__thiscall*)(Instance* _this, const std::string& name)>(ADDRESS_INSTANCE_SETNAME);

	const auto Instance__setParent =
		reinterpret_cast<void(__thiscall*)(Instance* _this, Instance* parent, bool force)>(ADDRESS_INSTANCE_SETPARENT);

	const auto Instance__raisePropertyChanged =
		reinterpret_cast<void(__thiscall*)(Instance* _this, Reflection::Descriptor* propDesc)>(ADDRESS_INSTANCE_RAISEPROPERTYCHANGED);

	// ===== `Http` class =====

	class Http;

	// HOOKED
	// static method
	typedef bool(__cdecl* Http__trustCheck_t)(const char* url);
	extern Http__trustCheck_t Http__trustCheck_orig;

	// ===== `HeartbeatTask` class =====

	class HeartbeatTask
	{
	private:
		char padding1[552];
	public:
		double fps;
	};

	// HOOKED
	typedef HeartbeatTask* (__thiscall* HeartbeatTask__constructor_t)(HeartbeatTask* _this, int a2, int a3);
	extern HeartbeatTask__constructor_t HeartbeatTask__constructor_orig;

	// ===== `RunService` class =====

	class RunService
	{
	private:
		char padding1[120];
	public:
		double elapsedTime;
	private:
		char padding2[8];
	public:
		double elapsedTimeAtLastStep;
	private:
		char padding3[8];
	public:
		void* heartbeatSignalStart;
	private:
		char padding4[8];
	public:
		void* steppedSignalStart;
	};

	// HOOKED
	typedef void(__thiscall* RunService__step_t)(RunService* _this, double delta);
	extern RunService__step_t RunService__step_orig;

	// ===== `DataModelJob` class =====
	
	class DataModelJob;

	const auto DataModelJob__sleepTime = 
		reinterpret_cast<void(__thiscall*)(DataModelJob* _this, double* a2, int a3, double fps)>(ADDRESS_DATAMODELJOB_SLEEPTIME);

	// ===== `NetworkSettings` class =====

	class NetworkSettings
	{
	private:
		char padding1[180];
	public:
		float dataSendRate;
		float physicsSendRate;
	private:
		char padding2[4];
	public:
		double receiveRate;
	};

	// HOOKED
	typedef void(__thiscall* NetworkSettings__setDataSendRate_t)(NetworkSettings* _this, float value);
	extern NetworkSettings__setDataSendRate_t NetworkSettings__setDataSendRate_orig;

	// HOOKED
	typedef void(__thiscall* NetworkSettings__setPhysicsSendRate_t)(NetworkSettings* _this, float value);
	extern NetworkSettings__setPhysicsSendRate_t NetworkSettings__setPhysicsSendRate_orig;

	// HOOKED
	typedef void(__thiscall* NetworkSettings__setReceiveRate_t)(NetworkSettings* _this, double value);
	extern NetworkSettings__setReceiveRate_t NetworkSettings__setReceiveRate_orig;
}
