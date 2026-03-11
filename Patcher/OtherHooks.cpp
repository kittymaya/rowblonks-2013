#include "pch.h"
#include "OtherHooks.h"
#include "Patches.h"
#include "VC90Defs.h"
#include "Config.h"

#include <dwmapi.h>

// ===== bypass "invalid request" for some urls =====

invalidRequestCheck_t invalidRequestCheck_orig = reinterpret_cast<invalidRequestCheck_t>(ADDRESS_INVALID_REQUEST_CHECK);

// really only takes 1 std::string by value, and is responsible for calling its destructor
void __cdecl invalidRequestCheck_hook(char a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
	auto url = reinterpret_cast<vc90::std::string*>(&a1);
	(*vc90::std::string__destructor)(url);
}

// ===== bypass GetScriptState.ashx check =====

scriptHashCheck_t scriptHashCheck_orig = reinterpret_cast<scriptHashCheck_t>(ADDRESS_SCRIPT_HASH_CHECK);

bool __fastcall scriptHashCheck_hook(void* _this, void*, int a2, int a3)
{
	return true;
}

// ===== physics fps unlocking =====

getPhysicsStepsPerSec_t getPhysicsStepsPerSec_orig = reinterpret_cast<getPhysicsStepsPerSec_t>(ADDRESS_PHYSICS_STEPS_PER_SEC);

int __cdecl getPhysicsStepsPerSec_hook()
{
	return Config::fpsUnlocked ? Config::desiredFps : 30;
}

getSecsPerPhysicsStep_t getSecsPerPhysicsStep_orig = reinterpret_cast<getSecsPerPhysicsStep_t>(ADDRESS_SECS_PER_PHYSICS_STEP);

double __cdecl getSecsPerPhysicsStep_hook()
{
	return 1.0 / static_cast<double>(getPhysicsStepsPerSec_hook());
}

// ===== fps fixes for MotorJoint and Motor6DJoint =====

static float getAdjustedMaxVelocity(float maxVelocity)
{
	if (Config::fpsUnlocked)
		maxVelocity *= 30.0f / static_cast<float>(Config::desiredFps);

	return maxVelocity;
}

void* motor6dJointFpsFix_ptr = reinterpret_cast<void*>(ADDRESS_MOTOR6DJOINT_FPS_FIX);
void* motor6dJointFpsFix_jumpOut = reinterpret_cast<void*>(ADDRESS_MOTOR6DJOINT_FPS_FIX_JUMPOUT);

void __declspec(naked) motor6dJointFpsFix_hook()
{
	__asm
	{
		// store adjusted max velocity in st(0)
		push dword ptr ds:[esi + 0xD0]
		call getAdjustedMaxVelocity
		add esp, 4

		// the xmm1 register was set again in getAdjustedMaxVelocity
		xorps xmm1, xmm1

		// return to original code
		jmp motor6dJointFpsFix_jumpOut
	}
}

void* motorJointFpsFix_ptr = reinterpret_cast<void*>(ADDRESS_MOTORJOINT_FPS_FIX);
void* motorJointFpsFix_jumpOut = reinterpret_cast<void*>(ADDRESS_MOTORJOINT_FPS_FIX_JUMPOUT);

void __declspec(naked) motorJointFpsFix_hook()
{
	__asm
	{
		// store adjusted max velocity in st(0)
		push dword ptr ds:[ecx + 0xA8]
		call getAdjustedMaxVelocity
		add esp, 4

		// return to original code
		jmp motorJointFpsFix_jumpOut
	}
}

// ===== dark window title bars =====

CreateWindowExA_t CreateWindowExA_orig = CreateWindowExA;

HWND __stdcall CreateWindowExA_hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	auto hWnd = CreateWindowExA_orig(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (!hWndParent)
	{
		BOOL useImmersiveDarkMode = TRUE;
		DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode));
	}

	return hWnd;
}
