#ifndef ACTINIUM_MANAGER_H_73b34fb1_bb4d_4228_ad13_f19c91cda787
#define ACTINIUM_MANAGER_H_73b34fb1_bb4d_4228_ad13_f19c91cda787

#include <pthread.h>


extern "C"{
#include "config.h"
#include "console.h"
#include "nodescenter.h"
#include "node.h"

#define MANAGER_MODNAME "ActMan"

#define ACTMAN_NODEPORTITEM "NodeSvrPort"
#define ACTMAN_USERPORTITEM "UserSvrPort"

#define ACTMAN_DEFUSERPORT 8403
#define ACTMAN_DEFNODEPORT 8404

#define ACTMAN_STATE_IDLE 0
#define ACTMAN_STATE_RUN  1
#define ACTMAN_STATE_STOP 2

class CActMan
{
public:
    CActMan();
    ~CActMan();

    int InitMan();
    int Run();
    int InitCmds();
    int Stop();

    static int CmdExit(PCOMMAND pCmd, char *strRet, void *pContext);
    int onCmdExit(PCOMMAND pCmd, char *strRet);
    static int CmdShowUsers(PCOMMAND pCmd, char *strRet, void *pContext);
    int onCmdShowUsers(PCOMMAND pCmd, char *strRet);
    static int CmdShowNodes(PCOMMAND pCmd, char *strRet, void *pContext);
    int onCmdShowNodes(PCOMMAND pCmd, char *strRet);
    static int CmdShutDown(PCOMMAND pCmd, char *strRet, void *pContext);
    int onCmdShutDown(PCOMMAND pCmd, char *strRet);
protected:

    CNodesCenter m_cNodesCenter;
    CConsole m_cUsersCenter;

    int m_iUsersPort;
    int m_iNodesPort;

    int m_iState;

private:
    int m_iModID;
};






}
#endif
