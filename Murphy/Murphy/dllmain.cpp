// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <cstdio>
#include <cstdint>

namespace Steam
{
    namespace Offsets
    {
        // Steam Overlay 15/08/2022
        uint64_t SteamPresentHook = 0x88E30;
        uint64_t SteamOriginalPresent = 0x1405C8;
        uint64_t HookFunc = 0x82BB0;
    }

    __int64(__fastcall * OrigPresent)(void*, __int64, __int64);
    __int64 __fastcall PresentHook(void* swap_chain, __int64 sync_interval, __int64 flags)
    {
        printf("Steam present hooked! %p, %llx, %llx\n", swap_chain, sync_interval, flags);
        return OrigPresent(swap_chain, sync_interval, flags);
    }

    bool HookSteamOverlay()
    {
        /*
        Steam::HookFunction(
        *(_QWORD *)(DXGIVTable + 64),
        Present_sub_88E30,
        &Steam::Globals::OriginalPresent,
        1i64);
        */
        auto base = (uint64_t)GetModuleHandleA("GameOverlayRenderer64.dll");
        if (!base) return false;

        auto hook_function = reinterpret_cast<__int64(__fastcall*)(uint64_t, uint64_t, uint64_t*, uint64_t)>(0);
        *(uint64_t*)&hook_function = base + Offsets::HookFunc;

        hook_function(base + Offsets::SteamPresentHook, (uint64_t)PresentHook, (uint64_t*)&OrigPresent, 1);
        return true;
    }
}

namespace Discord
{
    namespace Offsets
    {
        // Discord Overlay 15/08/2022
        uint64_t DiscordSwapChainPresent = 0x170C99;
    }

    __int64(__fastcall* OrigPresent)(void*, __int64, __int64);
    __int64 __fastcall PresentHook(void* swap_chain, __int64 sync_interval, __int64 flags)
    {
        printf("Discord present hooked! %p, %llx, %llx\n", swap_chain, sync_interval, flags);
        return OrigPresent(swap_chain, sync_interval, flags);
    }

    bool HookDiscordOverlay()
    {
        auto base = (uint64_t)GetModuleHandleA("DiscordHook64.dll");
        if (!base) return false;
    }
}


void __stdcall OnAttach(HMODULE hmodule)
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    printf("Injected into %d\n", GetCurrentProcessId());

    if (!Steam::HookSteamOverlay)
    {
        printf("Failed to hook steam overlay!");
        return;
    }

    if (!Discord::HookDiscordOverlay)
    {
        printf("Failed to hook discord overlay!");
        return;
    }

    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 1)
        {
            break;
        }
    }

    printf("Goodbye\n");
    FreeLibraryAndExitThread(hmodule, 0);
}


BOOL __stdcall DllMain( HMODULE hmodule, DWORD  reason, void*)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(0, 0, (PTHREAD_START_ROUTINE)OnAttach, hmodule, 0, 0));
        break;
    }
    return TRUE;
}

