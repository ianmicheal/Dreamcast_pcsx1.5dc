#include "stdafx.h"
#include "externals.h"
#include "cfg.h"
#include "gpu.h"

char * pConfigFile=NULL;

void ReadConfig(void)
{

 // defaults
 iResX=640;iResY=480;
 iColDepth=16;
 iWindowMode=1;
 iUseScanLines=0;
 UseFrameLimit=0;
 UseFrameSkip=0;
 iFrameLimit=2;
 iFrameRate=200;
 dwCfgFixes=0;
 iUseFixes=0;
 iUseNoStrechBlt=1;
 iShowFPS=1;
}

