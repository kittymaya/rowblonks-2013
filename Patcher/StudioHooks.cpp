#include "pch.h"
#include "StudioHooks.h"
#include "Patches.h"
#include "Config.h"

// ===== ComputeProp null dereference fix =====

computePropNullDerefFix_t computePropNullDerefFix_orig = reinterpret_cast<computePropNullDerefFix_t>(ADDRESS_COMPUTEPROP_NULL_DEREF);

float __fastcall computePropNullDerefFix_hook(float* _this)
{
	if (_this)
		return _this[30];

	return 0.0f;
}

// ===== `RobloxView::RenderRequestJob` member function hooks =====

RobloxView__RenderRequestJob__sleepTime_t RobloxView__RenderRequestJob__sleepTime_orig =
	reinterpret_cast<RobloxView__RenderRequestJob__sleepTime_t>(ADDRESS_ROBLOXVIEW_RENDERREQUESTJOB_SLEEPTIME);

void __fastcall RobloxView__RenderRequestJob__sleepTime_hook(RobloxView__RenderRequestJob* _this, void*, double* a2, int a3)
{
	if (!Config::fpsUnlocked)
	{
		RobloxView__RenderRequestJob__sleepTime_orig(_this, a2, a3);
		return;
	}

	if (_this->awake)
		RBX::DataModelJob__sleepTime(reinterpret_cast<RBX::DataModelJob*>(_this), a2, a3, Config::desiredFps);
	else
		*a2 = std::numeric_limits<double>::max();
}
