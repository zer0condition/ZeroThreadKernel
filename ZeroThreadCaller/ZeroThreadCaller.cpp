#include <Windows.h>
#include <iostream>

/* Keep this running, it serves as a "system thread" for the loop in the hook */
#define STARTZEROTHREAD 0x1337

typedef __int64(*NtCreateCompositionSurfaceHandle_t)(__int64 a1, unsigned int a2, unsigned __int64 a3);
NtCreateCompositionSurfaceHandle_t oNtCreateCompositionSurfaceHandle;

int main()
{
    HMODULE hModule = LoadLibraryA("win32u.dll");

    if (hModule) {
        oNtCreateCompositionSurfaceHandle = (NtCreateCompositionSurfaceHandle_t)GetProcAddress(hModule, "NtCreateCompositionSurfaceHandle");
        printf("NtCreateCompositionSurfaceHandle: %p\n", oNtCreateCompositionSurfaceHandle);

        getchar();

        oNtCreateCompositionSurfaceHandle((int)STARTZEROTHREAD, NULL, NULL);

        getchar();
    }
    return 0;
}