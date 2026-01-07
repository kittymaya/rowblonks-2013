#include "pch.h"
#include "Patches.h"
#include "RBXHooks.h"
#include "OtherHooks.h"

#if defined(PATCHER_PLAYER)
#include "PlayerHooks.h"
#elif defined(PATCHER_STUDIO)
#include "StudioHooks.h"
#endif

#include <detours.h>

static const std::unordered_map<void*, void*> hooks = {
#ifdef PATCHER_PLAYER
    { &RBX::Network::initWithPlayerSecurity_orig, RBX::Network::initWithPlayerSecurity_hook },

    // ===== `RBX::Network::ClientReplicator` member function hooks =====
    { &RBX::Network::ClientReplicator__canUseProtocolVersion_orig, RBX::Network::ClientReplicator__canUseProtocolVersion_hook },

    // ===== `RBX::Network::Replicator::RockyItem` member function hooks =====
    { &RBX::Network::Replicator__RockyItem__write_orig, RBX::Network::Replicator__RockyItem__write_hook },
#endif

    // ===== `RBX::ContentId` member function hooks =====
    { &RBX::ContentId__convertToLegacyContent_orig, RBX::ContentId__convertToLegacyContent_hook },

    // // ===== `RBX::ContentProvider` member function hooks =====
    { &RBX::ContentProvider__verifyScriptSignature_orig, RBX::ContentProvider__verifyScriptSignature_hook },
    { &RBX::ContentProvider__verifyRequestedScriptSignature_orig, RBX::ContentProvider__verifyRequestedScriptSignature_hook },

    // ===== `RBX::DataModel` member function hooks =====
    { &RBX::DataModel__startCoreScripts_orig, RBX::DataModel__startCoreScripts_hook },

    // ===== `RBX::ScriptContext` member function hooks =====
    { &RBX::ScriptContext__openState_orig, RBX::ScriptContext__openState_hook },

    // ===== `RBX::Http` member function hooks =====
    { &RBX::Http__trustCheck_orig, RBX::Http__trustCheck_hook },

    // ===== `RBX::HeartbeatTask` member function hooks =====
    { &RBX::HeartbeatTask__constructor_orig, RBX::HeartbeatTask__constructor_hook },

    // ===== `RBX::RunService` member function hooks =====
    { &RBX::RunService__step_orig, RBX::RunService__step_hook },

    // ===== `RBX::NetworkSettings` member function hooks =====
    { &RBX::NetworkSettings__setDataSendRate_orig, RBX::NetworkSettings__setDataSendRate_hook },
    { &RBX::NetworkSettings__setPhysicsSendRate_orig, RBX::NetworkSettings__setPhysicsSendRate_hook },
    { &RBX::NetworkSettings__setReceiveRate_orig, RBX::NetworkSettings__setReceiveRate_hook },

#ifdef PATCHER_PLAYER
    // ===== `CRobloxWnd::RenderRequestJob` member function hooks =====
    { &CRobloxWnd__RenderRequestJob__sleepTime_orig, CRobloxWnd__RenderRequestJob__sleepTime_hook },

    // ===== `CRobloxWnd::UserInputJob` member function hooks =====
    { &CRobloxWnd__UserInputJob__sleepTime_orig, CRobloxWnd__UserInputJob__sleepTime_hook },
#endif

#ifdef PATCHER_STUDIO
    // ===== `RobloxView::RenderRequestJob` member function hooks =====
    { &RobloxView__RenderRequestJob__sleepTime_orig, RobloxView__RenderRequestJob__sleepTime_hook },
#endif

    // ===== other hooks =====
    { &invalidRequestCheck_orig, invalidRequestCheck_hook },
    { &scriptHashCheck_orig, scriptHashCheck_hook },
#ifdef PATCHER_STUDIO
    { &computePropNullDerefFix_orig, computePropNullDerefFix_hook },
#endif

    { &getPhysicsStepsPerSec_orig, getPhysicsStepsPerSec_hook },
    { &getSecsPerPhysicsStep_orig, getSecsPerPhysicsStep_hook },
    { &motor6dJointFpsFix_ptr, motor6dJointFpsFix_hook },
    { &motorJointFpsFix_ptr, motorJointFpsFix_hook },
    { &InternetOpenA_orig, InternetOpenA_hook },
};

static std::runtime_error patchError(const char* format, ...)
{

    va_list args;
    va_start(args, format);

    int size = vsnprintf(nullptr, 0, format, args);

    std::string buffer;
    buffer.resize(size);

    vsnprintf(&buffer[0], size + 1, format, args);

    va_end(args);

    return std::runtime_error(buffer);
}

static void initHooks()
{
    LONG error = DetourTransactionBegin();
    if (error != NO_ERROR)
        throw patchError("DetourTransactionBegin returned %d", error);

    for (auto& hook : hooks)
    {
        auto ptr = reinterpret_cast<void**>(hook.first);

        error = DetourAttach(ptr, hook.second);
        if (error != NO_ERROR)
            throw patchError("Couldn't hook function at 0x%p! DetourAttach returned %d", *ptr, error);
    }

    error = DetourTransactionCommit();
    if (error != NO_ERROR)
        throw patchError("DetourTransactionCommit returned %d", error);
}

static void writeBytes(void* address, const char* data, size_t size, DWORD flags)
{
    DWORD oldFlags;
    if (!VirtualProtect(address, size, flags, &oldFlags))
        throw patchError("VirtualProtect failed to change protection flags for 0x%p - 0x%p (1)", address, reinterpret_cast<DWORD>(address) + size);

    std::memcpy(address, data, size);

    if (!VirtualProtect(address, size, oldFlags, &oldFlags))
        throw patchError("VirtualProtect failed to change protection flags for 0x%p - 0x%p (2)", address, reinterpret_cast<DWORD>(address) + size);
}

template<typename T>
static void writeValue(void* address, T value, DWORD flags)
{
    writeBytes(address, reinterpret_cast<const char*>(&value), sizeof(T), flags);
}

static void fillBytes(void* address, uint8_t value, size_t size, DWORD flags)
{
    DWORD oldFlags;
    if (!VirtualProtect(address, size, flags, &oldFlags))
        throw patchError("VirtualProtect failed to change protection flags for 0x%p - 0x%p (1)", address, reinterpret_cast<DWORD>(address) + size);

    std::memset(address, value, size);

    if (!VirtualProtect(address, size, oldFlags, &oldFlags))
        throw patchError("VirtualProtect failed to change protection flags for 0x%p - 0x%p (2)", address, reinterpret_cast<DWORD>(address) + size);
}

void Patches::init()
{
    // ===== bypass certificate pinning =====
    // write a jump within RBX::Http::httpGetPostWinInet to bypass the check
    writeBytes(reinterpret_cast<void*>(ADDRESS_CERT_PIN_JUMP), "\xE9\xE3\x02\x00\x00", 5, PAGE_EXECUTE_READWRITE);

#ifdef PATCHER_STUDIO
    // ===== remove serverplaceid parameter from character appearances =====
    fillBytes(reinterpret_cast<void*>(ADDRESS_SERVERPLACEID_1), 0x90, 0x49, PAGE_EXECUTE_READWRITE);
    fillBytes(reinterpret_cast<void*>(ADDRESS_SERVERPLACEID_2), 0x90, 0x33, PAGE_EXECUTE_READWRITE);
#endif

#ifdef PATCHER_PLAYER
    // ===== disable ProgramMemoryChecker =====
    // SECURITY BYPASS
    // write a RET to the start of RBX::ProgramMemoryCheckerImpl::step
    fillBytes(reinterpret_cast<void*>(ADDRESS_PROGRAMMEMORYCHECKERIMPL_STEP), 0xC3, 1, PAGE_EXECUTE_READWRITE);
#endif

    // ===== SSAO fix =====
    // replace JL instruction with JB following the 128mb VRAM check
    // this makes FRM treat VRAM as unsigned to fix the SSAO toggle
    fillBytes(reinterpret_cast<void*>(ADDRESS_FRM_SSAO_VRAM_JUMP), 0x72, 1, PAGE_EXECUTE_READWRITE);

    // ===== TextBox:CaptureFocus fix =====
    // changes the required security level in the static initializer
    // for the CaptureFocus func descriptor from RobloxScriptSecurity to None
    fillBytes(reinterpret_cast<void*>(ADDRESS_TEXTBOX_CAPTUREFOCUS_SECURITY), 0, 1, PAGE_EXECUTE_READWRITE);

    initHooks();
}
