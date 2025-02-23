#include "structs.h"
#include <orbis/Sysmodule.h>
#include <GoldHEN.h>
#include <minhook.h>

attr_public const char *g_pluginName = "minhook_prefixmodz_example";
attr_public const char *g_pluginDesc = "Example usage of the Orbis port of Minhook.";
attr_public const char *g_pluginAuth = "TsudaKageyu, Faultz, ItsJokerZz, Prefixmodz";
attr_public uint32_t g_pluginVersion = 0x00000100;

pthread_t start_thread;
pthread_t stop_thread;

const float box_alien_color_tint_clr[4] = { 0.0f, 0.0f, 0.0f, 0.4f };
const float recolor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

// Call Of Duty Ghost default_mp 1.21 

MH_STATUS MinHook_Status;

#define Material_RegisterHandle_a 0x0000000000981320
#define R_AddCmdDrawQuadPic_a 0x0000000000993BF0

typedef Material* (*Material_RegisterHandle_t)(const char*, int);
Material_RegisterHandle_t Material_RegisterHandle = Material_RegisterHandle_t(Material_RegisterHandle_a);

typedef void(*R_AddCmdDrawQuadPic_o)(float verts[4][2], const float* color, Material* material);
R_AddCmdDrawQuadPic_o R_AddCmdDrawQuadPic_orig = nullptr;

void R_AddCmdDrawQuadPic_Hook(float verts[4][2], const float* clr, Material* material)
{
	const char* materialName = material->info.name;
	if (!strcmp(materialName, "bkgd_title_bar") ||

		!strcmp(materialName, "frontend_mp_alpha_shards_mid") ||
		!strcmp(materialName, "frontend_mp_alpha_shards_fore") ||
		!strcmp(materialName, "frontend_mp_alpha_art") ||
		!strcmp(materialName, "frontend_sq_art") ||
		!strcmp(materialName, "frontend_aliens_art") ||
		!strcmp(materialName, "frontend_sp_alpha_shards") ||
		!strcmp(materialName, "frontend_mp_add_glints") ||
		!strcmp(materialName, "preview_mp_alien_town_blur") ||
		!strcmp(materialName, "card_bg") ||
		!strcmp(materialName, "card_bg_001") ||
		!strcmp(materialName, "card_image_darken") ||
		!strcmp(materialName, "card_shape_01") ||
		!strcmp(materialName, "icon_alien_rank_pvt1") ||
		!strcmp(materialName, "box_alien_header_footer") ||
		!strcmp(materialName, "box_white_gradient_bot") ||
		!strcmp(materialName, "box_white_gradient_fade_rt") ||
		!strcmp(materialName, "box_dropshadow_ltcap") ||
		!strcmp(materialName, "box_dropshadow_hori_spacer") ||
		!strcmp(materialName, "box_dropshadow_midcap") ||
		!strcmp(materialName, "box_dropshadow_vert_spacer") ||
		!strcmp(materialName, "box_dropshadow_rtcap") ||
		!strcmp(materialName, "btn_00a_ltcap") ||
		!strcmp(materialName, "btn_00a_mid") ||
		!strcmp(materialName, "btn_00a_rtcap") ||
		!strcmp(materialName, "btn_00a_ltcap_f") ||
		!strcmp(materialName, "btn_00a_mid_f") ||
		!strcmp(materialName, "btn_00a_rtcap_f") ||
		!strcmp(materialName, "btn_alien_00a_ltcap") ||
		!strcmp(materialName, "btn_alien_00a_mid") ||
		!strcmp(materialName, "btn_alien_00a_rtcap") ||
		!strcmp(materialName, "btn_alien_00a_ltcap_f") ||
		!strcmp(materialName, "btn_alien_00a_mid_f") ||
		!strcmp(materialName, "btn_alien_00a_rtcap_f") ||
		!strcmp(materialName, "btn_alien_overlay_02"))
		clr = recolor;

	if (!strcmp(materialName, "box_alien_color_tint"))
		clr = box_alien_color_tint_clr;

	//// Swap Materials
	if (!strcmp(materialName, "frontend_mp_alpha_art") ||
		!strcmp(materialName, "frontend_sq_art") ||
		!strcmp(materialName, "frontend_aliens_art"))
		material = Material_RegisterHandle("preview_mp_alien_town_blur", 0);

	if (!strcmp(materialName, "frontend_mp_alpha_shards_mid") ||
		!strcmp(materialName, "frontend_mp_alpha_shards_fore") ||
		!strcmp(materialName, "frontend_sp_alpha_shards"))
		material = Material_RegisterHandle("frontend_mp_add_glints", 0);

	//printf("[R_AddCmdDrawQuadPic_Hook]: Material->Info.Name: %s\n", materialName);

	return R_AddCmdDrawQuadPic_orig(verts, clr, material);
}

void *main_start(void *)
{
  sceKernelDebugOutText(0, "[DEBUG] main_start()\n");

  MinHook_Status = MH_Initialize();

  MinHook_Status = MH_CreateHook((LPVOID)R_AddCmdDrawQuadPic_a, (PVOID)R_AddCmdDrawQuadPic_Hook, (LPVOID*)&R_AddCmdDrawQuadPic_orig);

  MinHook_Status = MH_EnableHook(MH_ALL_HOOKS);

  pthread_exit(nullptr);
  return nullptr;
}

void* main_stop(void*)
{
    sceKernelDebugOutText(0, "[DEBUG] main_stop()\n");

    MinHook_Status = MH_RemoveHook((LPVOID)R_AddCmdDrawQuadPic_a);

    MinHook_Status = MH_DisableHook(MH_ALL_HOOKS);

    MinHook_Status = MH_Uninitialize();

    pthread_exit(nullptr);
    return nullptr;
}

void init()
{
  sceKernelLoadStartModule("libkernel.sprx", 0, nullptr, 0, nullptr, nullptr);

  pthread_create(&start_thread, nullptr, main_start, nullptr);
  pthread_join(start_thread, nullptr);
}

void fini()
{
  pthread_create(&stop_thread, nullptr, main_stop, nullptr);
  pthread_join(stop_thread, nullptr);
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
