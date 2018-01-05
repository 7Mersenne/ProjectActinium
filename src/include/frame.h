#ifndef ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea
#define ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea

#include "config.h"

#define FRAME_MODNAME "ActFrame"

class CActFrame
{
public:
    CActFrame();
    ~CActFrame();

    int InitFrame();


protected:
    int m_iModID;
    class CActConfig *m_pConfig;
    class CActNode *m_pNode;
};







#endif