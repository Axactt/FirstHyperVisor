#include<ntifs.h> // For inluding all header such as ntddk.h and ntdef.h
#include<ntddk.h>
#include"Logger.h"  // for logging of error an dinformation to DbgPrint


void MyHyperVisorUnload(_In_ PDRIVER_OBJECT DriverObject)
{
	LogInfo("MyHyperVisor Unload called\n");
	//?? Create the symbolic link to target device_object name or driver_name
   //? Symbolic link variable  created again in this function as ealier creation was in local scope  of DriverEntry function
	UNICODE_STRING symLink{};
	RtlInitUnicodeString(&symLink, L"\\??\\MyHyperVisor");

	//! delete the symbolic link
	IoDeleteSymbolicLink(&symLink);

	//! delete the device Object
	IoDeleteDevice(DriverObject->DeviceObject);

}

extern "C" NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	Log(LogLevel::Information, "DriverEntry called. Registry Path: %wZ\n", RegistryPath);
	DriverObject->DriverUnload = MyHyperVisorUnload;

	//DriverObject->MajorFunction[IRP_MJ_CREATE] = MyHyperVisor

	// NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT DeviceObject = NULL;
	UNICODE_STRING deviceName{};
	UNICODE_STRING symLink{};
    
	//! Assign the Driver Device name with name of our driver. 
	//? It is basically name of the driver by which we communicate from user mode
	RtlInitUnicodeString(&deviceName, L"\\Device\\MyHyperVisor");
	

	//Create the device object with the Device name Unicode string
	NTSTATUS status = IoCreateDevice(

		DriverObject, // Oue driver object
		0,           // no need for extra bytes
		&deviceName,  //the device name created under \Devivce Object manager dir
		FILE_DEVICE_UNKNOWN, // device type
		FILE_DEVICE_SECURE_OPEN,     //characteristics flags
		FALSE,  // not exclusive
	   &DeviceObject); // the resulting pointer to the Device_object

	if (!NT_SUCCESS(status)) // error checking 
	{
		LogError("Failed to create device object(0x%08X)\n", status);
		return status;
	}
	else
	{
		//?  Following flags added in HyperVisor driver tutorial needs to be checked for purpose
		DeviceObject->Flags |= IO_TYPE_DEVICE;          // Setting the bits using Bitwise |= OR operator
		DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING); // cLEARING the bits using bitwise Bitwise AND and Bitwise NOT together:

	}

	NT_ASSERT(DeviceObject); //todo why this used to find out

	//! Assign the unicode string variable for symbolicLink 
	RtlInitUnicodeString(&symLink, L"\\??\\MyHyperVisor");
	
	//!This creates the symboliclink and connect it to our target device_object
	//! Uses the Unicode string variable earlier initialized for symbolic-link and device-object name
	//! 
	status = IoCreateSymbolicLink(&symLink, &deviceName); 

	if (!NT_SUCCESS(status))
	{
		LogError("Failed to create Symbolic Link (0x%X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	NT_ASSERT(NT_SUCCESS(status));
	return STATUS_SUCCESS;

}




