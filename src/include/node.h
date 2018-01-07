#ifndef ACTINIUM_NODE_H_fa781fcc_31cc_462a_a676_34be11aa73d7
#define ACTINIUM_NODE_H_fa781fcc_31cc_462a_a676_34be11aa73d7
extern "C"{
#define NODEID_MAXLEN 128

#define NODE_MODNAME "ActNode"


typedef struct tag_ActNodeInfo
{
    char strID[NODEID_MAXLEN];
    int iVersion;
    int iFlags;
    int iType;
    int iInput;
    int iOutput;
}ACTNODEINFO, *PACTNODEINFO;

class CActNode
{
public:
    CActNode();

    int GetInfo(ACTNODEINFO &sINfo);
    virtual int PrintMe()=0;

protected:
    int m_iModID;
    ACTNODEINFO m_sInfo;
    
};
class CActNode *ActNewNode();
}


#endif
