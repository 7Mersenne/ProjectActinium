#ifndef ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea
#define ACTINIUM_FRAME_H_44d22393_e378_4a45_a1eb_584ae67e41ea

#include <pthread.h>


//extern "C"{
#include "config.h"
#include "console.h"
#include "node.h"

#define FRAME_MODNAME "ActFrame"

#define ACTFRM_NODEPATHNAME "NodePathName"
#define ACTFRM_CTRLIP "CtrlIP"
#define ACTFRM_CTRLPORT "CtrlPort"

#define ACTFRM_STATE_IDLE 0
#define ACTFRM_STATE_RUN 1
#define ACTFRM_STATE_PAUSE 2

#define ACTFRM_CMD_EXIT_NAME "exit"
#define ACTFRM_CMD_EXIT_USAGE "0 ops, exit main frame."

class CActFrame
{
public:
    CActFrame();
    ~CActFrame();

    int InitFrame();
    int UninitFrame();
    int AttachNode(char *strNodePathName);
    int DetachNode();
    int Run();
    int Pause(int iGo);
    int Stop();

    static void *ThreadFunc(void *arg);
    void *MainThread();
    //int GetStatus

    static int OnCmdExit(PCOMMAND pCmd, char *strRet, void *pContext);


protected:
    int m_iState;
    pthread_t m_MainThread;
    CActNode *m_pNode;
    void *m_pdlHandle;
    char m_strNodePathName[CONFIGITEM_DATALEN];
private:
    int m_iModID;
};






//}
#endif
