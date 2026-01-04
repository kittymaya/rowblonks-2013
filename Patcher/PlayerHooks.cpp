#include "pch.h"
#include "PlayerHooks.h"
#include "Patches.h"
#include "Config.h"

RBX::Network::initWithPlayerSecurity_t RBX::Network::initWithPlayerSecurity_orig = 
	reinterpret_cast<RBX::Network::initWithPlayerSecurity_t>(ADDRESS_NETWORK_INITWITHPLAYERSECURITY);

// SECURITY BYPASS
void __cdecl RBX::Network::initWithPlayerSecurity_hook()
{
	initWithoutSecurity();
}

// ===== `RBX::Network::ClientReplicator` member function hooks =====

RBX::Network::ClientReplicator__canUseProtocolVersion_t RBX::Network::ClientReplicator__canUseProtocolVersion_orig =
	reinterpret_cast<RBX::Network::ClientReplicator__canUseProtocolVersion_t>(ADDRESS_CLIENTREPLICATOR_CANUSEPROTOCOLVERSION);

bool __fastcall RBX::Network::ClientReplicator__canUseProtocolVersion_hook(RBX::Network::ClientReplicator* _this, void*, int version)
{
	// to match the studio protocol version (originally uses version 13)
	return version <= 12;
}

// ===== `RBX::Network::Replicator::RockyItem` member function hooks =====

RBX::Network::Replicator__RockyItem__write_t RBX::Network::Replicator__RockyItem__write_orig =
	reinterpret_cast<RBX::Network::Replicator__RockyItem__write_t>(ADDRESS_NETWORK_REPLICATOR_ROCKYITEM_WRITE);

// SECURITY BYPASS
// never send rocky items
bool __fastcall RBX::Network::Replicator__RockyItem__write_hook(RBX::Network::Replicator__RockyItem* _this, void*, void* bitstream)
{
	return true;
}

// ===== `CRobloxWnd::RenderRequestJob` member function hooks =====

CRobloxWnd__RenderRequestJob__sleepTime_t CRobloxWnd__RenderRequestJob__sleepTime_orig =
	reinterpret_cast<CRobloxWnd__RenderRequestJob__sleepTime_t>(ADDRESS_CROBLOXWND_RENDERREQUESTJOB_SLEEPTIME);

void __fastcall CRobloxWnd__RenderRequestJob__sleepTime_hook(CRobloxWnd__RenderRequestJob* _this, void*, double* a2, int a3)
{
	if (!Config::fpsUnlocked)
	{
		CRobloxWnd__RenderRequestJob__sleepTime_orig(_this, a2, a3);
		return;
	}

	if (_this->awake)
		RBX::DataModelJob__sleepTime(reinterpret_cast<RBX::DataModelJob*>(_this), a2, a3, Config::desiredFps);
	else
		*a2 = std::numeric_limits<double>::max();
}

// ===== `CRobloxWnd::UserInputJob` member function hooks =====

CRobloxWnd__UserInputJob__sleepTime_t CRobloxWnd__UserInputJob__sleepTime_orig =
	reinterpret_cast<CRobloxWnd__UserInputJob__sleepTime_t>(ADDRESS_CROBLOXWND_USERINPUTJOB_SLEEPTIME);

void __fastcall CRobloxWnd__UserInputJob__sleepTime_hook(CRobloxWnd__UserInputJob* _this, void*, double* a2, int a3)
{
	RBX::DataModelJob__sleepTime(reinterpret_cast<RBX::DataModelJob*>(_this), a2, a3, 
		Config::fpsUnlocked ? Config::desiredFps : 60.0);
}
