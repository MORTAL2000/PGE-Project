#include "../common_features/pge_texture.h"
#include "config_manager.h"

#include "../common_features/graphics_funcs.h"

#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

#include <QtDebug>

ConfigManager::ConfigManager()
{
}

DataFolders      ConfigManager::dirs;
QString          ConfigManager::config_dir;
QString          ConfigManager::data_dir;

QStringList ConfigManager::errorsList;

//Common Data
unsigned int ConfigManager::screen_width=800;
unsigned int ConfigManager::screen_height=600;

ConfigManager::screenType ConfigManager::screen_type = ConfigManager::SCR_Static;

//Loading Screen setings
LoadingScreenData ConfigManager::LoadingScreen;

//World map settings
WorldMapData ConfigManager::WorldMap;

QVector<PGE_Texture > ConfigManager::common_textures;

unsigned long ConfigManager::music_custom_id;
unsigned long ConfigManager::music_w_custom_id;
QVector<obj_music > ConfigManager::main_music_lvl;
QVector<obj_music > ConfigManager::main_music_wld;
QVector<obj_music > ConfigManager::main_music_spc;

QVector<obj_sound > ConfigManager::main_sound;


//Level config Data
QVector<obj_block >     ConfigManager::lvl_blocks;
QMap<long, obj_block>   ConfigManager::lvl_block_indexes;
CustomDirManager ConfigManager::Dir_Blocks;


QVector<PGE_Texture >   ConfigManager::level_textures; //Texture bank



QVector<PGE_Texture > ConfigManager::world_textures;


QString ConfigManager::imgFile, ConfigManager::imgFileM;
QString ConfigManager::tmpstr;
QStringList ConfigManager::tmp;

unsigned long ConfigManager::total_data;
QString ConfigManager::bgoPath;
QString ConfigManager::BGPath;
QString ConfigManager::blockPath;
QString ConfigManager::npcPath;
QString ConfigManager::effectPath;

QString ConfigManager::tilePath;
QString ConfigManager::scenePath;
QString ConfigManager::pathPath;
QString ConfigManager::wlvlPath;

QString ConfigManager::commonGPath;



long  ConfigManager::getBlockTexture(long blockID)
{
    if(!lvl_block_indexes.contains(blockID))
    {
        return -1;
    }

    if(lvl_block_indexes[blockID].isInit)
    {

        if(lvl_block_indexes[blockID].textureArrayId < level_textures.size())
            return lvl_block_indexes[blockID].textureArrayId;
        else
            return -1;
    }
    else
    {
        QString imgFile = Dir_Blocks.getCustomFile(lvl_block_indexes[blockID].image_n);
        QString maskFile = Dir_Blocks.getCustomFile(lvl_block_indexes[blockID].mask_n);

        PGE_Texture texture;
        texture.w = 0;
        texture.h = 0;
        texture.texture = 0;
        texture.texture_layout = NULL;
        texture.format = 0;
        texture.nOfColors = 0;

        long id = level_textures.size();

        lvl_block_indexes[blockID].textureArrayId = id;

        level_textures.push_back(texture);

        GraphicsHelps::loadTexture( level_textures[id],
             imgFile,
             maskFile
             );

        //qDebug()<< blockID << "Loading..." << level_textures[id].texture;

        lvl_block_indexes[blockID].image = &(level_textures[id]);
        lvl_block_indexes[blockID].textureID = level_textures[id].texture;
        lvl_block_indexes[blockID].isInit = true;

        return id;
    }
}




void ConfigManager::setConfigPath(QString p)
{
    config_dir = ApplicationPath + "/" +  "configs/" + p + "/";
}

bool ConfigManager::loadBasics()
{
    QString gui_ini = config_dir + "main.ini";
    QSettings guiset(gui_ini, QSettings::IniFormat);
    guiset.setIniCodec("UTF-8");

    guiset.beginGroup("main");
        data_dir = (guiset.value("application-dir", "0").toBool() ?
                        ApplicationPath + "/" : config_dir + "data/" );
        guiset.endGroup();


    total_data=0;
    errorsList.clear();

    //dirs
    if((!QDir(config_dir).exists())||(QFileInfo(config_dir).isFile()))
    {
        QMessageBox::critical(NULL, "Config error",
                              QString("CONFIG DIR NOT FOUND AT: %1").arg(config_dir),
                              QMessageBox::Ok);
        return false;
    }

    QString main_ini = config_dir + "main.ini";

    if(!QFileInfo(main_ini).exists())
    {
        QMessageBox::critical(NULL, "Config error",
                              QString("Can't open the 'main.ini' config file!"),
                              QMessageBox::Ok);
        return false;
    }

    QSettings mainset(main_ini, QSettings::IniFormat);
    mainset.setIniCodec("UTF-8");

    mainset.beginGroup("main");

        data_dir = (mainset.value("application-dir", false).toBool() ?
                        ApplicationPath + "/" : config_dir + "data/" );

        dirs.worlds = data_dir + mainset.value("worlds", "worlds").toString() + "/";

        dirs.music = data_dir + mainset.value("music", "data/music").toString() + "/";
        dirs.sounds = data_dir + mainset.value("sound", "data/sound").toString() + "/";

        dirs.glevel = data_dir + mainset.value("graphics-level", "data/graphics/level").toString() + "/";
        dirs.gworld= data_dir + mainset.value("graphics-worldmap", "data/graphics/worldmap").toString() + "/";
        dirs.gplayble = data_dir + mainset.value("graphics-characters", "data/graphics/characters").toString() + "/";

        dirs.gcommon = data_dir + mainset.value("graphics-common", "data-custom").toString() + "/";

        dirs.gcustom = data_dir + mainset.value("custom-data", "data-custom").toString() + "/";
    mainset.endGroup();

    if( mainset.status() != QSettings::NoError )
    {
        QMessageBox::critical(NULL, "Config error",
                              QString("ERROR LOADING main.ini N:%1").arg(mainset.status()),
                              QMessageBox::Ok);
        return false;
    }


    ////////////////////////////////Preparing////////////////////////////////////////
    bgoPath =   dirs.glevel +  "background/";
    BGPath =    dirs.glevel +  "background2/";
    blockPath = dirs.glevel +  "block/";
    npcPath =   dirs.glevel +  "npc/";
    effectPath= dirs.glevel +  "effect/";

    tilePath =  dirs.gworld +  "tile/";
    scenePath = dirs.gworld +  "scene/";
    pathPath =  dirs.gworld +  "path/";
    wlvlPath =  dirs.gworld +  "level/";

    commonGPath = dirs.gcommon + "/";
    //////////////////////////////////////////////////////////////////////////////////

    QString engine_ini = config_dir + "engine.ini";

    if(!QFileInfo(engine_ini).exists())
    {
        QMessageBox::critical(NULL, "Config error",
                              QString("Can't open the 'engine.ini' config file!"),
                              QMessageBox::Ok);
        return false;
    }

    QSettings engineset(engine_ini, QSettings::IniFormat);
    engineset.setIniCodec("UTF-8");



    engineset.beginGroup("common");
        screen_width = engineset.value("screen-width", 800).toInt();
        screen_height = engineset.value("screen-height", 600).toInt();
        QString scrType = engineset.value("screen-type", "static").toString();

        if(scrType == "dynamic")
            screen_type = SCR_Dynamic;
        else
            screen_type = SCR_Static;
    engineset.endGroup();



    ////// World map settings

    engineset.beginGroup("world-map");
        WorldMap.backgroundImg = engineset.value("background", "").toString();
        WorldMap.viewport_x = engineset.value("viewport-x", "").toInt();
        WorldMap.viewport_y = engineset.value("viewport-y", "").toInt();
        WorldMap.viewport_w = engineset.value("viewport-w", "").toInt();
        WorldMap.viewport_h = engineset.value("viewport-h", "").toInt();

        WorldMap.title_x = engineset.value("level-title-x", "").toInt();
        WorldMap.title_y = engineset.value("level-title-y", "").toInt();
        WorldMap.title_w = engineset.value("level-title-w", "").toInt();

        QString ttlAlign = engineset.value("level-title-align", "left").toString();
        if(ttlAlign=="center")
            WorldMap.title_align = WorldMapData::align_center;
        else
        if(ttlAlign=="right")
            WorldMap.title_align = WorldMapData::align_right;
        else
            WorldMap.title_align = WorldMapData::align_left;

        WorldMap.points_en = engineset.value("points-counter", "").toBool();
        WorldMap.points_x = engineset.value("points-counter-x", "").toInt();
        WorldMap.points_y = engineset.value("points-counter-y", "").toInt();

        WorldMap.health_en = engineset.value("health-counter", "").toBool();
        WorldMap.health_x = engineset.value("health-counter-x", "").toInt();
        WorldMap.health_y = engineset.value("health-counter-y", "").toInt();

        WorldMap.star_en = engineset.value("star-counter", "").toBool();
        WorldMap.star_x = engineset.value("star-counter-x", "").toInt();
        WorldMap.star_y = engineset.value("star-counter-y", "").toInt();

        WorldMap.coin_en = engineset.value("coin-counter", "").toBool();
        WorldMap.coin_x = engineset.value("coin-counter-x", "").toInt();
        WorldMap.coin_y = engineset.value("coin-counter-y", "").toInt();

        WorldMap.portrait_en = engineset.value("portrait", "").toBool();
        WorldMap.portrait_x = engineset.value("portrait-x", "").toInt();
        WorldMap.portrait_y = engineset.value("portrait-y", "").toInt();

    engineset.endGroup();


    ////////// Loading scene settings

    int LoadScreenImages=0;
    engineset.beginGroup("loading-scene");
        LoadingScreen.bg_color_r = engineset.value("bg-color-r", 0).toInt();
        LoadingScreen.bg_color_g = engineset.value("bg-color-g", 0).toInt();
        LoadingScreen.bg_color_b = engineset.value("bg-color-b", 0).toInt();
        LoadingScreen.backgroundImg = engineset.value("background", "").toString();
        LoadingScreen.updateDelay = engineset.value("updating-time", 128).toInt();
        LoadScreenImages = engineset.value("additional-images", 0).toInt();
    engineset.endGroup();


    LoadingScreen.AdditionalImages.clear();
    for(int i=1; i<=LoadScreenImages; i++)
    {
        engineset.beginGroup(QString("loading-image-%1").arg(i));
        LoadingScreenAdditionalImage img;

        img.imgFile = engineset.value("image", "").toString();
        img.animated = engineset.value("animated", false).toBool();
        if(img.animated)
            img.frames = engineset.value("animated", 1).toInt();
        else
            img.frames = 1;
        img.x =  engineset.value("pos-x", 1).toInt();
        img.y =  engineset.value("pos-y", 1).toInt();
        LoadingScreen.AdditionalImages.push_back(img);

        engineset.endGroup();
    }

    return true;
}

void ConfigManager::addError(QString bug, QtMsgType level)
{
    Q_UNUSED(level);
    errorsList<<bug;
}



bool ConfigManager::unloadLevelConfigs()
{

    lvl_block_indexes.clear();
    lvl_blocks.clear();

    while(!level_textures.isEmpty())
    {
        glDeleteTextures( 1, &(level_textures.first().texture) );
        level_textures.pop_front();
    }

    //level_textures.clear();
    return true;
}
