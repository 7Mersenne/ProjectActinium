#ifndef ACTINIUM_PORT_H_9888c549_84fe_42fd_8d92_726f870cef2b
#define ACTINIUM_PORT_H_9888c549_84fe_42fd_8d92_726f870cef2b
extern "C"{

#define PORT_MODNAME "ActPort"

class CPort
{
public:
    CPort();
    ~CPort();

    int GetCnt();
    int Connect(CPort *pIn, int iPortID);

protected:
    int m_iCnt;

private:
    int m_iModID;
};










}
#endif

