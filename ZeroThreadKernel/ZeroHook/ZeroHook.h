#include "../ZeroUtils/ZeroUtils.h"

namespace ZeroHook
{
    SIZE_T GetHookSize(PVOID Function);
    PVOID HookFunction(PVOID OriginalFunction, PVOID HookFunction, SIZE_T* HookLength);
    NTSTATUS UnhookFunction(PVOID Trampoline, PVOID OriginalFunction, SIZE_T HookLength);
}