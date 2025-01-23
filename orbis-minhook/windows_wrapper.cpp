#include "windows_wrapper.h"

int convert_to_sce_protection(DWORD flProtect)
{
	int protection = 0;
	if (flProtect & PAGE_NOACCESS)
	{
		protection = PROT_NONE;
	}
	else
	{
		if (flProtect & PAGE_READONLY)
		{
			protection |= PROT_READ;
		}
		if (flProtect & PAGE_READWRITE)
		{
			protection |= PROT_READ | PROT_WRITE;
		}
		if (flProtect & PAGE_EXECUTE)
		{
			protection |= PROT_EXEC;
		}
		if (flProtect & PAGE_EXECUTE_READ)
		{
			protection |= PROT_READ | PROT_EXEC;
		}
		if (flProtect & PAGE_EXECUTE_READWRITE)
		{
			protection |= PROT_READ | PROT_WRITE | PROT_EXEC;
		}
	}
	return protection;
}

DWORD convert_to_win_protection(int protection)
{
	DWORD flProtect = 0;
	if (protection & PROT_NONE)
	{
		flProtect = PAGE_NOACCESS;
	}
	else
	{
		if (protection & PROT_READ)
		{
			if (protection & PROT_WRITE)
			{
				flProtect = PAGE_READWRITE;
			}
			else
			{
				flProtect = PAGE_READONLY;
			}
		}
		if (protection & PROT_EXEC)
		{
			if (protection & PROT_READ)
			{
				if (protection & PROT_WRITE)
				{
					flProtect = PAGE_EXECUTE_READWRITE;
				}
				else
				{
					flProtect = PAGE_EXECUTE_READ;
				}
			}
			else
			{
				flProtect = PAGE_EXECUTE;
			}
		}
	}
	return flProtect;
}

// wrapper for sceKernelVirtualQuery
SIZE_T VirtualQuery(LPVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
	// we need to replicate the results of VirtualQuery on windows.

	SceKernelVirtualQueryInfo pageInfo{};
	if (int res = sceKernelVirtualQuery(lpAddress, 0, &pageInfo, sizeof(SceKernelVirtualQueryInfo)) < 0)
	{
		printf("sceKernelVirtualQuery failed: %d\n", res);
	}

	lpBuffer->BaseAddress = pageInfo.start;
	lpBuffer->AllocationBase = reinterpret_cast<PVOID>(pageInfo.offset);
	lpBuffer->AllocationProtect = pageInfo.protection;
	lpBuffer->PartitionId = 0;
	lpBuffer->RegionSize = reinterpret_cast<uint64_t>(pageInfo.end) - reinterpret_cast<uint64_t>(pageInfo.start);
	lpBuffer->State = pageInfo.isCommitted ? MEM_COMMIT : MEM_FREE;
	lpBuffer->Protect = pageInfo.protection;
	lpBuffer->Type = pageInfo.memoryType;

	return sizeof(SceKernelVirtualQueryInfo);
}

LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	// we need to replicate the results of VirtualAlloc on windows.

	void* addr = nullptr;
	if (int res = sceKernelMapFlexibleMemory(&addr, dwSize, flProtect, 0) < 0)
	{
		printf("sceKernelMapNamedFlexibleMemory failed: %d\n", res);
	}

	return addr;
}

BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
	// we need to replicate the results of VirtualFree on windows.

	if (int res = sceKernelMunmap(lpAddress, dwSize) < 0)
	{
		printf("sceKernelMunmap failed: %d\n", res);
		return FALSE;
	}
	return TRUE;
}

BOOL VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
{
	// we need to replicate the results of VirtualProtect on windows.

	int oldProt = 0;
	int res = sceKernelQueryMemoryProtection(lpAddress, 0, 0, &oldProt);
	if (res < 0)
	{
		printf("sceKernelMprotect failed: %d\n", res);
		return FALSE;
	}

	if (lpflOldProtect)
		*lpflOldProtect = convert_to_win_protection(oldProt);

	auto newProt = convert_to_sce_protection(flNewProtect);

	if (int res = sceKernelMprotect(lpAddress, dwSize, flNewProtect) < 0)
	{
		printf("sceKernelMprotect failed: %d\n", res);
		return FALSE;
	}
	return TRUE;
}

VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
	size_t count;
	SceKernelModuleInfo moduleInfo;

	int res = sceKernelGetModuleInfo(0, &moduleInfo);
	if (res < 0)
	{
		printf("sceKernelGetModuleInfo failed: %d\n", res);
	}

	uint64_t address = reinterpret_cast<uint64_t>(moduleInfo.segmentInfo->address);

	lpSystemInfo->lpMinimumApplicationAddress = reinterpret_cast<PVOID>(address);
	lpSystemInfo->lpMaximumApplicationAddress = reinterpret_cast<PVOID>(address + moduleInfo.segmentInfo->size);
	lpSystemInfo->dwPageSize = PAGE_SIZE;
}

LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
	// we need to replicate the results of HeapAlloc on windows.

	void* addr = nullptr;
	if (int res = sceKernelMapFlexibleMemory(&addr, dwBytes, PROT_READ | PROT_WRITE, 0) < 0)
	{
		printf("sceKernelMapNamedFlexibleMemory failed: %d\n", res);
	}
	return addr;
}

BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
	// we need to replicate the results of HeapFree on windows.
	if (int res = sceKernelMunmap(lpMem, 0) < 0)
	{
		printf("sceKernelMunmap failed: %d\n", res);
		return FALSE;
	}
	return TRUE;
}

LPVOID HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes)
{
	// To replicate the behavior of HeapReAlloc, we need to allocate new memory,
	// copy the data from the old memory block, and then free the old block.

	void* newAddr = nullptr;

	// Allocate new memory with the requested size
	if (int res = sceKernelMapFlexibleMemory(&newAddr, dwBytes, PROT_READ | PROT_WRITE, 0) < 0)
	{
		printf("sceKernelMapFlexibleMemory failed: %d\n", res);
		return nullptr;
	}

	// If the old memory pointer is valid, copy the data to the new block and free the old one
	if (lpMem != nullptr)
	{
		// Determine the size of the old block (assuming we can somehow know its size).
		// This step may vary depending on the actual implementation details of the allocation system.
		// For now, we'll assume we somehow have access to the size of the old block.
		size_t oldSize = 0; // Replace this with an appropriate method to get the old block size.

		// Copy the old data to the new block (only up to the minimum of the two sizes)
		memcpy(newAddr, lpMem, oldSize < dwBytes ? oldSize : dwBytes);

		// Free the old block
		if (sceKernelMunmap(lpMem, 0) < 0)
		{
			printf("sceKernelMunmap failed while freeing old memory.\n");
		}
	}

	return newAddr;
}

// GetThreadContext
BOOL GetThreadContext(HANDLE hThread, LPCONTEXT lpContext)
{
	SceDbgUcontext ucontext;
	pthread_get_user_context_np(hThread, &ucontext);

	lpContext->Rdi = ucontext.

	return TRUE;
}