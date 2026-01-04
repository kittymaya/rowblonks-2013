#include "pch.h"
#include "LuaApiExtensions.h"
#include "RBXDefs.h"
#include "Config.h"

// ===== functionality related to script identities =====

static int getThreadIdentity(lua_State* L)
{
	lua_pushinteger(L, RobloxExtraSpace::get(L)->identity);
	return 1;
}

static int setThreadIdentity(lua_State* L)
{
	if (!Config::allowChangingScriptIdentities)
		luaL_error(L, "no");

	RobloxExtraSpace::get(L)->identity = luaL_checkinteger(L, 1);

	return 0;
}

// ===== functionality related to local corescripts =====

static const auto makeCoreScriptSharedPtr =
	reinterpret_cast<void*(__thiscall*)(void*, RBX::CoreScript*, bool)>(ADDRESS_MAKE_CORESCRIPT_SHAREDPTR);

// maybe move this later
static void __forceinline fixedAddScript(RBX::ScriptContext* scriptContext, void* script)
{
	__asm
	{
		sub esp, 0x68
		mov dword ptr ds:[esp], 0x2
		mov dword ptr ds:[esp + 0x8], 0
		mov dword ptr ds:[esp + 0x28], 0
		mov dword ptr ds:[esp + 0x48], 0
		push script
		mov ecx, scriptContext
		call RBX::ScriptContext__addScript
	}
}

static int addLocalCoreScript(lua_State* L)
{
	Lua::checkPermissions(L, 2, "add a local CoreScript");

	std::string name = luaL_checkstring(L, 1);
	auto parent = Lua::checkInstance(L, 2);

	std::string source = "rbxasset://../extra/corescripts/" + name + ".lua";

	auto buf = (*vc90::operator_new)(RBX::CoreScript::size);
	auto coreScript = RBX::CoreScript__constructor(reinterpret_cast<RBX::CoreScript*>(buf), source);

	// create a boost::shared_ptr<RBX::CoreScript>
	uint32_t* sharedPtr[2];
	makeCoreScriptSharedPtr(sharedPtr, coreScript, false);

	auto csInstance = reinterpret_cast<RBX::Instance*>(coreScript);
	RBX::Instance__setRobloxLocked(csInstance, true);
	RBX::Instance__setName(csInstance, name);
	RBX::Instance__setParent(csInstance, parent, false);

	auto scriptContext = RobloxExtraSpace::get(L)->shared->scriptContext;
	if (parent != reinterpret_cast<RBX::Instance*>(scriptContext))
		fixedAddScript(scriptContext, csInstance);

	// atomically decrement the ref count of the boost::shared_ptr
	InterlockedExchangeSubtract(*(sharedPtr + 1), 1);

	return 0;
}

static const auto makeScriptSharedPtr =
	reinterpret_cast<void*(__thiscall*)(void*, RBX::Script*, bool)>(ADDRESS_MAKE_SCRIPT_SHAREDPTR);

static const auto registerScriptLibrary =
	reinterpret_cast<RBX::Script**(__thiscall*)(void*, const std::string&)>(ADDRESS_REGISTER_SCRIPT_LIBRARY);

static int registerLocalLibrary(lua_State* L)
{
	Lua::checkPermissions(L, 2, "register a local library");

	std::string name = luaL_checkstring(L, 1);

	auto buf = (*vc90::operator_new)(RBX::Script::size);
	auto script = RBX::Script__constructor(reinterpret_cast<RBX::Script*>(buf));

	// create a boost::shared_ptr<RBX::Script>
	uint32_t* sharedPtr[2];
	makeScriptSharedPtr(sharedPtr, script, false);

	std::stringstream sourceStream;

	// script objects can only be registered as a library if their source property is set
	// so we have to read the library file ourselves and set the property
	std::string path = Config::gameDirectory + "extra/libraries/" + name + ".lua";
	std::ifstream sourceFile(path);
	sourceStream << sourceFile.rdbuf();
	sourceFile.close();

	char buf2[RBX::ProtectedString::size];
	auto source = RBX::ProtectedString__fromTrustedSource(buf2, sourceStream.str());
	RBX::Script__setDisabled(script, true);
	RBX::Script__setSource(script, source);

	auto scriptInstance = reinterpret_cast<RBX::Instance*>(script);
	RBX::Instance__setRobloxLocked(scriptInstance, true);
	RBX::Instance__setName(scriptInstance, name);

	auto scriptContext = RobloxExtraSpace::get(L)->shared->scriptContext;
	RBX::Instance__setParent(scriptInstance, reinterpret_cast<RBX::Instance*>(scriptContext), false);

	// not fully sure what this is (probably adding to a map?), but this is done to register the script object as a library
	// this is NOT checking if a library was already registered with the given name
	auto res = registerScriptLibrary(*(reinterpret_cast<uint32_t**>(scriptContext) + 150) + 8, name);
	*res = script;

	// atomically decrement the ref count of the boost::shared_ptr
	InterlockedExchangeSubtract(*(sharedPtr + 1), 1);

	RBX::ProtectedString__destructor(source);

	return 0;
}

// ===== functionality related to game chat =====

static int produceGameChat(lua_State* L)
{
	std::string message = luaL_checkstring(L, 1);

	if (auto dataModel = Lua::getDataModel(L))
	{
		if (auto players = RBX::DataModel__find__Players(dataModel))
			RBX::Players__gameChat(players, message);
	}

	return 0;
}

static const luaL_Reg mayaLib[] = {
	{ "GetThreadIdentity", getThreadIdentity },
	{ "SetThreadIdentity", setThreadIdentity },

	{ "AddLocalCoreScript", addLocalCoreScript },
	{ "RegisterLocalLibrary", registerLocalLibrary },

	{ "ProduceGameChat", produceGameChat },

    { nullptr, nullptr },
};

int Lua::openApiExtensionsLibrary(lua_State* L)
{
	luaL_register(L, "maya", mayaLib);
	return 1;
}
