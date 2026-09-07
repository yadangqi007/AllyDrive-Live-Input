#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <xinput.h>
#include <cstdio>

using GetStateFn = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
using SetStateFn = DWORD (WINAPI*)(DWORD, XINPUT_VIBRATION*);

int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 2;
    HMODULE proxy = LoadLibraryW(argv[1]);
    if (!proxy) {
        std::printf("LoadLibrary failed: %lu\n", GetLastError());
        return 3;
    }
    auto get_state = reinterpret_cast<GetStateFn>(GetProcAddress(proxy, "XInputGetState"));
    auto set_state = reinterpret_cast<SetStateFn>(GetProcAddress(proxy, "XInputSetState"));
    if (!get_state || !set_state) {
        std::puts("Missing XInput exports");
        return 4;
    }
    XINPUT_STATE state{};
    const DWORD result = get_state(0, &state);
    if (result != ERROR_SUCCESS && result != ERROR_DEVICE_NOT_CONNECTED) {
        std::printf("Unexpected XInput result: %lu\n", result);
        return 5;
    }
    std::printf("Proxy loaded and forwarded XInput (result=%lu)\n", result);
    FreeLibrary(proxy);
    return 0;
}
