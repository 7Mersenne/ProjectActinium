#ifndef ACTINIUM_NODE_H_fa781fcc_31cc_462a_a676_34be11aa73d7
#define ACTINIUM_NODE_H_fa781fcc_31cc_462a_a676_34be11aa73d7

#define NODEID_MAXLEN 128

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

protected:
    int m_iModID;
    ACTNODEINFO m_sInfo;
    
};



#endif