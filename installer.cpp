#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

static bool process_running(const wchar_t* wanted) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool found = false;
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (lstrcmpiW(entry.szExeFile, wanted) == 0) { found = true; break; }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return found;
}

static bool write_resource(WORD id, const wchar_t* destination) {
    HRSRC resource = FindResourceW(nullptr, MAKEINTRESOURCEW(id), MAKEINTRESOURCEW(10));
    if (!resource) return false;
    HGLOBAL loaded = LoadResource(nullptr, resource);
    const void* bytes = loaded ? LockResource(loaded) : nullptr;
    const DWORD size = SizeofResource(nullptr, resource);
    if (!bytes || !size) return false;

    wchar_t temporary[MAX_PATH]{};
    lstrcpyW(temporary, destination);
    lstrcatW(temporary, L".allydrive-new");
    HANDLE file = CreateFileW(temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    const bool ok = WriteFile(file, bytes, size, &written, nullptr) && written == size;
    FlushFileBuffers(file);
    CloseHandle(file);
    if (!ok || !MoveFileExW(temporary, destination,
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary);
        return false;
    }
    return true;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    const wchar_t* game = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Automobilista 2\\";
    wchar_t exe[MAX_PATH]{}, dll[MAX_PATH]{}, ini[MAX_PATH]{}, backup[MAX_PATH]{};
    lstrcpyW(exe, game); lstrcatW(exe, L"AMS2AVX.exe");
    lstrcpyW(dll, game); lstrcatW(dll, L"XINPUT9_1_0.dll");
    lstrcpyW(ini, game); lstrcatW(ini, L"allydrive.ini");

    if (process_running(L"AMS2AVX.exe")) {
        MessageBoxW(nullptr, L"Cierra Automobilista 2 y vuelve a ejecutar el instalador.",
                    L"AllyDrive Live Input", MB_OK | MB_ICONWARNING);
        return 1;
    }
    if (GetFileAttributesW(exe) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(nullptr, L"No se encontro AMS2AVX.exe en la carpeta esperada de Steam.",
                    L"AllyDrive Live Input", MB_OK | MB_ICONERROR);
        return 2;
    }

    const bool previous_allydrive = GetFileAttributesW(ini) != INVALID_FILE_ATTRIBUTES;
    if (GetFileAttributesW(dll) != INVALID_FILE_ATTRIBUTES && !previous_allydrive) {
        lstrcpyW(backup, dll); lstrcatW(backup, L".allydrive-backup");
        if (GetFileAttributesW(backup) == INVALID_FILE_ATTRIBUTES) CopyFileW(dll, backup, TRUE);
    }
    if (!write_resource(101, dll) || !write_resource(102, ini)) {
        MessageBoxW(nullptr, L"Windows no pudo copiar los archivos del mod.",
                    L"AllyDrive Live Input", MB_OK | MB_ICONERROR);
        return 3;
    }

    MessageBoxW(nullptr,
        L"AllyDrive Live Input 0.4.0 se instalo correctamente.\n\n"
        L"La direccion y la chase camera se aplicaran automaticamente al entrar a pista.",
        L"Instalacion completada", MB_OK | MB_ICONINFORMATION);
    return 0;
}
