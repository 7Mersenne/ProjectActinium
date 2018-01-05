#define ACT_MAIN

#include <stdio.h>

#include "include/debug.h"
#include "include/frame.h"


int main(int argc, char *argv[])
{
    g_cDebug.Init();
	g_cDebug.SetAllLevel(ACTDBG_LEVEL_DEBUG);

	class CActFrame *pFrame = new CActFrame;
	if(!pFrame) return -1;
	pFrame->InitFrame();
	delete pFrame;
	return 0;
}
