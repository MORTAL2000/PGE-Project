#ifndef LVL_BLOCK_H
#define LVL_BLOCK_H

#include "../../physics/base_object.h"
#include "../../data_configs/obj_block.h"

#include <file_formats.h>

class LVL_Block : public PGE_Phys_Object
{
public:
    LVL_Block();
    ~LVL_Block();
    void init();

    LevelBlock* data; //Local settings
    bool slippery;

    obj_block * setup;//Global config

    //float posX();
    //float posY();
};

#endif // LVL_BLOCK_H
