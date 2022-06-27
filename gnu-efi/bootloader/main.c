#include <efi.h>
#include <efilib.h>
#include <elf.h>

typedef unsigned long long size_t;

typedef struct 
{
	void* BaseAddress;
	size_t BufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int PixelsPerScanLine;
} FrameBuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct
{
	PSF1_HEADER* header;
	void* glyphBuffer;
} PSF1_FONT;


FrameBuffer framebuffer;



FrameBuffer* InitializeGOP()
{
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);

	if(EFI_ERROR(status))
	{
		Print(L"GOP Not Found :((\n\r");
		return NULL; 
	}
	else
	{
		Print(L"GOP Found\n\r");
	}

	framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	framebuffer.BufferSize = gop->Mode->FrameBufferSize;
	framebuffer.width = gop->Mode->Info->HorizontalResolution;
	framebuffer.height = gop->Mode->Info->VerticalResolution;
	framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;

}




EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
	EFI_FILE* LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if (Directory == NULL){
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS){
		return NULL;
	}
	return LoadedFile;

}

PSF1_FONT* loadFont(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* fontFile = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if(fontFile == NULL) return NULL;

	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData,sizeof(PSF1_HEADER),(void**)&fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	fontFile->Read(fontFile,&size,fontHeader);

	if(fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1)
	{
		Print(L"Font file not correct format");
		return NULL;
	}
	UINTN glyphBufferSize = fontHeader->charsize*256;
	if(fontHeader->mode == 1)
	{
		glyphBufferSize = fontHeader->charsize*512;
	}

	void* glyphBuffer;
	{
		fontFile->SetPosition(fontFile,sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData,glyphBufferSize,(void**)&glyphBuffer);
		fontFile->Read(fontFile,&glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->header = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;
	return finishedFont;
}

int memcmp(const void* aptr, const void* bptr, size_t n){
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);
	Print(L"Love you chloe \n\r");

	EFI_FILE* Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if (Kernel == NULL){
		Print(L"Could not load kernel \n\r");
	}
	else{
		Print(L"Kernel Loaded Successfully \n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	)
	{
		Print(L"kernel format is bad\r\n");
	}
	else
	{
		Print(L"kernel header successfully verified\r\n");
	}

	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for (
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type){
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}
		}
	}

	Print(L"Kernel Loaded\n\r");
	
	int (*KernelStart)(FrameBuffer*, PSF1_FONT*, int) = ((__attribute__((sysv_abi)) int (*)(FrameBuffer*, PSF1_FONT*, int) ) header.e_entry);

	FrameBuffer* newBuff = InitializeGOP();

	Print(L"Base: %x\n\rSize: %x\n\rWidth: %d\n\rHeight: %d\n\rPPS: %d\n\r",
	newBuff->BaseAddress,
	newBuff->BufferSize,
	newBuff->width,
	newBuff->height,
	newBuff->PixelsPerScanLine
	);

	unsigned int y = 100;
	unsigned int BBP = 4;
	
	PSF1_FONT* font = loadFont(NULL, L"Uni3-TerminusBoldVGA14.psf", ImageHandle, SystemTable);

	if(font == NULL)
	{
		Print(L"Font could not be loaded\n\r");
	}
	else
	{
		Print(L"Font Loaded Successfully!\n\r");
		
	}

	//Print(L"%d\r\n", KernelStart(newBuff, font));
	KernelStart(newBuff, font, 0);
	// Somehow, the kernel returned. Show panic screen
	KernelStart(newBuff, font, 1);

	return EFI_SUCCESS; // Exit the UEFI application
}
