#include <stdio.h>
extern "C"{
#include "../include/node_in.h"
#include "../include/debug.h"

class CActNode *ActNewNode()
{
    class CActNodeIn *pNode = new class CActNodeIn();
    return (class CActNode *)pNode;
}




CActNodeIn::CActNodeIn():CActNode()
{

}

CActNodeIn::~CActNodeIn()
{
    
}

int CActNodeIn::PrintMe()
{
    printf("I'm NodeIn.\n");
}

}
