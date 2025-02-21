#include <orbis/Sysmodule.h>
#include <GoldHEN.h>
#include <minhook.h>

attr_public const char *g_pluginName = "minhook_example";
attr_public const char *g_pluginDesc = "Example usage of the Orbis port of Minhook.";
attr_public const char *g_pluginAuth = "TsudaKageyu, Faultz, ItsJokerZz";
attr_public uint32_t g_pluginVersion = 0x00000100;

pthread_t thread;

void (*functionToHook_o)();
void hookFunction()
{
  sceKernelDebugOutText(0, "[DEBUG] function hooked: functionToHook()->hookFunction()\n");

  ((void (*)())functionToHook_o)();
}

void functionToHook()
{
  sceKernelDebugOutText(0, "[DEBUG] original function with no hooking: functionToHook()\n");
}

void *main_start(void *)
{
  sceKernelDebugOutText(0, "[DEBUG] main_start()\n");

  // Initialize MinHook.
  if (MH_Initialize() != MH_OK)
    return 0;

  // Hook functionToHook
  if (MH_CreateHook((LPVOID)functionToHook, (LPVOID)hookFunction, reinterpret_cast<void **>(&functionToHook_o)) != MH_OK)
    return 0;

  // Enable the hook for functionToHook
  if (MH_EnableHook((LPVOID)functionToHook) != MH_OK)
    return 0;

  // Call functionToHook
  functionToHook();

  // Disable the hook for functionToHook
  if (MH_DisableHook((LPVOID)functionToHook) != MH_OK)
    return 0;

  // Uninitialize MinHook.
  if (MH_Uninitialize() != MH_OK)
    return 0;

  pthread_exit(nullptr);
  return nullptr;
}

void init()
{
  sceKernelLoadStartModule("libkernel.sprx", 0, nullptr, 0, nullptr, nullptr);

  pthread_create(&thread, nullptr, main_start, nullptr);
  pthread_join(thread, nullptr);
}

void fini()
{
  pthread_cancel(thread);
  pthread_join(thread, nullptr);

  MH_DisableHook(MH_ALL_HOOKS);
  MH_Uninitialize();
}

extern "C"
{
  s32 attr_public plugin_load(s32, const char *[])
  {
    sceKernelDebugOutText(0, "[DEBUG] plugin_load()\n");
    init();
    return 0;
  }

  s32 attr_public plugin_unload(s32, const char *[])
  {
    sceKernelDebugOutText(0, "[DEBUG] plugin_unload()\n");
    fini();
    return 0;
  }

  s32 attr_module_hidden module_start(s64, const void *)
  {
    sceKernelDebugOutText(0, "[DEBUG] module_start()\n");
    init();
    return 0;
  }

  s32 attr_module_hidden module_stop(s64, const void *)
  {
    sceKernelDebugOutText(0, "[DEBUG] module_stop()\n");
    fini();
    return 0;
  }
}
