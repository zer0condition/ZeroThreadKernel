#pragma once
#include <ntifs.h>
#include <stdarg.h>
#include <windef.h>

extern "C" NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(PVOID ImageBase, PCCH RoutineName);
extern "C" NTKERNELAPI NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);

#include "ZeroUtils/ZeroUtils.h"
#include "ZeroHook/ZeroHook.h"