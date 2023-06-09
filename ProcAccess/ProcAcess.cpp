#include <ntddk.h>
#include "ProcAccessPublic.h"

void OnUnload(PDRIVER_OBJECT);
NTSTATUS OnCreateClose(PDEVICE_OBJECT, PIRP Irp);
NTSTATUS OnDeviceControl(PDEVICE_OBJECT, PIRP Irp);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING ) {
	UNREFERENCED_PARAMETER(DriverObject);

	DriverObject->DriverUnload = OnUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] =
		DriverObject->MajorFunction[IRP_MJ_CLOSE] =
		OnCreateClose;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
		OnDeviceControl;

	UNICODE_STRING devName;
	PDEVICE_OBJECT devObj;
	RtlInitUnicodeString(&devName, L"\\Device\\ProcAccess");
	auto status = IoCreateDevice(DriverObject, 0, &devName,
		FILE_DEVICE_UNKNOWN, 0, FALSE, &devObj);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	UNICODE_STRING symName = 
		RTL_CONSTANT_STRING(L"\\??\\ProcAccess");
	status = IoCreateSymbolicLink(&symName, &devName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(devObj);
		return status;
	}

	return STATUS_SUCCESS;
}

void OnUnload(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symName =
		RTL_CONSTANT_STRING(L"\\??\\ProcAccess");
	IoDeleteSymbolicLink(&symName);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS OnCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return STATUS_SUCCESS;
}

NTSTATUS OnDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
	auto& dic = irpSp->Parameters.DeviceIoControl;
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG len = 0;
	switch (dic.IoControlCode) {
		case IOCTL_OPEN_PROCESS:
			if (dic.InputBufferLength < sizeof(ULONG) || dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			auto pid = *(ULONG*)Irp->AssociatedIrp.SystemBuffer;
			HANDLE hProcess;
			CLIENT_ID cid{};
			cid.UniqueProcess = UlongToHandle(pid);
			OBJECT_ATTRIBUTES attr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, 0);
			status = ZwOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &attr, &cid);
			if (NT_SUCCESS(status)) {
				*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hProcess;
				len = sizeof(HANDLE);
			}

			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, 0);
	return status;
}
