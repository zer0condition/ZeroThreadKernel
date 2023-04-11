#include <ntifs.h>
#include <stdarg.h>
#include <windef.h>
#include "ZeroThreadKernel.h"

#define STARTZEROTHREAD 0x1337

typedef __int64(*NtCreateCompositionSurfaceHandle_t)(__int64 a1, unsigned int a2, unsigned __int64 a3);
NtCreateCompositionSurfaceHandle_t oNtCreateCompositionSurfaceHandle;
NtCreateCompositionSurfaceHandle_t TrampolineNtCreateCompositionSurfaceHandle;
SIZE_T oNtCreateCompositionSurfaceHandleBytes;

int Times = 0;

__int64 __fastcall hkNtCreateCompositionSurfaceHandle(__int64 a1, unsigned int a2, unsigned __int64 a3)
{
	Print("[ZeroThreadKernel] hkNtCreateCompositionSurfaceHandle!");

	if ((int)a1 == (int)STARTZEROTHREAD) /* Sanity check. */
	{
		while (true) {
			Print("[ZeroThreadKernel] %i", Times);
			Times++;

		}
	}

	return TrampolineNtCreateCompositionSurfaceHandle(a1, a2, a3);
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	ZeroThreadKernel::UnhookFunction((PVOID)TrampolineNtCreateCompositionSurfaceHandle, (PVOID)oNtCreateCompositionSurfaceHandle, oNtCreateCompositionSurfaceHandleBytes);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING DriverName) 
{
	DriverObject->DriverUnload = DriverUnload;

	/* Store the original  */
	oNtCreateCompositionSurfaceHandle = (NtCreateCompositionSurfaceHandle_t)GetKernelModuleExport("dxgkrnl.sys", "NtCreateCompositionSurfaceHandle");
	Print("NtCreateCompositionSurfaceHandle: %p", oNtCreateCompositionSurfaceHandle);

	TrampolineNtCreateCompositionSurfaceHandle = (NtCreateCompositionSurfaceHandle_t)ZeroThreadKernel::HookFunction((PVOID)oNtCreateCompositionSurfaceHandle, (PVOID)hkNtCreateCompositionSurfaceHandle, &oNtCreateCompositionSurfaceHandleBytes);

	return STATUS_SUCCESS;
}