#ifndef ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea
#define ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea

#include <pthread.h>

extern "C"{
#include "config.h"

#define FRAME_MODNAME "ActFrame"

#define ACTFRM_STATE_IDLE 0
#define ACTFRM_STATE_RUN 1
#define ACTFRM_STATE_PAUSE 2

class CActFrame
{
public:
    CActFrame();
    ~CActFrame();

    int InitFrame();
    int Run();
    int Pause(int iGo);
    int Stop();

    static void *ThreadFunc(void *arg);
    void *MainThread();
    //int GetStatus


protected:
    int m_iModID;
    int m_iState;
    pthread_t m_MainThread;
    class CActNode *m_pNode;
};






}
#endif
