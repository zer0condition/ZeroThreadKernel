#include "ZeroUtils.h"

NTSTATUS ZeroUtils::MdlRtlCopyMemory(PVOID Destination, const VOID * Source, SIZE_T Length) {
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

NTSTATUS ZeroUtils::MdlRtlFillMemory(PVOID Destination, BYTE Value, SIZE_T Length) {
    PMDL Mdl = IoAllocateMdl(Destination, Length, 0, 0, 0);
    if (!Mdl) return STATUS_UNSUCCESSFUL;

    MmBuildMdlForNonPagedPool(Mdl);
    PVOID Mapped = (PVOID)MmMapLockedPages(Mdl, KernelMode);
    if (!Mapped) {
        IoFreeMdl(Mdl);
        return STATUS_UNSUCCESSFUL;
    }

    KIRQL OldIrql = KeRaiseIrqlToDpcLevel();
    RtlFillMemory(Mapped, Length, Value);
    KeLowerIrql(OldIrql);

    MmUnmapLockedPages(Mapped, Mdl);
    IoFreeMdl(Mdl);

    return STATUS_SUCCESS;
}

PVOID ZeroUtils::GetKernelModuleExport(const char* ModuleName, const char* FunctionName)
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
        if (strcmp((char*)(Module[i].OffsetToFileName + Module[i].FullPathName), ModuleName) == 0)
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