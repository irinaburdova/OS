#include "efi.h"
#include "efilib.h"
                   
EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	InitializeLib(image, systab);

	EFI_MEMORY_DESCRIPTOR *MemoryMap;
	UINTN DescriptorSize, MapKey, MappingSize, MemoryMapSize, index;
	UINT32 DescriptorVersion;
	EFI_MEMORY_DESCRIPTOR *Descriptor;
	UINT64 PageSize, Size;
	PageSize = 4096;

	EFI_STATUS err = EFI_SUCCESS;
	int i = 0;
	
	MemoryMapSize = 0;
	err = uefi_call_wrapper(systab->BootServices->GetMemoryMap, 5, &MemoryMapSize, NULL, NULL, NULL, NULL);
	if (err == EFI_BUFFER_TOO_SMALL) {
		err = uefi_call_wrapper(systab->BootServices->AllocatePool, 3, EfiLoaderData, MemoryMapSize, &MemoryMap);
		if (err != EFI_SUCCESS) {
			Print(L"Error: Failed to allocate pool for memory map");
			return err;
		}
		err = uefi_call_wrapper(systab->BootServices->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
		if (err != EFI_SUCCESS) {
			if (err == EFI_BUFFER_TOO_SMALL) {
				Print(L"Error: buffer to small \r\n");			
				uefi_call_wrapper(systab->BootServices->FreePool, 1, &MemoryMap);
			
			}
			Print(L"Error: Coudn't get memory map \r\n");
		}

		Print(L"Memory Map Size: %d\n", MemoryMapSize);
		Print(L"Map Key: %d\n", MapKey);
		Print(L"Descriptor Version: %d\n", DescriptorVersion);
		Print(L"Descriptor Size: %d\n\n", DescriptorSize);
		
		MappingSize = 0;
		Size = MemoryMapSize / DescriptorSize;
		for (i = 0; i < Size; i++){
			switch(MemoryMap[i].Type){
				case EfiConventionalMemory : MappingSize += MemoryMap[i].NumberOfPages * PageSize / 1048576;
				case EfiBootServicesCode : MappingSize += MemoryMap[i].NumberOfPages * PageSize / 1048576;
				case EfiBootServicesData : MappingSize += MemoryMap[i].NumberOfPages * PageSize / 1048576;
			}

		}
		Print(L"Total ьemory available for general use: %d Mb\r\n",MappingSize);

			
	}else{
		Print(L"Error: Everything is bad\r\n");
	}
	
	
	
	Print(L"Hit any key to continue\r\n");
	uefi_call_wrapper(systab->BootServices->WaitForEvent, 3, 1, &systab->ConIn->WaitForKey, &index);

	uefi_call_wrapper(systab->BootServices->FreePool, 1, MemoryMap);

	return EFI_SUCCESS;
}

