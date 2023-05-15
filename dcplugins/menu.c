/***************************************************************************
                         menu.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************// 
// History of changes:
//                                    
// 2002/05/25 - Pete
// - Added menu support for linuzappz's fast forward skipping
//
// 2002/01/13 - linuzappz
// - Added timing for the szDebugText (to 2 secs)
//
// 2001/12/22 - syo
// - modified for "transparent menu" 
//   (Pete: added 'V' display for WaitVBlank)
//
// 2001/11/09 - Darko Matesic
// - added recording status 
//   (Pete: added terminate zero to the menu buffer ;)
//
// 2001/10/28 - Pete  
// - generic cleanup for the Peops release
//
//*************************************************************************// 

#include "stdafx.h"

#ifdef _WINDOWS

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "record.h" 

#endif

#define _IN_MENU

#include "externals.h"
#include "draw.h"
#include "menu.h"
#include "gpu.h"

unsigned long dwCoreFlags=0;

////////////////////////////////////////////////////////////////////////
// create lists/stuff for fonts (actually there are no more lists, but I am too lazy to change the func names ;)
////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
HFONT hGFont=NULL;
BOOL  bTransparent=FALSE;
#endif

void InitMenu(void)
{
#ifdef _WINDOWS
 hGFont=CreateFont(//-8,
                   13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
                   ANSI_CHARSET,OUT_DEFAULT_PRECIS,
                   CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
                   DEFAULT_PITCH,
                   //"Courier New");
                   //"MS Sans Serif");
                   "Arial");
#endif
}

////////////////////////////////////////////////////////////////////////
// kill existing lists/fonts
////////////////////////////////////////////////////////////////////////

void CloseMenu(void)
{
#ifdef _WINDOWS
 if(hGFont) DeleteObject(hGFont);
 hGFont=NULL;
#else
 DestroyPic();
#endif
}

////////////////////////////////////////////////////////////////////////
// DISPLAY FPS/MENU TEXT
////////////////////////////////////////////////////////////////////////

#include <time.h>
extern time_t tStart;

int iMPos=0;                                           // menu arrow pos

void DisplayText(void)                                 // DISPLAY TEXT
{
#ifdef _WINDOWS
 HDC hdc;HFONT hFO;

 IDirectDrawSurface_GetDC(DX.DDSRender,&hdc);
 hFO=(HFONT)SelectObject(hdc,hGFont);

 SetTextColor(hdc,RGB(0,255,0));
 if(bTransparent) 
      SetBkMode(hdc,TRANSPARENT);
 else SetBkColor(hdc,RGB(0,0,0));

 if(szDebugText[0] && ((time(NULL) - tStart) < 2))     // special debug text? show it
  {
   RECT r={0,0,1024,1024};
   DrawText(hdc,szDebugText,lstrlen(szDebugText),&r,DT_LEFT|DT_NOCLIP);
  }
 else                                                  // else standard gpu menu
  {
   szDebugText[0]=0;
   lstrcat(szDispBuf,szMenuBuf);
   ExtTextOut(hdc,0,0,0,NULL,szDispBuf,lstrlen(szDispBuf),NULL);
  }

 SelectObject(hdc,hFO);
 IDirectDrawSurface_ReleaseDC(DX.DDSRender,hdc);
#endif
}

////////////////////////////////////////////////////////////////////////
// Build Menu buffer (== Dispbuffer without FPS)...
////////////////////////////////////////////////////////////////////////

void BuildDispMenu(int iInc)
{
 if(!(ulKeybits&KEY_SHOWFPS)) return;                  // mmm, cheater ;)

 iMPos+=iInc;                                          // up or down
 if(iMPos<0) iMPos=2;                                  // wrap around
 if(iMPos>2) iMPos=0;

 strcpy(szMenuBuf,"   FL   FS   GF       ");           // main menu items

 if(UseFrameLimit)                                     // set marks
  {
   if(iFrameLimit==1) szMenuBuf[2]  = '+';
   else               szMenuBuf[2]  = '*';
  }
 if(iFastFwd)       szMenuBuf[7]  = '~';
 else
 if(UseFrameSkip)   szMenuBuf[7]  = '*';
 if(dwActFixes)     szMenuBuf[12] = '*';

 if(dwCoreFlags&1)  szMenuBuf[18]  = 'A';
 if(dwCoreFlags&2)  szMenuBuf[18]  = 'M';
#ifdef _WINDOWS
 if(bVsync_Key)     szMenuBuf[19]  = 'V';
#endif
 if(lSelectedSlot)  szMenuBuf[20]  = '0'+(char)lSelectedSlot;   

 szMenuBuf[(iMPos+1)*5]='<';                           // set arrow

#ifdef _WINDOWS
 if(RECORD_RECORDING)
  {
   szMenuBuf[21]  = ' ';
   szMenuBuf[22]  = ' ';
   szMenuBuf[23]  = ' ';
   szMenuBuf[24]  = ' ';
   szMenuBuf[25]  = 'R';
   szMenuBuf[26]  = 'e';
   szMenuBuf[27]  = 'c';
   szMenuBuf[28]  = 0;
  }

 if(DX.DDSScreenPic) ShowTextGpuPic();
#endif
}

////////////////////////////////////////////////////////////////////////
// Some menu action...
////////////////////////////////////////////////////////////////////////

void SwitchDispMenu(int iStep)                         // SWITCH DISP MENU
{
 if(!(ulKeybits&KEY_SHOWFPS)) return;                  // tststs

 switch(iMPos)
  {//////////////////////////////////////////////////////
   case 0:                                             // frame limit
    {
     int iType=0;
     bInitCap = TRUE;

#ifdef _WINDOWS
     if(iFrameLimit==1 && UseFrameLimit &&
        GetAsyncKeyState(VK_SHIFT)&32768)
      {
       iFrameRate+=iStep;
       if(iFrameRate<3) iFrameRate=3;
       SetAutoFrameCap();
       break;
      }
#endif

     if(UseFrameLimit) iType=iFrameLimit;
     iType+=iStep;
     if(iType<0) iType=2;
     if(iType>2) iType=0;
     if(iType==0) UseFrameLimit=0;
     else
      {
       UseFrameLimit=1;
       iFrameLimit=iType;
       SetAutoFrameCap();
      }
    } break;
   //////////////////////////////////////////////////////
   case 1:                                             // frame skip
    if(iStep>0)
     {
      if(!UseFrameSkip) {UseFrameSkip=1;iFastFwd = 0;}
      else
       {
        if(!iFastFwd) iFastFwd=1;
        else {UseFrameSkip=0;iFastFwd = 0;}
       }
     }
    else
     {
      if(!UseFrameSkip) {UseFrameSkip=1;iFastFwd = 1;}
      else
       {
        if(iFastFwd) iFastFwd=0;
        else {UseFrameSkip=0;iFastFwd = 0;}
       }
     }
    bSkipNextFrame=FALSE;
    break;
   //////////////////////////////////////////////////////
   case 2:                                             // special fixes
    if(iUseFixes) {iUseFixes=0;dwActFixes=0;}
    else          {iUseFixes=1;dwActFixes=dwCfgFixes;}
    if(iFrameLimit==2) SetAutoFrameCap();
    SetFixes();
    break;
  }

 BuildDispMenu(0);                                     // update info
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

