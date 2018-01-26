#ifndef ACTINIUM_NODE_IN_H_c6d4d071_3e05_4788_b325_5b9d81373e61
#define ACTINIUM_NODE_IN_H_c6d4d071_3e05_4788_b325_5b9d81373e61
extern "C"{
#include "../include/node.h"

#define NODEIN_MODNAME "NodeinFile"

class CActNodeInFile:public CActNode
{
public:
    CActNodeInFile();
    ~CActNodeInFile();

    int Init();
    int OneStep();
    int Reset();
    int Config(char *strConfig);

private:
    int m_iModID;
};

class CActNode *ActNewNode();

int ActDeleteNode(class CActNode *pNode);

}
#endif
