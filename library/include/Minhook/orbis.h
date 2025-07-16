#pragma once

template <typename T = uintptr_t>
T GetAbsoluteAddress(uintptr_t address, uintptr_t base = -1)
{
	char debugMsg[128];
	auto result = reinterpret_cast<T>((base == -1 ? 0x400000 : base) + address);
	snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetAbsoluteAddress: address=%p, base=%p, result=%p\n",
			 (void *)address, (void *)base, (void *)result);
	sceKernelDebugOutText(0, debugMsg);
	return result;
}

template <typename T = uintptr_t>
T GetRelativeAddress(uintptr_t address, uintptr_t base)
{
	char debugMsg[128];
	auto result = reinterpret_cast<T>(address - base);
	snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetRelativeAddress: address=%p, base=%p, result=%p\n",
			 (void *)address, (void *)base, (void *)result);
	sceKernelDebugOutText(0, debugMsg);
	return result;
}

template <typename T = uintptr_t>
T GetModuleAddress(std::string module)
{
	char debugMsg[128];
	snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetModuleAddress: Loading module %s\n", module.c_str());
	sceKernelDebugOutText(0, debugMsg);

	int library = sceKernelLoadStartModule(module.data(), 0, nullptr, 0, nullptr, nullptr);
	if (library > 0)
	{
		OrbisKernelModuleInfo moduleInfo;
		moduleInfo.size = sizeof(OrbisKernelModuleInfo);
		if (sceKernelGetModuleInfo(library, &moduleInfo) == 0)
		{
			snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetModuleAddress: Module %s loaded at %p\n",
					 module.c_str(), moduleInfo.segmentInfo[0].address);
			sceKernelDebugOutText(0, debugMsg);
			return reinterpret_cast<T>(moduleInfo.segmentInfo[0].address);
		}
		else
		{
			snprintf(debugMsg, sizeof(debugMsg), "[ERROR] GetModuleAddress: Failed to get module info for %s\n",
					 module.c_str());
			sceKernelDebugOutText(0, debugMsg);
		}
	}
	else
	{
		snprintf(debugMsg, sizeof(debugMsg), "[ERROR] GetModuleAddress: Failed to load module %s\n", module.c_str());
		sceKernelDebugOutText(0, debugMsg);
	}
	return {};
}

template <typename T = uintptr_t>
T GetExport(int handle, std::string function)
{
	char debugMsg[128];
	snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetExport: Resolving function %s from handle %d\n",
			 function.c_str(), handle);
	sceKernelDebugOutText(0, debugMsg);

	uint64_t libraryFunctionAddr;
	if (sceKernelDlsym(handle, function.data(), (void **)&libraryFunctionAddr) == 0)
	{
		snprintf(debugMsg, sizeof(debugMsg), "[DEBUG] GetExport: Function %s resolved at 0x%" PRIX64 "\n",
				 function.c_str(), libraryFunctionAddr);
		sceKernelDebugOutText(0, debugMsg);
		return (T)libraryFunctionAddr;
	}
	else
	{
		snprintf(debugMsg, sizeof(debugMsg), "[ERROR] GetExport: Failed to resolve function %s\n", function.c_str());
		sceKernelDebugOutText(0, debugMsg);
	}
	return 0;
}

thread *GetByName(const char *name);
thread *GetById(uint32_t id);
