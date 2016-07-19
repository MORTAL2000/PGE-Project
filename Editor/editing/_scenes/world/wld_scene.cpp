/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2016 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common_features/app_path.h>
#include <common_features/themes.h>
#include <editing/edit_world/world_edit.h>

#include "wld_scene.h"
#include "items/item_tile.h"
#include "items/item_scene.h"
#include "items/item_path.h"
#include "items/item_level.h"
#include "items/item_music.h"

#include "edit_modes/wld_mode_hand.h"
#include "edit_modes/wld_mode_select.h"
#include "edit_modes/wld_mode_erase.h"
#include "edit_modes/wld_mode_place.h"
#include "edit_modes/wld_mode_square.h"
#include "edit_modes/wld_mode_circle.h"
#include "edit_modes/wld_mode_line.h"
#include "edit_modes/wld_mode_fill.h"
#include "edit_modes/wld_mode_resize.h"
#include "edit_modes/wld_mode_setpoint.h"

WldScene::WldScene(MainWindow *mw,
                   GraphicsWorkspace * parentView,
                   dataconfigs &configs,
                   WorldData &FileData,
                   QObject *parent) :
    QGraphicsScene(parent),
    m_mw(mw),
    m_configs(&configs), // Pointer to Main Configs
    m_data(&FileData), //Add pointer to level data
    m_viewPort(parentView),
    m_subWindow(nullptr)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    if(parent)
    {
        if(strcmp(parent->metaObject()->className(), "WorldEdit")==0)
        {
            m_subWindow = qobject_cast<WorldEdit*>(parent);
        }
    }

    //Editing mode
    m_editMode = 0;
    m_eraserIsEnabled = false;
    m_pastingMode = false;
    m_disableMoveItems = false;
    m_busyMode = false;

    m_mouseLeftPressed = false; //Left mouse key is pressed
    m_mouseMidPressed = false;  //Middle mouse key is pressed
    m_mouseRightPressed = false;//Right mouse key is pressed

    m_mouseIsMoved  = false; //Mouse was moved with right mouseKey

    m_skipChildMousePressEvent  = false;
    m_skipChildMouseMoveEvent   = false;
    m_skipChildMousReleaseEvent = false;

    m_lastTerrainArrayID    = 0;
    m_lastSceneryArrayID    = 0;
    m_lastPathArrayID       = 0;
    m_lastLevelArrayID      = 0;
    m_lastMusicBoxArrayID   = 0;

    isSelectionDialog = false;

    //Editing process flags
    m_mouseIsMovedAfterKey = false;
    //haveSelected = false;

    m_emptyCollisionCheck = false;

    m_placingItemType = 0;

    m_resizeBox = NULL;

    m_contextMenuIsOpened = false;

    selectedPoint = QPoint(0, 0);
    selectedPointNotUsed = true;
    pointTarget = nullptr;
    pointImg = QPixmap(":/images/set_point.png");

    pointAnimation.setSettings(pointImg, true, 4, 64);
    animator.registerAnimation(&pointAnimation);

    m_cursorItemImg = NULL;
    resetCursor();
    m_labelBox = NULL;

    //set dummy images if target not exist or wrong
    m_dummyTerrainImg   = Themes::Image(Themes::dummy_terrain);
    m_dummySceneryImg   = Themes::Image(Themes::dummy_scenery);
    m_dummyPathImg      = Themes::Image(Themes::dummy_path);
    m_dummyLevelImg     = Themes::Image(Themes::dummy_wlevel);

    m_musicBoxImg = Themes::Image(Themes::dummy_musicbox);

    //Build animators for dummies
    SimpleAnimator * tmpAnimator;
        tmpAnimator = new SimpleAnimator(m_dummyTerrainImg, 0);
    m_animatorsTerrain.push_back( tmpAnimator );
        tmpAnimator = new SimpleAnimator(m_dummySceneryImg, 0);
    m_animatorsScenery.push_back( tmpAnimator );
        tmpAnimator = new SimpleAnimator(m_dummyPathImg, 0);
    m_animatorsPaths.push_back( tmpAnimator );
        tmpAnimator = new SimpleAnimator(m_dummyLevelImg, 0);
    m_animatorsLevels.push_back( tmpAnimator );

    //Init default rotation tables
    local_rotation_table_tiles.clear();
    local_rotation_table_sceneries.clear();
    local_rotation_table_paths.clear();
    local_rotation_table_levels.clear();
    foreach(obj_rotation_table x, m_configs->main_rotation_table)
    {
        if(x.type==ItemTypes::WLD_Tile)
            local_rotation_table_tiles[x.id]=x;
        else
        if(x.type==ItemTypes::WLD_Scenery)
            local_rotation_table_sceneries[x.id]=x;
        else
        if(x.type==ItemTypes::WLD_Path)
            local_rotation_table_paths[x.id]=x;
        else
        if(x.type==ItemTypes::WLD_Level)
            local_rotation_table_levels[x.id]=x;
    }


    //set Default Z Indexes
    tileZ=0; // tiles
    sceneZ=5; // scenery
    pathZ=10; // paths
    levelZ=15; // levels
    musicZ=20; // musicboxes

    //HistoryIndex
    historyIndex=0;

    //Locks
    m_lockTerrain=false;
    m_lockScenery=false;
    m_lockPath=false;
    m_lockLevel=false;
    m_lockMusicBox=false;

    connect(this, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

    long padding=100000;

    QGraphicsRectItem * bigRect = addRect(-padding, -padding, padding*2, padding*2, QPen(Qt::transparent), QBrush(Qt::transparent));
    bigRect->setZValue(-10000000000);


    //Build edit mode classes
    WLD_ModeHand * modeHand = new WLD_ModeHand(this);
    m_editModes.push_back(modeHand);

    WLD_ModeSelect * modeSelect = new WLD_ModeSelect(this);
    m_editModes.push_back(modeSelect);

    WLD_ModeResize * modeResize = new WLD_ModeResize(this);
    m_editModes.push_back(modeResize);

    WLD_ModeErase * modeErase = new WLD_ModeErase(this);
    m_editModes.push_back(modeErase);

    WLD_ModePlace * modePlace = new WLD_ModePlace(this);
    m_editModes.push_back(modePlace);

    WLD_ModeRect * modeSquare = new WLD_ModeRect(this);
    m_editModes.push_back(modeSquare);

    WLD_ModeCircle * modeCircle = new WLD_ModeCircle(this);
    m_editModes.push_back(modeCircle);

    WLD_ModeLine * modeLine = new WLD_ModeLine(this);
    m_editModes.push_back(modeLine);

    WLD_ModeSetPoint * modeSetPoint = new WLD_ModeSetPoint(this);
    m_editModes.push_back(modeSetPoint);

    WLD_ModeFill * modeFill = new WLD_ModeFill(this);
    m_editModes.push_back(modeFill);

    m_editModeObj = modeSelect;
    m_editModeObj->set();
}

WldScene::~WldScene()
{
    if(m_labelBox) delete m_labelBox;
    while(!m_editModes.isEmpty())
    {
        EditMode *tmp = m_editModes.first();
        m_editModes.pop_front();
        delete tmp;
    }
}

void WldScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);
    if(!opts.grid_show) return;

    int gridSize=m_configs->default_grid;
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);

    QVarLengthArray<QLineF, 100> lines;
    for (qreal x = left; x < rect.right(); x += gridSize)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setOpacity(0.5);
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine));
    painter->drawLines(lines.data(), lines.size());
    painter->setPen(QPen(QBrush(Qt::white), 1, Qt::DashLine));
    painter->drawLines(lines.data(), lines.size());
}
