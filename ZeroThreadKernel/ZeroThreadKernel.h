#pragma once
#include "ZeroThreadUtils.h"

namespace ZeroThreadKernel
{
    BYTE JMPShell[] = {
        /*
        * jmp RIP        ; JMP to RIP
        * dq 0           ; ABS Address
        * dq 0           ; ABS Address
        */

        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    SIZE_T GetHookSize(PVOID Function)
    {
        SIZE_T Length = sizeof(JMPShell);
        while (true) {
            if (*(BYTE*)((PBYTE)Function + Length) == 0x45) break; // 0x45 (MOV)
            if (*(BYTE*)((PBYTE)Function + Length) == 0x48) break; // 0x48 (MOV)
            if (*(BYTE*)((PBYTE)Function + Length) == 0xC3) break; // 0xC3 (RET)
            Length++;
        };
        return Length;
    }

    PVOID HookFunction(PVOID OriginalFunction, PVOID HookFunction, SIZE_T* HookLength)
    {
        if (!OriginalFunction) {
            Print("[HookFunction] Failed to get function");
            return NULL;
        }

        *HookLength = GetHookSize(OriginalFunction);
        
        PVOID Trampoline = ExAllocatePool(NonPagedPoolExecute, *HookLength + sizeof(JMPShell));
        if (!Trampoline) {
            Print("[HookFunction] Failed to allocate trampoline");
            return NULL;
        }

        if (!NT_SUCCESS(MdlRtlCopyMemory(Trampoline, OriginalFunction, *HookLength))) {
            Print("[HookFunction] Failed to copy function to trampoline");
            ExFreePoolWithTag(Trampoline, 0);
            return NULL;
        }

        *(PBYTE*)&JMPShell[6] = (PBYTE)OriginalFunction + *HookLength;
        if (!NT_SUCCESS(MdlRtlCopyMemory((PBYTE)Trampoline + *HookLength, &JMPShell[0], sizeof(JMPShell)))) {
            Print("[HookFunction] Failed to write JMPShell on trampoline");
            ExFreePoolWithTag(Trampoline, 0);
            return NULL;
        }

        *(PBYTE*)&JMPShell[6] = (PBYTE)HookFunction;
        if (!NT_SUCCESS(MdlRtlCopyMemory(OriginalFunction, &JMPShell[0], sizeof(JMPShell)))) {
            Print("[HookFunction] Failed to write JMPShell");
            ExFreePoolWithTag(Trampoline, 0);
            return NULL;
        }

        if (sizeof(JMPShell) > *HookLength) { /* NOP the remaining bytes */
            if (!NT_SUCCESS(MdlRtlFillMemory((PBYTE)OriginalFunction + sizeof(JMPShell), 0x90 /* (NOP) */, *HookLength - sizeof(JMPShell)))) {
                Print("[HookFunction] Failed to NOP the remaining bytes");
            }
        }

        Print("[HookFunction] Function Hooked");

        return Trampoline;
    }

    NTSTATUS UnhookFunction(PVOID Trampoline, PVOID OriginalFunction, SIZE_T HookLength)
    {
        if (!NT_SUCCESS(MdlRtlCopyMemory(OriginalFunction, Trampoline, HookLength))) {
            Print("[UnhookFunction] Failed to restore original bytes");
            return STATUS_UNSUCCESSFUL;
        }

        ExFreePoolWithTag(Trampoline, 0);

        Print("[UnhookFunction] Function Unhooked");
    }
}