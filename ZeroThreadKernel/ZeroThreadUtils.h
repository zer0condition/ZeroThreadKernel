#pragma once

#define Print( content, ... ) DbgPrintEx( 0, 0, "[>] " content, __VA_ARGS__ )

extern "C" NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(PVOID ImageBase, PCCH RoutineName);
extern "C" NTKERNELAPI NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);

NTSTATUS MdlRtlCopyMemory(PVOID Destination, const VOID* Source, SIZE_T Length) {
	PMDL Mdl = IoAllocateMdl(Destination, (ULONG)Length, 0, 0, 0);
	if (!Mdl) return STATUS_UNSUCCESSFUL;

	MmBuildMdlForNonPagedPool(Mdl);
	PVOID Mapped = MmMapLockedPages(Mdl, KernelMode);
	if (!Mapped) {
		IoFreeMdl(Mdl);
		return STATUS_UNSUCCESSFUL;
	}

	KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
	RtlCopyMemory(Mapped, Source, Length);
	KeLowerIrql(OldIrql);

	MmUnmapLockedPages(Mapped, Mdl);
	IoFreeMdl(Mdl);

	return STATUS_SUCCESS;
}

NTSTATUS MdlRtlFillMemory(PVOID Destination, BYTE Value, SIZE_T Length) {
	PMDL Mdl = IoAllocateMdl(Destination, Length, 0, 0, 0);
	if (!Mdl) return STATUS_UNSUCCESSFUL;

	MmBuildMdlForNonPagedPool(Mdl);
	PVOID Mapped = (PVOID)MmMapLockedPages(Mdl, KernelMode);
	if (!Mapped) {
		IoFreeMdl(Mdl);
		return STATUS_UNSUCCESSFUL;
	}

	KIRQL KIRQL = KeRaiseIrqlToDpcLevel();
	RtlFillMemory(Mapped, Length, Value);
	KeLowerIrql(KIRQL);

	MmUnmapLockedPages(Mapped, Mdl);
	IoFreeMdl(Mdl);

	return STATUS_SUCCESS;
}

typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

PVOID GetKernelModuleExport(const char* ModuleName, const char* FunctionName)
{
    PVOID ModuleBase = 0;

    ULONG Bytes = 0;
    NTSTATUS Status = ZwQuerySystemInformation(SystemModuleInformation, NULL, Bytes, &Bytes);

    if (Bytes == 0) {
        Print("[GetKernelModuleExport] ZwQuerySystemInformation[1] failed");
        return NULL;
    }

    PRTL_PROCESS_MODULES Modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, Bytes, 'NeiH');

    Status = ZwQuerySystemInformation(SystemModuleInformation, Modules, Bytes, &Bytes);

    if (!NT_SUCCESS(Status)) {
        Print("[GetKernelModuleExport] ZwQuerySystemInformation[2] failed");
        return NULL;
    }

    PRTL_PROCESS_MODULE_INFORMATION Module = Modules->Modules;

    for (ULONG i = 0; i < Modules->NumberOfModules; i++)
    {
        if (strcmp((char*)Module[i].FullPathName, ModuleName) == 0)
        {
            ModuleBase = Module[i].ImageBase;
            break;
        }
    }

    if (Modules) { 
        ExFreePoolWithTag(Modules, 'NeiH');
    }

    if (ModuleBase == 0) {
        Print("[GetKernelModuleExport] Failed to get module base");
        return NULL; 
    }

    return RtlFindExportedRoutineByName(ModuleBase, FunctionName);
}