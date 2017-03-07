#include "luafuncs_core_paths.h"
#include <data_configs/config_manager.h>

std::string Binding_Core_GlobalFuncs_Paths::path_commongfx()
{
    return ConfigManager::PathCommonGFX();
}

std::string Binding_Core_GlobalFuncs_Paths::path_level_block()
{
    return ConfigManager::PathLevelBlock();
}

std::string Binding_Core_GlobalFuncs_Paths::path_level_bgo()
{
    return ConfigManager::PathLevelBGO();
}

std::string Binding_Core_GlobalFuncs_Paths::path_level_npc()
{
    return ConfigManager::PathLevelNPC();
}

std::string Binding_Core_GlobalFuncs_Paths::path_effect()
{
    return ConfigManager::PathLevelEffect();
}

std::string Binding_Core_GlobalFuncs_Paths::path_world_tile()
{
    return ConfigManager::PathWorldTiles();
}

std::string Binding_Core_GlobalFuncs_Paths::path_world_scene()
{
    return ConfigManager::PathWorldScenery();
}

std::string Binding_Core_GlobalFuncs_Paths::path_world_path()
{
    return ConfigManager::PathWorldPaths();
}

std::string Binding_Core_GlobalFuncs_Paths::path_world_level()
{
    return ConfigManager::PathWorldLevels();
}

std::string Binding_Core_GlobalFuncs_Paths::path_music()
{
    return ConfigManager::PathMusic();
}

std::string Binding_Core_GlobalFuncs_Paths::path_sound()
{
    return ConfigManager::PathSound();
}

luabind::scope Binding_Core_GlobalFuncs_Paths::bindToLua()
{
    using namespace luabind;
    return
        namespace_("Paths")[
            def("CommonGFX",   &path_commongfx),
            def("LevelBlocks", &path_level_block),
            def("LevelBGOs",   &path_level_bgo),
            def("LevelNPCs",   &path_level_npc),
            def("WorldTiles",  &path_world_tile),
            def("WorldScenery",&path_world_scene),
            def("WorldPath",   &path_world_path),
            def("WorldLevel",  &path_world_level),
            def("EffectGFX",   &path_commongfx),
            def("Music",  &path_music),
            def("Sound",  &path_sound)
            ];
}
