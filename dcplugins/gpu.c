/***************************************************************************
                          gpu.c  -  description
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
// 2002/08/31 - Pete
// - delayed odd/even toggle for FF8 intro scanlines - Pete
//
// 2002/08/03 - Pete
// - "Sprite 1" command count added
//
// 2002/08/03 - Pete
// - handles "screen disable" correctly
//
// 2002/07/28 - Pete
// - changed dmachain handler (monkey hero)
//
// 2002/06/15 - Pete
// - removed dmachain fixes, added dma endless loop detection instead
//
// 2002/05/31 - Lewpy 
// - Win95/NT "disable screensaver" fix
//
// 2002/05/30 - Pete
// - dmawrite/read wrap around
//
// 2002/05/15 - Pete
// - Added dmachain "0" check game fix
//
// 2002/04/20 - linuzappz
// - added iFastFwd stuff
//
// 2002/02/18 - linuzappz
// - Added DGA2 support to PIC stuff
//
// 2002/02/10 - Pete
// - Added dmacheck for The Mummy and T'ai Fu
//
// 2002/01/13 - linuzappz
// - Added timing in the GPUdisplayText func
//
// 2002/01/06 - lu
// - Added some #ifdef for the linux configurator
//
// 2002/01/05 - Pete
// - fixed unwanted screen clearing on horizontal centering (causing
//   flickering in linux version)
//
// 2001/12/10 - Pete
// - fix for Grandia in ChangeDispOffsetsX
//
// 2001/12/05 - syo (syo68k@geocities.co.jp)
// - added disable screen saver for "stop screen saver" option
//
// 2001/11/20 - linuzappz
// - added Soft and About DlgProc calls in GPUconfigure and
//   GPUabout, for linux
//
// 2001/11/09 - Darko Matesic
// - added recording frame in updateLace and stop recording
//   in GPUclose (if it is still recording)
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
#include "resource.h" 

#endif
                 
#define _IN_GPU

#ifdef _WINDOWS
#include "record.h"
#endif

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "cfg.h"
#include "prim.h"
#include "psemu.h"
#include "menu.h"
#include "key.h"
#include "fps.h"
                       
////////////////////////////////////////////////////////////////////////
// PPDK developer must change libraryName field and can change revision and build
////////////////////////////////////////////////////////////////////////

const  unsigned char version  = 1;    // do not touch - library for PSEmu 1.x
const  unsigned char revision = 1;
const  unsigned char build    = 8;    // increase that with each version

#ifdef _WINDOWS
static char *libraryName      = "P.E.Op.S. Soft Driver";
#else
#ifndef _SDL
static char *libraryName      = "P.E.Op.S. SoftX Driver";
static char *libraryInfo      = "P.E.Op.S. SoftX Driver V1.8\nCoded by Pete Bernert and the P.E.Op.S. team\n";
#else
static char *libraryName      = "P.E.Op.S. SoftSDL Driver";
static char *libraryInfo      = "P.E.Op.S. SoftSDL Driver V1.8\nCoded by Pete Bernert and the P.E.Op.S. team\n";
#endif
#endif

static char *PluginAuthor     = "Pete Bernert and the P.E.Op.S. team";
 
////////////////////////////////////////////////////////////////////////
// memory image of the PSX vram 
////////////////////////////////////////////////////////////////////////

// ATTENTION: double psx vram (security in new soft drawing funcs)
unsigned char  psxVSecure[1024*1024*2];
unsigned char  *psxVub;
signed   char  *psxVsb;
unsigned short *psxVuw;
unsigned short *psxVuw_eom;
signed   short *psxVsw;
unsigned long  *psxVul;
signed   long  *psxVsl;

////////////////////////////////////////////////////////////////////////
// GPU globals
////////////////////////////////////////////////////////////////////////

static long       lGPUdataRet;
long              lGPUstatusRet;
char              szDispBuf[64];
char              szMenuBuf[32];
char              szDebugText[512];
unsigned long     ulStatusControl[256];      

static unsigned   long gpuDataM[256];
static unsigned   char gpuCommand = 0;
static long       gpuDataC = 0;
static long       gpuDataP = 0;

VRAMLoad_t        VRAMWrite;
VRAMLoad_t        VRAMRead;
DATAREGISTERMODES DataWriteMode;
DATAREGISTERMODES DataReadMode;

BOOL              bSkipNextFrame = FALSE;
int               iColDepth;
int               iWindowMode;
short             sDispWidths[8] = {256,320,512,640,368,384,512,640};
PSXDisplay_t      PSXDisplay;
PSXDisplay_t      PreviousPSXDisplay;
long              lSelectedSlot=0;
BOOL              bChangeWinMode=FALSE;
//unsigned int      iMaxDMACommandCounter=2000000;
//unsigned long     dwDMAChainStop=0;
BOOL              bDoLazyUpdate=FALSE;

#ifdef _WINDOWS

////////////////////////////////////////////////////////////////////////
// screensaver stuff: dynamically load kernel32.dll to avoid export dependeny
////////////////////////////////////////////////////////////////////////

int				  iStopSaver=0;
HINSTANCE kernel32LibHandle = NULL;

// A stub function, that does nothing .... but it does "nothing" well :)
EXECUTION_STATE WINAPI STUB_SetThreadExecutionState(EXECUTION_STATE esFlags)
{
	return esFlags;
}

// The dynamic version of the system call is prepended with a "D_"
EXECUTION_STATE (WINAPI *D_SetThreadExecutionState)(EXECUTION_STATE esFlags) = STUB_SetThreadExecutionState;

BOOL LoadKernel32(void)
{
	// Get a handle to the kernel32.dll (which is actually already loaded)
	kernel32LibHandle = LoadLibrary("kernel32.dll");

	// If we've got a handle, then locate the entry point for the SetThreadExecutionState function
	if (kernel32LibHandle != NULL)
	{
		if ((D_SetThreadExecutionState = (EXECUTION_STATE (WINAPI *)(EXECUTION_STATE))GetProcAddress (kernel32LibHandle, "SetThreadExecutionState")) == NULL)
			D_SetThreadExecutionState = STUB_SetThreadExecutionState;
	}

	return TRUE;
}

BOOL FreeKernel32(void)
{
	// Release the handle to kernel32.dll
	if (kernel32LibHandle != NULL)
		FreeLibrary(kernel32LibHandle);

	// Set to stub function, to avoid nasty suprises if called :)
	D_SetThreadExecutionState = STUB_SetThreadExecutionState;

	return TRUE;
}
#else

// Linux: Stub the functions
BOOL LoadKernel32(void)
{
	return TRUE;
}

BOOL FreeKernel32(void)
{
	return TRUE;
}

#endif

////////////////////////////////////////////////////////////////////////
// some misc external display funcs
////////////////////////////////////////////////////////////////////////

#include <time.h>
time_t tStart;

void CALLBACK GPUdisplayText(char * pText)             // some debug func
{
 if(!pText) {szDebugText[0]=0;return;}
 if(strlen(pText)>511) return;
 time(&tStart);
 strcpy(szDebugText,pText);
}

////////////////////////////////////////////////////////////////////////

void CALLBACK GPUdisplayFlags(unsigned long dwFlags)   // some info func
{
 dwCoreFlags=dwFlags;
 BuildDispMenu(0);
}

////////////////////////////////////////////////////////////////////////
// stuff to make this a true PDK module
////////////////////////////////////////////////////////////////////////

char * CALLBACK PSEgetLibName(void)
{
 return libraryName;
}

unsigned long CALLBACK PSEgetLibType(void)
{
 return  PSE_LT_GPU;
}

unsigned long CALLBACK PSEgetLibVersion(void)
{
 return version<<16|revision<<8|build;
}

#ifndef _WINDOWS
char * GPUgetLibInfos(void)
{
 return libraryInfo;
}
#endif

////////////////////////////////////////////////////////////////////////
// Snapshot func
////////////////////////////////////////////////////////////////////////

char * pGetConfigInfos(int iCfg)
{
 char szO[2][4]={"off","on "};
 char szTxt[256];
 char * pB=(char *)malloc(32767);

 if(!pB) return NULL;
 *pB=0;
 //----------------------------------------------------//
 sprintf(szTxt,"Plugin: %s %d.%d.%d\r\n",libraryName,version,revision,build);
 strcat(pB,szTxt);
 sprintf(szTxt,"Author: %s\r\n\r\n",PluginAuthor);
 strcat(pB,szTxt);
 //----------------------------------------------------//
 if(iCfg && iWindowMode)
  sprintf(szTxt,"Resolution/Color:\r\n- %dx%d ",LOWORD(iWinSize),HIWORD(iWinSize));
 else
  sprintf(szTxt,"Resolution/Color:\r\n- %dx%d ",iResX,iResY);
 strcat(pB,szTxt);
 if(iWindowMode && iCfg) 
   strcpy(szTxt,"Window mode\r\n\r\n");
 else
 if(iWindowMode) 
   sprintf(szTxt,"Window mode - [%d Bit]\r\n\r\n",iDesktopCol);
 else
   sprintf(szTxt,"Fullscreen - [%d Bit]\r\n\r\n",iColDepth);
 strcat(pB,szTxt);
 //----------------------------------------------------//
 sprintf(szTxt,"Framerate:\r\n- FPS limit: %s\r\n",szO[UseFrameLimit]);
 strcat(pB,szTxt);
 sprintf(szTxt,"- Frame skipping: %s",szO[UseFrameSkip]);
 strcat(pB,szTxt);
 if(iFastFwd) strcat(pB," (fast forward)");
 strcat(pB,"\r\n");
 if(iFrameLimit==2)
      strcpy(szTxt,"- FPS limit: Auto\r\n\r\n");
 else sprintf(szTxt,"- FPS limit: %d\r\n\r\n",iFrameRate);
 strcat(pB,szTxt);
 //----------------------------------------------------//
 strcpy(szTxt,"Misc:\r\n- Scanlines: ");
 if(iUseScanLines==0) strcat(szTxt,"disabled");
 else
 if(iUseScanLines==1) strcat(szTxt,"standard");
 else
 if(iUseScanLines==2) strcat(szTxt,"double blitting");
 strcat(szTxt,"\r\n");
 strcat(pB,szTxt);
 sprintf(szTxt,"- Game fixes: %s [%08lx]\r\n",szO[iUseFixes],dwCfgFixes);
 strcat(pB,szTxt);
 //----------------------------------------------------//
 return pB;
}

void DoTextSnapShot(int iNum)
{
 FILE *txtfile;char szTxt[256];char * pB;

#ifdef _WINDOWS
 sprintf(szTxt,"SNAP\\PEOPSSOFT%03d.txt",iNum);
#else
 sprintf(szTxt,"%s/peopssoft%03d.txt",getenv("HOME"),iNum);
#endif

 if((txtfile=fopen(szTxt,"wb"))==NULL)
  return;                                              
 //----------------------------------------------------//
 pB=pGetConfigInfos(0);
 if(pB)
  {
   fwrite(pB,strlen(pB),1,txtfile);
   free(pB);
  }
 fclose(txtfile); 
}

////////////////////////////////////////////////////////////////////////

void CALLBACK GPUmakeSnapshot(void)                    // snapshot of whole vram
{
 FILE *bmpfile;
 char filename[256];     
 unsigned char header[0x36];
 long size;
 unsigned char line[1024*3];
 short i,j;
 unsigned char empty[2]={0,0};
 unsigned short color;
 unsigned long snapshotnr = 0;
 
 size=512*1024*3+0x38;
 
 // fill in proper values for BMP

 // hardcoded BMP header
 memset(header,0,0x36);
 header[0]='B';
 header[1]='M';
 header[2]=size&0xff;
 header[3]=(size>>8)&0xff;
 header[4]=(size>>16)&0xff;
 header[5]=(size>>24)&0xff;
 header[0x0a]=0x36;
 header[0x0e]=0x28;
 header[0x12]=1024%256;
 header[0x13]=1024/256;
 header[0x16]=512%256;
 header[0x17]=512/256;
 header[0x1a]=0x01;
 header[0x1c]=0x18;
 header[0x26]=0x12;
 header[0x27]=0x0B;
 header[0x2A]=0x12;
 header[0x2B]=0x0B;

 // increment snapshot value & try to get filename
 do
  {
   snapshotnr++;
#ifdef _WINDOWS
   sprintf(filename,"SNAP\\PEOPSSOFT%03d.bmp",snapshotnr);
#else
   sprintf(filename,"%s/peopssoft%03ld.bmp",getenv("HOME"),snapshotnr);
#endif

   bmpfile=fopen(filename,"rb");
   if (bmpfile == NULL) break;
   fclose(bmpfile);
  }
 while(TRUE);

 // try opening new snapshot file
 if((bmpfile=fopen(filename,"wb"))==NULL)
  return;
 
 fwrite(header,0x36,1,bmpfile);
 for(i=511;i>=0;i--)
  {
   for(j=0;j<1024;j++)
    {
     color=psxVuw[i*1024+j];
     line[j*3+2]=(color<<3)&0xf1;
     line[j*3+1]=(color>>2)&0xf1;
     line[j*3+0]=(color>>7)&0xf1;
    }
   fwrite(line,1024*3,1,bmpfile);
  }
 fwrite(empty,0x2,1,bmpfile);
 fclose(bmpfile);  

 DoTextSnapShot(snapshotnr);
}        

////////////////////////////////////////////////////////////////////////
// INIT, will be called after lib load... well, just do some var init...
////////////////////////////////////////////////////////////////////////
 
long CALLBACK GPU__init()                                // GPU INIT
{
 memset(ulStatusControl,0,256*sizeof(unsigned long));  // init save state scontrol field

 szDebugText[0]=0;                                     // init debug text buffer

 //!!! ATTENTION !!!
 psxVub=psxVSecure+512*1024;                           // security offset into double sized psx vram!

 psxVsb=(signed char *)psxVub;                         // different ways of accessing PSX VRAM
 psxVsw=(signed short *)psxVub;
 psxVsl=(signed long *)psxVub;
 psxVuw=(unsigned short *)psxVub;
 psxVul=(unsigned long *)psxVub;

 psxVuw_eom=psxVuw+1024*512;                           // pre-calc of end of vram
                        
 memset(psxVSecure,0x00,1024*1024*2);
 
 SetFPSHandler();   

 PSXDisplay.RGB24        = FALSE;                      // init some stuff
 PSXDisplay.Interlaced   = FALSE;
 PSXDisplay.DrawOffset.x = 0;
 PSXDisplay.DrawOffset.y = 0;
 PSXDisplay.DisplayMode.x= 320;
 PSXDisplay.DisplayMode.y= 240;
 PreviousPSXDisplay.DisplayMode.x= 320;
 PreviousPSXDisplay.DisplayMode.y= 240;
 PSXDisplay.Disabled     = FALSE;
 PreviousPSXDisplay.Range.x0 =0;
 PreviousPSXDisplay.Range.y0 =0;
 PSXDisplay.Range.x0=0;
 PSXDisplay.Range.x1=0;
 PreviousPSXDisplay.DisplayModeNew.y=0;
 PSXDisplay.Double=1;

 DataWriteMode = DR_NORMAL;

 // Reset transfer values, to prevent mis-transfer of data
 memset(&VRAMWrite,0,sizeof(VRAMLoad_t));
 memset(&VRAMRead,0,sizeof(VRAMLoad_t));
 
 // device initialised already !
 lGPUstatusRet = 0x14802000;
 GPUIsIdle;
 GPUIsReadyForCommands;

 // Get a handle for kernel32.dll, and access the required export function
 LoadKernel32();

 return 0;
}

////////////////////////////////////////////////////////////////////////
// Here starts all...
////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
long CALLBACK GPUopen(HWND hwndGPU)                    // GPU OPEN
{
 hWGPU = hwndGPU;                                      // store hwnd

 SetKeyHandler();                                      // sub-class window

 if(bChangeWinMode) ReadWinSizeConfig();               // alt+enter toggle?
 else                                                  // or first time startup?
  {
   ReadConfig();                                       // read registry
   InitFPS();
  }

 bIsFirstFrame = TRUE;                                 // we have to init later

 ulInitDisplay();                                      // setup direct draw

 if(iStopSaver)
  D_SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED|ES_CONTINUOUS);

 return 0;
}

#else

long GPU__open(unsigned long * disp,char * CapText,char * CfgFile)
{
 unsigned long d;

 pCaptionText=CapText;
#ifndef _FPSE
 pConfigFile=CfgFile;
#endif
 ReadConfig();                                         // read registry

 InitFPS();

 bIsFirstFrame = TRUE;                                 // we have to init later

 d=ulInitDisplay();                                      // setup x

 if(disp) *disp=d;                                     // wanna x pointer? ok

 if(d) return 0;
 return -1;
}

#endif

////////////////////////////////////////////////////////////////////////
// time to leave...
////////////////////////////////////////////////////////////////////////

long CALLBACK GPU__close()                               // GPU CLOSE
{
#ifdef _WINDOWS
 if(RECORD_RECORDING==TRUE) {RECORD_Stop();RECORD_RECORDING=FALSE;BuildDispMenu(0);}
#endif

 ReleaseKeyHandler();                                  // de-subclass window

 CloseDisplay();                                       // shutdown direct draw

#ifdef _WINDOWS
 if(iStopSaver)
  D_SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED);
#endif

 return 0;
}

////////////////////////////////////////////////////////////////////////
// I shot the sheriff
////////////////////////////////////////////////////////////////////////

long CALLBACK GPU__shutdown()                            // GPU SHUTDOWN
{
 // screensaver: release the handle for kernel32.dll
 FreeKernel32();

 return 0;                                             // nothinh to do
}

////////////////////////////////////////////////////////////////////////
// Update display (swap buffers)
////////////////////////////////////////////////////////////////////////

void updateDisplay(void)                               // UPDATE DISPLAY
{
 if(PSXDisplay.Disabled)                               // disable?
  {
   DoClearFrontBuffer();                               // -> clear frontbuffer
   return;                                             // -> and bye
  }

 if(dwActFixes&32)                                     // pc fps calculation fix
  {
   if(UseFrameLimit) PCFrameCap();                     // -> brake
   if(UseFrameSkip || ulKeybits&KEY_SHOWFPS)  
    PCcalcfps();         
  }

 if(ulKeybits&KEY_SHOWFPS)                             // make fps display buf
  {
   sprintf(szDispBuf,"FPS %06.1f",fps_cur);
  }                 

 if(iFastFwd)                                          // fastfwd ?
  {
   static int fpscount; UseFrameSkip=1;

   if(!bSkipNextFrame) DoBufferSwap();                 // -> to skip or not to skip
   if(fpscount%6)                                      // -> skip 6/7 frames
        bSkipNextFrame = TRUE;
   else bSkipNextFrame = FALSE;
   fpscount++;
   if(fpscount >= (int)fFrameRateHz) fpscount = 0;
   return;
  }

 if(UseFrameSkip)                                      // skip ?
  {
   if(!bSkipNextFrame) DoBufferSwap();                 // -> to skip or not to skip
   if((fps_skip < fFrameRateHz) && !(bSkipNextFrame))  // -> skip max one in a row
       {bSkipNextFrame = TRUE; fps_skip=fFrameRateHz;}
   else bSkipNextFrame = FALSE;
  }
 else                                                  // no skip ?
  {
   DoBufferSwap();                                     // -> swap
  }
}

////////////////////////////////////////////////////////////////////////
// roughly emulated screen centering bits... not complete !!!
////////////////////////////////////////////////////////////////////////

void ChangeDispOffsetsX(void)                          // X CENTER
{
 long lx,l;

 if(!PSXDisplay.Range.x1) return;

 l=PreviousPSXDisplay.DisplayMode.x;

 l*=(long)PSXDisplay.Range.x1;
 l/=2560;lx=l;l&=0xfffffff8;

 if(l==PreviousPSXDisplay.Range.y1) return;            // abusing range.y1 for
 PreviousPSXDisplay.Range.y1=(short)l;                        // storing last x range and test

 if(lx>=PreviousPSXDisplay.DisplayMode.x)
  {
   PreviousPSXDisplay.Range.x1=
    (short)PreviousPSXDisplay.DisplayMode.x;
   PreviousPSXDisplay.Range.x0=0;
  }
 else
  {
   PreviousPSXDisplay.Range.x1=(short)l;

   PreviousPSXDisplay.Range.x0=
    (PSXDisplay.Range.x0-500)/8;

   if(PreviousPSXDisplay.Range.x0<0)
    PreviousPSXDisplay.Range.x0=0;

   if((PreviousPSXDisplay.Range.x0+lx)>
      PreviousPSXDisplay.DisplayMode.x)
    {
     PreviousPSXDisplay.Range.x0=
      (short)(PreviousPSXDisplay.DisplayMode.x-lx);
     PreviousPSXDisplay.Range.x0+=2; //???

     PreviousPSXDisplay.Range.x1+=(short)(lx-l);
#ifndef _WINDOWS
     PreviousPSXDisplay.Range.x1-=2; // makes linux stretching easier
#endif
    }

#ifndef _WINDOWS
   // some linux alignment security
   PreviousPSXDisplay.Range.x0=PreviousPSXDisplay.Range.x0>>1;
   PreviousPSXDisplay.Range.x0=PreviousPSXDisplay.Range.x0<<1;
   PreviousPSXDisplay.Range.x1=PreviousPSXDisplay.Range.x1>>1;
   PreviousPSXDisplay.Range.x1=PreviousPSXDisplay.Range.x1<<1;
#endif

   DoClearScreenBuffer();
  }
}

////////////////////////////////////////////////////////////////////////

void ChangeDispOffsetsY(void)                          // Y CENTER
{
 int iT,iO=PreviousPSXDisplay.Range.y0;

 if(PSXDisplay.PAL) iT=48; else iT=28;

 if(PSXDisplay.Range.y0>=iT)
  {
   PreviousPSXDisplay.Range.y0=
    (short)((PSXDisplay.Range.y0-iT-4)*PSXDisplay.Double);
   if(PreviousPSXDisplay.Range.y0<0)
    PreviousPSXDisplay.Range.y0=0;
   PSXDisplay.DisplayModeNew.y+=
    PreviousPSXDisplay.Range.y0;
  }
 else PreviousPSXDisplay.Range.y0=0;

 if(iO!=PreviousPSXDisplay.Range.y0)
  DoClearScreenBuffer();
}

////////////////////////////////////////////////////////////////////////
// check if update needed
////////////////////////////////////////////////////////////////////////

void updateDisplayIfChanged(void)                      // UPDATE DISPLAY IF CHANGED
{
 if ((PSXDisplay.DisplayMode.y == PSXDisplay.DisplayModeNew.y) && 
     (PSXDisplay.DisplayMode.x == PSXDisplay.DisplayModeNew.x))
  {
   if((PSXDisplay.RGB24      == PSXDisplay.RGB24New) && 
      (PSXDisplay.Interlaced == PSXDisplay.InterlacedNew)) return;
  }

 PSXDisplay.RGB24         = PSXDisplay.RGB24New;       // get new infos

 PSXDisplay.DisplayMode.y = PSXDisplay.DisplayModeNew.y;
 PSXDisplay.DisplayMode.x = PSXDisplay.DisplayModeNew.x;
 PreviousPSXDisplay.DisplayMode.x=                     // previous will hold
  min(640,PSXDisplay.DisplayMode.x);                   // max 640x512... that's
 PreviousPSXDisplay.DisplayMode.y=                     // the size of my 
  min(512,PSXDisplay.DisplayMode.y);                   // back buffer surface
 PSXDisplay.Interlaced    = PSXDisplay.InterlacedNew;
    
 PSXDisplay.DisplayEnd.x=                              // calc end of display
  PSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
 PSXDisplay.DisplayEnd.y=
  PSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y+PreviousPSXDisplay.DisplayModeNew.y;
 PreviousPSXDisplay.DisplayEnd.x=
  PreviousPSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
 PreviousPSXDisplay.DisplayEnd.y=
  PreviousPSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y+PreviousPSXDisplay.DisplayModeNew.y;

 ChangeDispOffsetsX();

 if(iFrameLimit==2) SetAutoFrameCap();                 // -> set it

 if(UseFrameSkip) updateDisplay();                     // stupid stuff when frame skipping enabled
}

////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
void ChangeWindowMode(void)                            // TOGGLE FULLSCREEN - WINDOW
{
 GPUclose();
 iWindowMode=!iWindowMode;
 GPUopen(hWGPU);
 bChangeWinMode=FALSE;
}
#endif

////////////////////////////////////////////////////////////////////////
// update lace is called evry VSync
////////////////////////////////////////////////////////////////////////

void CALLBACK GPU__updateLace(void)                      // VSYNC
{
//ff8lines:
// lGPUstatusRet^=0x80000000;

 if(!(dwActFixes&32))
  {
   if(UseFrameLimit) FrameCap();
   if((ulKeybits&KEY_SHOWFPS) || UseFrameSkip) calcfps();  
  }                      

 if(PSXDisplay.Interlaced)                             // interlaced mode?
  {
   if(PSXDisplay.DisplayMode.x>0 && PSXDisplay.DisplayMode.y>0)
    {
     updateDisplay();
    }
  }
 else                                                  // non-interlaced?
  {
   if(dwActFixes&64)                                   // lazy screen update fix
    {
     if(bDoLazyUpdate && !UseFrameSkip) 
      updateDisplay(); 
     bDoLazyUpdate=FALSE;
    }
   else
    {
     if(bDoVSyncUpdate && !UseFrameSkip)               // some primitives drawn?
      updateDisplay();                                 // -> update display
    }
  }

#ifdef _WINDOWS

if(RECORD_RECORDING)
 if(RECORD_WriteFrame()==FALSE)
  {RECORD_RECORDING=FALSE;RECORD_Stop();}

 if(bChangeWinMode) ChangeWindowMode();                // toggle full - window mode

#endif

 bDoVSyncUpdate=FALSE;                                 // vsync done
}

////////////////////////////////////////////////////////////////////////
// process read request from GPU status register
////////////////////////////////////////////////////////////////////////


unsigned long CALLBACK GPU__readStatus(void)             // READ STATUS
{
 static int iNumRead=0;
 if((iNumRead++)==2)
  {
   iNumRead=0;
   lGPUstatusRet^=0x80000000;                           // interlaced bit toggle... we do it on every read status... needed by some games (like ChronoCross)
  }
 return lGPUstatusRet;
}

////////////////////////////////////////////////////////////////////////
// processes data send to GPU status register
// these are always single packet commands.
////////////////////////////////////////////////////////////////////////

void CALLBACK GPU__writeStatus(unsigned long gdata)      // WRITE STATUS
{
 unsigned long lCommand=(gdata>>24)&0xff;

 ulStatusControl[lCommand]=gdata;                      // store command for freezing

 switch(lCommand)
  {
   //--------------------------------------------------//
   // dis/enable display 
   case 0x03:  

    PreviousPSXDisplay.Disabled = PSXDisplay.Disabled;
    PSXDisplay.Disabled = (gdata & 1);

    if(PSXDisplay.Disabled) 
         lGPUstatusRet|=GPUSTATUS_DISPLAYDISABLED;
    else lGPUstatusRet&=~GPUSTATUS_DISPLAYDISABLED;

/*
    if ((PreviousPSXDisplay.Disabled != PSXDisplay.Disabled)&& 
        !(PSXDisplay.Disabled))
     {
      updateDisplay();                                 // show screen on enable
     }
*/

    return;

   //--------------------------------------------------//
   // setting transfer mode
   case 0x04:
    gdata &= 0x03;                                     // Only want the lower two bits

    DataWriteMode=DataReadMode=DR_NORMAL;
    if(gdata==0x02) DataWriteMode=DR_VRAMTRANSFER;
    if(gdata==0x03) DataReadMode =DR_VRAMTRANSFER;
    lGPUstatusRet&=~GPUSTATUS_DMABITS;                 // Clear the current settings of the DMA bits
    lGPUstatusRet|=(gdata << 29);                      // Set the DMA bits according to the received data

    return;
   //--------------------------------------------------//
   // setting display position
   case 0x05: 
    {
     PreviousPSXDisplay.DisplayPosition.x = PSXDisplay.DisplayPosition.x;
     PreviousPSXDisplay.DisplayPosition.y = PSXDisplay.DisplayPosition.y;

     PSXDisplay.DisplayPosition.y = (short)((gdata>>10)&0x3ff);
     if (PSXDisplay.DisplayPosition.y & 0x200) 
      PSXDisplay.DisplayPosition.y |= 0xfffffc00;
     if(PSXDisplay.DisplayPosition.y<0) 
      {
       PreviousPSXDisplay.DisplayModeNew.y=PSXDisplay.DisplayPosition.y/PSXDisplay.Double;
       PSXDisplay.DisplayPosition.y=0;
      }
     else PreviousPSXDisplay.DisplayModeNew.y=0;

     PSXDisplay.DisplayPosition.x = (short)(gdata & 0x3ff);
     PSXDisplay.DisplayEnd.x=
      PSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
     PSXDisplay.DisplayEnd.y=
      PSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y + PreviousPSXDisplay.DisplayModeNew.y;

     PreviousPSXDisplay.DisplayEnd.x=
      PreviousPSXDisplay.DisplayPosition.x+ PSXDisplay.DisplayMode.x;
     PreviousPSXDisplay.DisplayEnd.y=
      PreviousPSXDisplay.DisplayPosition.y+ PSXDisplay.DisplayMode.y + PreviousPSXDisplay.DisplayModeNew.y;
 
     if (!(PSXDisplay.Interlaced))                      // stupid frame skipping option
      {
       if(UseFrameSkip)  updateDisplay();
       else              bDoVSyncUpdate=TRUE;
       if(dwActFixes&64) bDoLazyUpdate=TRUE;
      }
    }return;
   //--------------------------------------------------//
   // setting width
   case 0x06:

    PSXDisplay.Range.x0=(short)(gdata & 0x7ff);
    PSXDisplay.Range.x1=(short)((gdata>>12) & 0xfff);

    PSXDisplay.Range.x1-=PSXDisplay.Range.x0;

    ChangeDispOffsetsX();

    return;
   //--------------------------------------------------//
   // setting height
   case 0x07:

    PSXDisplay.Range.y0=(short)(gdata & 0x3ff);
    PSXDisplay.Range.y1=(short)((gdata>>10) & 0x3ff);
                                      
    PreviousPSXDisplay.Height = PSXDisplay.Height;

    PSXDisplay.Height = PSXDisplay.Range.y1 - 
                        PSXDisplay.Range.y0 +
                        PreviousPSXDisplay.DisplayModeNew.y;

    if(PreviousPSXDisplay.Height!=PSXDisplay.Height)
     {
      PSXDisplay.DisplayModeNew.y=PSXDisplay.Height*PSXDisplay.Double;

      ChangeDispOffsetsY();

      updateDisplayIfChanged();
     }

    return;
   //--------------------------------------------------//
   // setting display infos
   case 0x08:

    PSXDisplay.DisplayModeNew.x =
     sDispWidths[(gdata & 0x03) | ((gdata & 0x40) >> 4)];

    if (gdata&0x04) PSXDisplay.Double=2;
    else            PSXDisplay.Double=1;

    PSXDisplay.DisplayModeNew.y = PSXDisplay.Height*PSXDisplay.Double;

    ChangeDispOffsetsY();

    PSXDisplay.PAL           = (gdata & 0x08)?TRUE:FALSE; // if 1 - PAL mode, else NTSC
    PSXDisplay.RGB24New      = (gdata & 0x10)?TRUE:FALSE; // if 1 - TrueColor
    PSXDisplay.InterlacedNew = (gdata & 0x20)?TRUE:FALSE; // if 1 - Interlace

    lGPUstatusRet&=~GPUSTATUS_WIDTHBITS;                   // Clear the width bits
    lGPUstatusRet|=
               (((gdata & 0x03) << 17) | 
               ((gdata & 0x40) << 10));                // Set the width bits

    if(PSXDisplay.InterlacedNew)
     {
      if(!PSXDisplay.Interlaced)
       {
        PreviousPSXDisplay.DisplayPosition.x = PSXDisplay.DisplayPosition.x;
        PreviousPSXDisplay.DisplayPosition.y = PSXDisplay.DisplayPosition.y;
       }
      lGPUstatusRet|=GPUSTATUS_INTERLACED;
     }
    else lGPUstatusRet&=~GPUSTATUS_INTERLACED;

    if (PSXDisplay.PAL)
         lGPUstatusRet|=GPUSTATUS_PAL;
    else lGPUstatusRet&=~GPUSTATUS_PAL;

    if (PSXDisplay.Double==2)
         lGPUstatusRet|=GPUSTATUS_DOUBLEHEIGHT;
    else lGPUstatusRet&=~GPUSTATUS_DOUBLEHEIGHT;

    if (PSXDisplay.RGB24New)
         lGPUstatusRet|=GPUSTATUS_RGB24;
    else lGPUstatusRet&=~GPUSTATUS_RGB24;

    updateDisplayIfChanged();

    return;
   //--------------------------------------------------//
   // ask about GPU version and other stuff
   case 0x10: 

    gdata&=0xff;

    lGPUdataRet=0;
    if(gdata==7) lGPUdataRet=0x2; 
    if(gdata==3) lGPUdataRet=drawX|(drawY<<10);
    if(gdata==4) lGPUdataRet=drawW|(drawH<<10);
    if(gdata==5) lGPUdataRet=PreviousPSXDisplay.DrawOffset.x|(PreviousPSXDisplay.DrawOffset.y<<11);

    return;
   //--------------------------------------------------//
  }   
}

////////////////////////////////////////////////////////////////////////
// vram read/write helpers, needed by LEWPY's optimized vram read/write :)
////////////////////////////////////////////////////////////////////////

__inline void FinishedVRAMWrite(void)
{
 VRAMWrite.Width +=VRAMWrite.x;
 VRAMWrite.Height+=VRAMWrite.y;

 if(!PSXDisplay.Interlaced)                            // stupid frame skipping
  {
   if(UseFrameSkip &&
      VRAMWrite.x<PSXDisplay.DisplayEnd.x &&
      VRAMWrite.Width >=PSXDisplay.DisplayPosition.x &&
      VRAMWrite.y<PSXDisplay.DisplayEnd.y &&
      VRAMWrite.Height>=PSXDisplay.DisplayPosition.y)
    updateDisplay();
  }

 bDoVSyncUpdate=TRUE;

 // Set register to NORMAL operation
 DataWriteMode = DR_NORMAL;
 // Reset transfer values, to prevent mis-transfer of data
 VRAMWrite.x = 0;
 VRAMWrite.y = 0;
 VRAMWrite.Width = 0;
 VRAMWrite.Height = 0;
 VRAMWrite.ColsRemaining = 0;
 VRAMWrite.RowsRemaining = 0;
}

__inline void FinishedVRAMRead(void)
{
 // Set register to NORMAL operation
 DataReadMode = DR_NORMAL;
 // Reset transfer values, to prevent mis-transfer of data
 VRAMRead.x = 0;
 VRAMRead.y = 0;
 VRAMRead.Width = 0;
 VRAMRead.Height = 0;
 VRAMRead.ColsRemaining = 0;
 VRAMRead.RowsRemaining = 0;

 // Indicate GPU is no longer ready for VRAM data in the STATUS REGISTER
 lGPUstatusRet&=~GPUSTATUS_READYFORVRAM;
}

////////////////////////////////////////////////////////////////////////
// core read from vram
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUreadDataMem(unsigned long * pMem, int iSize)
{
 int i;

 // Check for when a game sets the transfer mode, but is not transfering data
 if((DataReadMode==DR_VRAMTRANSFER) && 
    (VRAMRead.RowsRemaining==0) &&
    (VRAMRead.ColsRemaining == 0))
  {DataReadMode = DR_NORMAL;return;}

 if(DataReadMode!=DR_VRAMTRANSFER) return;

 GPUIsBusy;

 for(i=0;i<iSize;i++)
  {
   // Check if we can do a 32bit write
   if (VRAMRead.RowsRemaining > 2)
    {
     while(VRAMRead.ImagePtr>=psxVuw_eom)
      VRAMRead.ImagePtr-=524288;
     while(VRAMRead.ImagePtr<psxVuw)
      VRAMRead.ImagePtr+=524288;

//     if(VRAMRead.ImagePtr>=psxVuw &&
//        VRAMRead.ImagePtr<psxVuw_eom)
      *pMem++=lGPUdataRet=*((unsigned long *)VRAMRead.ImagePtr);

     VRAMRead.RowsRemaining-=2;
     VRAMRead.ImagePtr+=2;
     if(VRAMRead.RowsRemaining<=0)
      {
       VRAMRead.RowsRemaining = VRAMRead.Width;
       VRAMRead.ColsRemaining--;
       VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
      }
     if(VRAMRead.ColsRemaining<=0) {FinishedVRAMRead();goto ENDREAD;}
    }
   else
    {
     // Can't do a 32bit read, so settle with 2 seperate 16bit reads
     if ((VRAMRead.ColsRemaining > 0) && (VRAMRead.RowsRemaining > 0))
      {
       while(VRAMRead.ImagePtr>=psxVuw_eom)
        VRAMRead.ImagePtr-=524288;
       while(VRAMRead.ImagePtr<psxVuw)
        VRAMRead.ImagePtr+=524288;

//       if(VRAMRead.ImagePtr>=psxVuw &&
//          VRAMRead.ImagePtr<psxVuw_eom)
        lGPUdataRet=(unsigned long)*VRAMRead.ImagePtr;

       VRAMRead.ImagePtr++;
       VRAMRead.RowsRemaining--;
       if(VRAMRead.RowsRemaining<=0)
        {
         VRAMRead.RowsRemaining = VRAMRead.Width;
         VRAMRead.ColsRemaining--;
         VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
        }
       if(VRAMRead.ColsRemaining <= 0)
        {*pMem=lGPUdataRet;FinishedVRAMRead();goto ENDREAD;}

       while(VRAMRead.ImagePtr>=psxVuw_eom)
        VRAMRead.ImagePtr-=524288;
       while(VRAMRead.ImagePtr<psxVuw)
         VRAMRead.ImagePtr+=524288;

//       if(VRAMRead.ImagePtr>=psxVuw &&
//          VRAMRead.ImagePtr<psxVuw_eom)
        lGPUdataRet|=(unsigned long)(*VRAMRead.ImagePtr)<<16;
       *pMem++=lGPUdataRet;

       VRAMRead.ImagePtr++;
       VRAMRead.RowsRemaining--;
       if(VRAMRead.RowsRemaining<=0)
        {
         VRAMRead.RowsRemaining = VRAMRead.Width;
         VRAMRead.ColsRemaining--;
         VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
        }
       if(VRAMRead.ColsRemaining <= 0)
        {FinishedVRAMRead();goto ENDREAD;}
      }
     else {FinishedVRAMRead();goto ENDREAD;}
    }
  }

ENDREAD:
 GPUIsIdle;
 return;
}


////////////////////////////////////////////////////////////////////////

unsigned long CALLBACK GPU__readData(void)
{
 GPUIsBusy;

 // Check for when a game sets the transfer mode, but is not transfering data
 if((DataReadMode==DR_VRAMTRANSFER) && 
    (VRAMRead.RowsRemaining==0) &&
    (VRAMRead.ColsRemaining == 0))
  DataReadMode = DR_NORMAL;

 if(DataReadMode==DR_VRAMTRANSFER)
  {
   // Check if we can do a 32bit write
   if (VRAMRead.RowsRemaining > 2)
    {
     while(VRAMRead.ImagePtr>=psxVuw_eom)
      VRAMRead.ImagePtr-=524288;
     while(VRAMRead.ImagePtr<psxVuw)
      VRAMRead.ImagePtr+=524288;

//     if(VRAMRead.ImagePtr>=psxVuw &&
//        VRAMRead.ImagePtr<psxVuw_eom)
//      lGPUdataRet = *((unsigned long *)VRAMRead.ImagePtr);
     lGPUdataRet = *VRAMRead.ImagePtr | (*(VRAMRead.ImagePtr+1) << 16);
     VRAMRead.RowsRemaining-=2;
     VRAMRead.ImagePtr+=2;
     if(VRAMRead.RowsRemaining<=0)
      {
       VRAMRead.RowsRemaining = VRAMRead.Width;
       VRAMRead.ColsRemaining--;
       VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
      }
     if(VRAMRead.ColsRemaining<=0) FinishedVRAMRead();
    }
   else
    {
     // Can't do a 32bit read, so settle with 2 seperate 16bit reads
     if ((VRAMRead.ColsRemaining > 0) && (VRAMRead.RowsRemaining > 0))
      {
       while(VRAMRead.ImagePtr>=psxVuw_eom)
        VRAMRead.ImagePtr-=524288;
       while(VRAMRead.ImagePtr<psxVuw)
        VRAMRead.ImagePtr+=524288;

//       if(VRAMRead.ImagePtr>=psxVuw &&
//          VRAMRead.ImagePtr<psxVuw_eom)
//        lGPUdataRet = (unsigned long)*VRAMRead.ImagePtr;
       lGPUdataRet = *VRAMRead.ImagePtr | (*(VRAMRead.ImagePtr+1) << 16);
       VRAMRead.ImagePtr++;
       VRAMRead.RowsRemaining--;
       if(VRAMRead.RowsRemaining<=0)
        {
         VRAMRead.RowsRemaining = VRAMRead.Width;
         VRAMRead.ColsRemaining--;
         VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
        }
       if(VRAMRead.ColsRemaining <= 0)
        FinishedVRAMRead();
       if(DataReadMode==DR_VRAMTRANSFER)
        {
         while(VRAMRead.ImagePtr>=psxVuw_eom)
          VRAMRead.ImagePtr-=524288;
         while(VRAMRead.ImagePtr<psxVuw)
          VRAMRead.ImagePtr+=524288;

//         if(VRAMRead.ImagePtr>=psxVuw &&
//            VRAMRead.ImagePtr<psxVuw_eom)
//          lGPUdataRet|=(unsigned long)(*VRAMRead.ImagePtr)<<16;
          lGPUdataRet|= (*(VRAMRead.ImagePtr+1) << 16);
         VRAMRead.ImagePtr++;
         VRAMRead.RowsRemaining--;
         if(VRAMRead.RowsRemaining<=0)
          {
           VRAMRead.RowsRemaining = VRAMRead.Width;
           VRAMRead.ColsRemaining--;
           VRAMRead.ImagePtr += 1024 - VRAMRead.Width;
          }
         if(VRAMRead.ColsRemaining <= 0) 
          FinishedVRAMRead();
        }
      }
     else FinishedVRAMRead();
    }
  }

 GPUIsIdle;

 return lGPUdataRet;
}

////////////////////////////////////////////////////////////////////////
// processes data send to GPU data register
// extra table for fixing polyline troubles
////////////////////////////////////////////////////////////////////////

const unsigned char primTableCX[256] =
{
    // 00
    0,0,3,0,0,0,0,0,
    // 08
    0,0,0,0,0,0,0,0,
    // 10
    0,0,0,0,0,0,0,0,
    // 18
    0,0,0,0,0,0,0,0,
    // 20
    4,4,4,4,7,7,7,7,
    // 28
    5,5,5,5,9,9,9,9,
    // 30
    6,6,6,6,9,9,9,9,
    // 38
    8,8,8,8,12,12,12,12,
    // 40
    3,3,3,3,0,0,0,0,
    // 48
//  5,5,5,5,6,6,6,6,    // FLINE
    254,254,254,254,254,254,254,254,
    // 50
    4,4,4,4,0,0,0,0,
    // 58
//  7,7,7,7,9,9,9,9,    // GLINE
    255,255,255,255,255,255,255,255,
    // 60
    3,3,3,3,4,4,4,4,    
    // 68
    2,2,2,2,3,3,3,3,    // 3=SPRITE1???
    // 70
    2,2,2,2,3,3,3,3,
    // 78
    2,2,2,2,3,3,3,3,
    // 80
    4,0,0,0,0,0,0,0,
    // 88
    0,0,0,0,0,0,0,0,
    // 90
    0,0,0,0,0,0,0,0,
    // 98
    0,0,0,0,0,0,0,0,
    // a0
    3,0,0,0,0,0,0,0,
    // a8
    0,0,0,0,0,0,0,0,
    // b0
    0,0,0,0,0,0,0,0,
    // b8
    0,0,0,0,0,0,0,0,
    // c0
    3,0,0,0,0,0,0,0,
    // c8
    0,0,0,0,0,0,0,0,
    // d0
    0,0,0,0,0,0,0,0,
    // d8
    0,0,0,0,0,0,0,0,
    // e0
    0,1,1,1,1,1,1,0,
    // e8
    0,0,0,0,0,0,0,0,
    // f0
    0,0,0,0,0,0,0,0,
    // f8
    0,0,0,0,0,0,0,0
};

void CALLBACK GPUwriteDataMem(unsigned long * pMem, int iSize)
{
 unsigned char command;int i=0;
 unsigned long gdata=0;
 GPUIsBusy;
 GPUIsNotReadyForCommands;

 if((DataWriteMode == DR_VRAMTRANSFER) && 
    (VRAMWrite.RowsRemaining == 0) &&
    (VRAMWrite.ColsRemaining == 0))
  DataWriteMode = DR_NORMAL;

 if(DataWriteMode==DR_VRAMTRANSFER)
  {
   while(VRAMWrite.ColsRemaining)
    {
     while(VRAMWrite.RowsRemaining > 2)
      {
       if(i>=iSize) {goto ENDVRAM;}
       i++;
       gdata=*pMem++;

       while(VRAMWrite.ImagePtr>=psxVuw_eom)
        VRAMWrite.ImagePtr-=524288;
       while(VRAMWrite.ImagePtr<psxVuw)
        VRAMWrite.ImagePtr+=524288;
//       if(VRAMWrite.ImagePtr>=psxVuw &&
//          VRAMWrite.ImagePtr<psxVuw_eom)
        *((unsigned long *)VRAMWrite.ImagePtr) = gdata;
       VRAMWrite.RowsRemaining -= 2;
       VRAMWrite.ImagePtr += 2;
      }

     if(VRAMWrite.RowsRemaining)
      {
       if(i>=iSize) goto ENDVRAM;
       i++;
       gdata=*pMem++;

       while(VRAMWrite.ImagePtr>=psxVuw_eom)
        VRAMWrite.ImagePtr-=524288;
       while(VRAMWrite.ImagePtr<psxVuw)
        VRAMWrite.ImagePtr+=524288;

//       if(VRAMWrite.ImagePtr>=psxVuw &&
//          VRAMWrite.ImagePtr<psxVuw_eom)
        *VRAMWrite.ImagePtr = (unsigned short)gdata;

       VRAMWrite.ImagePtr++;
       VRAMWrite.RowsRemaining--;
       if (VRAMWrite.RowsRemaining <= 0)
        {
         VRAMWrite.RowsRemaining = VRAMWrite.Width;
         VRAMWrite.ColsRemaining--;
         VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
         if (VRAMWrite.ColsRemaining <= 0)
          {FinishedVRAMWrite();goto ENDVRAM;}
        }

       while(VRAMWrite.ImagePtr>=psxVuw_eom)
        VRAMWrite.ImagePtr-=524288;
       while(VRAMWrite.ImagePtr<psxVuw)
        VRAMWrite.ImagePtr+=524288;

//       if(VRAMWrite.ImagePtr>=psxVuw &&
//          VRAMWrite.ImagePtr<psxVuw_eom)
        *VRAMWrite.ImagePtr = (unsigned short)(gdata>>16);
       VRAMWrite.ImagePtr++;
       VRAMWrite.RowsRemaining--;
       if (VRAMWrite.RowsRemaining <= 0)
        {
         VRAMWrite.RowsRemaining = VRAMWrite.Width;
         VRAMWrite.ColsRemaining--;
         VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
         if (VRAMWrite.ColsRemaining <= 0) 
          {FinishedVRAMWrite();goto ENDVRAM;}
        }
      }
     else
      {
       VRAMWrite.RowsRemaining = VRAMWrite.Width;
       VRAMWrite.ColsRemaining--;
       VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
      }
    }
   FinishedVRAMWrite();
  }

ENDVRAM:

if(DataWriteMode==DR_NORMAL)
 for(;i<iSize;i++)
  {
   gdata=*pMem++;

     if(gpuDataC == 0)
      {
       command = (unsigned char)((gdata>>24) & 0xff);

       if(primTableCX[command])
        {
         gpuDataC = primTableCX[command];
         gpuCommand = command;
         gpuDataM[0] = gdata;
         gpuDataP = 1;
        }
       else goto ENDDMA;
      }
     else
      {
       gpuDataM[gpuDataP] = gdata;
       if(gpuDataC>128)
        {
         if((gpuDataC==254 && gpuDataP>3) ||
            (gpuDataC==255 && gpuDataP>4))
          {
           if(((gpuDataM[gpuDataP]&0x50000000)==0x50000000) && 
               (LOWORD(gpuDataM[gpuDataP])==HIWORD(gpuDataM[gpuDataP])))
            gpuDataP=gpuDataC-1;
          }
        }
       gpuDataP++;
      }

     if(gpuDataP == gpuDataC)
      {
       gpuDataC=gpuDataP=0;
       if (bSkipNextFrame)
            primTableSkip[gpuCommand]((unsigned char *)gpuDataM);
       else primTableJ[gpuCommand]((unsigned char *)gpuDataM);
      }
    } 

ENDDMA:

 lGPUdataRet=gdata;

 GPUIsReadyForCommands;
 GPUIsIdle;                
}

////////////////////////////////////////////////////////////////////////
 
void DoWriteData(unsigned long gdata)
{
 // Check if we can do a 32bit write
 if (VRAMWrite.RowsRemaining > 2)
  {
   while(VRAMWrite.ImagePtr>=psxVuw_eom)
    VRAMWrite.ImagePtr-=524288;
   while(VRAMWrite.ImagePtr<psxVuw)
    VRAMWrite.ImagePtr+=524288;

//   if(VRAMWrite.ImagePtr>=psxVuw &&
//      VRAMWrite.ImagePtr<psxVuw_eom)
//    *((unsigned long *)VRAMWrite.ImagePtr) = gdata;
   *VRAMWrite.ImagePtr = (unsigned short)gdata;
   *(VRAMWrite.ImagePtr+1) = (unsigned short)(gdata>>16);

   VRAMWrite.RowsRemaining -= 2;
   VRAMWrite.ImagePtr += 2;

   if (VRAMWrite.RowsRemaining <= 0)
    {
     VRAMWrite.RowsRemaining = VRAMWrite.Width;
     VRAMWrite.ColsRemaining--;
     VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
    }

   if (VRAMWrite.ColsRemaining <= 0)
    FinishedVRAMWrite();
  }
 else
  {
   // Can't do a 32bit write, so settle with 2 seperate 16bit writes
   if ((VRAMWrite.ColsRemaining > 0) && (VRAMWrite.RowsRemaining > 0))
    {
     while(VRAMWrite.ImagePtr>=psxVuw_eom)
      VRAMWrite.ImagePtr-=524288;
     while(VRAMWrite.ImagePtr<psxVuw)
      VRAMWrite.ImagePtr+=524288;

//     if(VRAMWrite.ImagePtr>=psxVuw &&              
//        VRAMWrite.ImagePtr<psxVuw_eom)             
     *VRAMWrite.ImagePtr = (unsigned short)gdata;  

     VRAMWrite.ImagePtr++;
     VRAMWrite.RowsRemaining--;
     if (VRAMWrite.RowsRemaining <= 0)
      {
       VRAMWrite.RowsRemaining = VRAMWrite.Width;
       VRAMWrite.ColsRemaining--;
       VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
      }
     if (VRAMWrite.ColsRemaining <= 0)
      FinishedVRAMWrite();

     if (DataWriteMode == DR_VRAMTRANSFER)
      {
       while(VRAMWrite.ImagePtr>=psxVuw_eom)
        VRAMWrite.ImagePtr-=524288;
       while(VRAMWrite.ImagePtr<psxVuw)
        VRAMWrite.ImagePtr+=524288;

//       if(VRAMWrite.ImagePtr>=psxVuw &&
//          VRAMWrite.ImagePtr<psxVuw_eom)
       *VRAMWrite.ImagePtr = (unsigned short)(gdata>>16);
       VRAMWrite.ImagePtr++;
       VRAMWrite.RowsRemaining--;
       if (VRAMWrite.RowsRemaining <= 0)
        {
         VRAMWrite.RowsRemaining = VRAMWrite.Width;
         VRAMWrite.ColsRemaining--;
         VRAMWrite.ImagePtr += 1024 - VRAMWrite.Width;
        }
       if (VRAMWrite.ColsRemaining <= 0) 
        FinishedVRAMWrite();
      }
    }
   else FinishedVRAMWrite();
  }
}

////////////////////////////////////////////////////////////////////////

void CALLBACK GPU__writeData(unsigned long gdata)
{
 unsigned char command;

 GPUIsBusy;
 GPUIsNotReadyForCommands;
 lGPUdataRet=gdata;

 if((DataWriteMode == DR_VRAMTRANSFER) && 
    (VRAMWrite.RowsRemaining == 0) && 
    (VRAMWrite.ColsRemaining == 0))
  DataWriteMode = DR_NORMAL;

 switch (DataWriteMode)
  {
   case DR_VRAMTRANSFER:                               // Image transfer to VRAM
    {
     DoWriteData(gdata);
    } break;

   // Series of GPU commands
   case DR_NORMAL:   
    {
     if(gpuDataC == 0)
      {
       command = (unsigned char)((gdata>>24) & 0xff);
       if(primTableCX[command])
        {
         gpuDataC    = primTableCX[command];
         gpuCommand  = command;
         gpuDataM[0] = gdata;
         gpuDataP    = 1;
        }
       else 
        {
         break;
        }
      }
     else
      {
       gpuDataM[gpuDataP] = gdata;

       if(gpuDataC>128)
        {
         if((gpuDataC==254 && gpuDataP>3) ||
            (gpuDataC==255 && gpuDataP>4))
          {
           if(((gpuDataM[gpuDataP]&0x50000000)==0x50000000) && 
               (LOWORD(gpuDataM[gpuDataP])==HIWORD(gpuDataM[gpuDataP])))
            gpuDataP=gpuDataC-1;
          }
        }

       gpuDataP++;
      }

     if(gpuDataP == gpuDataC)
      {
       gpuDataC=gpuDataP=0;
       if (bSkipNextFrame)
            primTableSkip[gpuCommand]((unsigned char *)gpuDataM);
       else primTableJ[gpuCommand]((unsigned char *)gpuDataM);
      }
     break;
    } 
  }

 GPUIsReadyForCommands;
 GPUIsIdle;

}

////////////////////////////////////////////////////////////////////////
// this functions will be removed soon (or 'soonish')... not really needed, but some emus want them
////////////////////////////////////////////////////////////////////////

void CALLBACK GPUsetMode(unsigned long gdata)
{
// DataWriteMode=(gdata&1)?DR_VRAMTRANSFER:DR_NORMAL;
// DataReadMode =(gdata&2)?DR_VRAMTRANSFER:DR_NORMAL;
}

long CALLBACK GPUgetMode(void)
{
 long iT=0;

 if(DataWriteMode==DR_VRAMTRANSFER) iT|=0x1;
 if(DataReadMode ==DR_VRAMTRANSFER) iT|=0x2;
 return iT;
}

////////////////////////////////////////////////////////////////////////
// call config dlg
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUconfigure(void)
{
#ifdef _WINDOWS
 HWND hWP=GetActiveWindow();

 DialogBox(hInst,MAKEINTRESOURCE(IDD_CFGSOFT),
           hWP,(DLGPROC)SoftDlgProc);
#else // LINUX
// SoftDlgProc();
#endif

 return 0;
}

////////////////////////////////////////////////////////////////////////
// sets all kind of act fixes
////////////////////////////////////////////////////////////////////////

void SetFixes(void)
 {
// Pete : disabled fixes (not needed anymore with the endless loop check)
//  if(dwActFixes&16)
//       iMaxDMACommandCounter=15000;
//  else iMaxDMACommandCounter=2000000;
//  if(dwActFixes&0x80)
//       dwDMAChainStop=0xffffffff;
//  else dwDMAChainStop=0;

  if(dwActFixes & 2) sDispWidths[4]=384;
  else               sDispWidths[4]=368;
 }

////////////////////////////////////////////////////////////////////////
// process gpu commands
////////////////////////////////////////////////////////////////////////

unsigned long lUsedAddr[3];

__inline BOOL CheckForEndlessLoop(unsigned long laddr)
{
 if(laddr==lUsedAddr[1]) return TRUE;
 if(laddr==lUsedAddr[2]) return TRUE;

 if(laddr<lUsedAddr[0]) lUsedAddr[1]=laddr;
 else                   lUsedAddr[2]=laddr;
 lUsedAddr[0]=laddr;
 return FALSE;
}

long CALLBACK GPU__dmaChain(unsigned long * baseAddrL, unsigned long addr)
{
 unsigned long dmaMem;
 //unsigned short CmdCnt;
 unsigned char * baseAddrB;
 short count;
 unsigned int DMACommandCounter = 0;
 void (* *primFunc)(unsigned char *);
 unsigned char command;

 GPUIsBusy;

 if(bSkipNextFrame) primFunc=primTableSkip;
 else               primFunc=primTableJ;

 lUsedAddr[0]=lUsedAddr[1]=lUsedAddr[2]=0xffffff;

 baseAddrB = (unsigned char*) baseAddrL;

 do
  {
   addr&=0x1FFFFF;

   if(DMACommandCounter++ > 2000000) break;
   if(CheckForEndlessLoop(addr)) break;

   count = baseAddrB[addr+3];

   dmaMem=addr+4;

   while(count>0)
    {
     lGPUdataRet=baseAddrL[dmaMem>>2];

     if(DataWriteMode==DR_VRAMTRANSFER &&
        VRAMWrite.ColsRemaining>0 && VRAMWrite.RowsRemaining>0)
      {
       DoWriteData(lGPUdataRet);
       dmaMem+=4;
       count--;
       continue;
      }

     if(gpuDataC == 0)
      {
       command = baseAddrB[dmaMem+3];
       if(primTableCX[command])
        {
         gpuDataC    = primTableCX[command];
         gpuCommand  = command;
         gpuDataM[0] = lGPUdataRet;
         gpuDataP    = 1;
        }
       else
        {
         dmaMem+=4;
         count--;
         continue;
        }
      }
     else
      {
       gpuDataM[gpuDataP] = lGPUdataRet;
       if(gpuDataC>128)
        {
         if((gpuDataC==254 && gpuDataP>3) ||
            (gpuDataC==255 && gpuDataP>4))
          {
           if(((gpuDataM[gpuDataP]&0x50000000)==0x50000000) && 
               (LOWORD(gpuDataM[gpuDataP])==HIWORD(gpuDataM[gpuDataP])))
            gpuDataP=gpuDataC-1;
          }
        }
       gpuDataP++;
      }

     if(gpuDataP == gpuDataC)
      {
       gpuDataC=gpuDataP=0;
       primFunc[gpuCommand]((unsigned char *)gpuDataM);
      }

     dmaMem+=4;
     count--;
    }
   
   addr = baseAddrL[addr>>2]&0xffffff;
  }
 while (addr != 0xffffff);// && (addr != dwDMAChainStop));

 GPUIsIdle;

 return 0;
}

////////////////////////////////////////////////////////////////////////
// show about dlg
////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
BOOL CALLBACK AboutDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {case IDOK:     EndDialog(hW,TRUE);return TRUE;}
    }
  }
 return FALSE;
}
#endif

void CALLBACK GPUabout(void)                           // ABOUT
{
#ifdef _WINDOWS
 HWND hWP=GetActiveWindow();                           // to be sure
 DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),
           hWP,(DLGPROC)AboutDlgProc);
#else // LINUX
#ifndef _FPSE
// AboutDlgProc();
#endif
#endif
 return;
}

////////////////////////////////////////////////////////////////////////
// We are ever fine ;)
////////////////////////////////////////////////////////////////////////

long CALLBACK GPUtest(void)
{
 // if test fails this function should return negative value for error (unable to continue)
 // and positive value for warning (can continue but output might be crappy)
 return 0;
}

////////////////////////////////////////////////////////////////////////
// Freeze
////////////////////////////////////////////////////////////////////////

typedef struct GPUFREEZETAG
{
 unsigned long ulFreezeVersion;      // should be always 1 for now (set by main emu)
 unsigned long ulStatus;             // current gpu status
 unsigned long ulControl[256];       // latest control register values
 unsigned char psxVRam[1024*512*2];  // current VRam image
} GPUFreeze_t;

////////////////////////////////////////////////////////////////////////

long CALLBACK GPUfreeze(unsigned long ulGetFreezeData,GPUFreeze_t * pF)
{
 //----------------------------------------------------//
 if(ulGetFreezeData==2)                                // 2: info, which save slot is selected? (just for display)
  {
   long lSlotNum=*((long *)pF);
   if(lSlotNum<0) return 0;
   if(lSlotNum>8) return 0;
   lSelectedSlot=lSlotNum+1;
   BuildDispMenu(0);
   return 1;
  }
 //----------------------------------------------------//
 if(!pF)                    return 0;                  // some checks
 if(pF->ulFreezeVersion!=1) return 0;

 if(ulGetFreezeData==1)                                // 1: get data
  {
   pF->ulStatus=lGPUstatusRet;
   memcpy(pF->ulControl,ulStatusControl,256*sizeof(unsigned long));
   memcpy(pF->psxVRam,  psxVub,         1024*512*2);

   return 1;
  }

 if(ulGetFreezeData!=0) return 0;                      // 0: set data

 lGPUstatusRet=pF->ulStatus;
 memcpy(ulStatusControl,pF->ulControl,256*sizeof(unsigned long));
 memcpy(psxVub,         pF->psxVRam,  1024*512*2);

// RESET TEXTURE STORE HERE, IF YOU USE SOMETHING LIKE THAT

 GPU__writeStatus(ulStatusControl[0]);
 GPU__writeStatus(ulStatusControl[1]);
 GPU__writeStatus(ulStatusControl[2]);
 GPU__writeStatus(ulStatusControl[3]);
 GPU__writeStatus(ulStatusControl[8]);                   // try to repair things
 GPU__writeStatus(ulStatusControl[6]);
 GPU__writeStatus(ulStatusControl[7]);
 GPU__writeStatus(ulStatusControl[5]);
 GPU__writeStatus(ulStatusControl[4]);

 return 1;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SAVE STATE DISPLAY STUFF
////////////////////////////////////////////////////////////////////////

// font 0-9, 24x20 pixels, 1 byte = 4 dots
// 00 = black
// 01 = white
// 10 = red
// 11 = transparent

unsigned char cFont[10][120]=
{
// 0
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 1
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x05,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x05,0x55,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 2
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x01,0x40,0x00,0x00,
 0x80,0x00,0x05,0x00,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x15,0x55,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 3
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x01,0x54,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 4
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x54,0x00,0x00,
 0x80,0x00,0x01,0x54,0x00,0x00,
 0x80,0x00,0x01,0x54,0x00,0x00,
 0x80,0x00,0x05,0x14,0x00,0x00,
 0x80,0x00,0x14,0x14,0x00,0x00,
 0x80,0x00,0x15,0x55,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x55,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 5
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x15,0x55,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x15,0x54,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 6
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x01,0x54,0x00,0x00,
 0x80,0x00,0x05,0x00,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x14,0x00,0x00,0x00,
 0x80,0x00,0x15,0x54,0x00,0x00,
 0x80,0x00,0x15,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 7
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x15,0x55,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x00,0x50,0x00,0x00,
 0x80,0x00,0x01,0x40,0x00,0x00,
 0x80,0x00,0x01,0x40,0x00,0x00,
 0x80,0x00,0x05,0x00,0x00,0x00,
 0x80,0x00,0x05,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 8
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
},
// 9
{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x05,0x54,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x05,0x00,0x00,
 0x80,0x00,0x14,0x15,0x00,0x00,
 0x80,0x00,0x05,0x55,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x05,0x00,0x00,
 0x80,0x00,0x00,0x14,0x00,0x00,
 0x80,0x00,0x05,0x50,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0x80,0x00,0x00,0x00,0x00,0x00,
 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa
}
};

////////////////////////////////////////////////////////////////////////

void PaintPicDot(unsigned char * p,unsigned char c)
{

 if(c==0) {*p++=0x00;*p++=0x00;*p=0x00;return;}        // black
 if(c==1) {*p++=0xff;*p++=0xff;*p=0xff;return;}        // white
 if(c==2) {*p++=0x00;*p++=0x00;*p=0xff;return;}        // red
                                                       // transparent
}

////////////////////////////////////////////////////////////////////////
// the main emu allocs 128x96x3 bytes, and passes a ptr
// to it in pMem... the plugin has to fill it with
// 8-8-8 bit BGR screen data (Win 24 bit BMP format 
// without header). 
// Beware: the func can be called at any time,
// so you have to use the frontbuffer to get a fully
// rendered picture

#ifdef _WINDOWS
void CALLBACK GPUgetScreenPic(unsigned char * pMem)    
{
 HRESULT ddrval;DDSURFACEDESC xddsd;unsigned char * pf;
 int x,y,c,v;RECT r;
 float XS,YS;

 memset(&xddsd, 0, sizeof(DDSURFACEDESC));
 xddsd.dwSize   = sizeof(DDSURFACEDESC);
 xddsd.dwFlags  = DDSD_WIDTH | DDSD_HEIGHT;
 xddsd.dwWidth  = iResX;
 xddsd.dwHeight = iResY;

 r.left=0; r.right =iResX;
 r.top=0;  r.bottom=iResY;

 if(iWindowMode)
  {
   POINT Point={0,0};
   ClientToScreen(DX.hWnd,&Point);
   r.left+=Point.x;r.right+=Point.x;
   r.top+=Point.y;r.bottom+=Point.y;
  }

 XS=(float)iResX/128;
 YS=(float)iResY/96;

 ddrval=IDirectDrawSurface_Lock(DX.DDSPrimary,NULL, &xddsd, DDLOCK_WAIT|DDLOCK_READONLY, NULL);

 if(ddrval==DDERR_SURFACELOST) IDirectDrawSurface_Restore(DX.DDSPrimary);
 
 pf=pMem;

 if(ddrval==DD_OK)
  {
   unsigned char * ps=(unsigned char *)xddsd.lpSurface;

   if(iDesktopCol==16)
    {
     unsigned short sx;
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned short *)((ps)+
              r.top*xddsd.lPitch+
              (((int)((float)y*YS))*xddsd.lPitch)+
               r.left*2+
               ((int)((float)x*XS))*2));
         *(pf+0)=(sx&0x1f)<<3;
         *(pf+1)=(sx&0x7e0)>>3;
         *(pf+2)=(sx&0xf800)>>8;
         pf+=3;
        }
      }
    }
   else
   if(iDesktopCol==15)
    {
     unsigned short sx;
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned short *)((ps)+
              r.top*xddsd.lPitch+
              (((int)((float)y*YS))*xddsd.lPitch)+
               r.left*2+
               ((int)((float)x*XS))*2));
         *(pf+0)=(sx&0x1f)<<3;
         *(pf+1)=(sx&0x3e0)>>2;
         *(pf+2)=(sx&0x7c00)>>7;
         pf+=3;
        }
      }
    }
   else       
    {
     unsigned long sx;
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned long *)((ps)+
              r.top*xddsd.lPitch+
              (((int)((float)y*YS))*xddsd.lPitch)+
               r.left*4+
               ((int)((float)x*XS))*4));
         *(pf+0)=(unsigned char)((sx&0xff));
         *(pf+1)=(unsigned char)((sx&0xff00)>>8);
         *(pf+2)=(unsigned char)((sx&0xff0000)>>16);
         pf+=3;
        }
      }
    }
  }

 IDirectDrawSurface_Unlock(DX.DDSPrimary,&xddsd);

 /////////////////////////////////////////////////////////////////////
 // generic number/border painter

 pf=pMem+(103*3);                                      // offset to number rect

 for(y=0;y<20;y++)                                     // loop the number rect pixel
  {
   for(x=0;x<6;x++)
    {
     c=cFont[lSelectedSlot][x+y*6];                    // get 4 char dot infos at once (number depends on selected slot)
     v=(c&0xc0)>>6;
     PaintPicDot(pf,(unsigned char)v);pf+=3;                // paint the dots into the rect
     v=(c&0x30)>>4;
     PaintPicDot(pf,(unsigned char)v);pf+=3;
     v=(c&0x0c)>>2;
     PaintPicDot(pf,(unsigned char)v);pf+=3;
     v=c&0x03;
     PaintPicDot(pf,(unsigned char)v);pf+=3;
    }
   pf+=104*3;                                          // next rect y line
  }

 pf=pMem;                                              // ptr to first pos in 128x96 pic
 for(x=0;x<128;x++)                                    // loop top/bottom line
  {
   *(pf+(95*128*3))=0x00;*pf++=0x00;
   *(pf+(95*128*3))=0x00;*pf++=0x00;                   // paint it red
   *(pf+(95*128*3))=0xff;*pf++=0xff;
  }
 pf=pMem;                                              // ptr to first pos
 for(y=0;y<96;y++)                                     // loop left/right line
  {
   *(pf+(127*3))=0x00;*pf++=0x00;
   *(pf+(127*3))=0x00;*pf++=0x00;                      // paint it red
   *(pf+(127*3))=0xff;*pf++=0xff;
   pf+=127*3;                                          // offset to next line
  }
}

#else
// LINUX version:

#ifdef USE_DGA2
#include <X11/extensions/xf86dga.h>
extern XDGADevice *dgaDev;
#endif
extern char * Xpixels;

void GPUgetScreenPic(unsigned char * pMem)
{
 unsigned short c;unsigned char * pf;int x,y;

 float XS=(float)iResX/128;
 float YS=(float)iResY/96;

 pf=pMem;

 memset(pMem, 0, 128*96*3);

 if(Xpixels)
  {
   unsigned char * ps=(unsigned char *)Xpixels;

   if(iDesktopCol==16)
    {
     long lPitch=iResX<<1;
     unsigned short sx;
#ifdef USE_DGA2
     if (!iWindowMode) lPitch+= (dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth) * 2;
#endif
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned short *)((ps)+
              (((int)((float)y*YS))*lPitch)+
               ((int)((float)x*XS))*2));
         *(pf+0)=(sx&0x1f)<<3;
         *(pf+1)=(sx&0x7e0)>>3;
         *(pf+2)=(sx&0xf800)>>8;
         pf+=3;
        }
      }
    }
   else
   if(iDesktopCol==15)
    {
     long lPitch=iResX<<1;
     unsigned short sx;
#ifdef USE_DGA2
     if (!iWindowMode) lPitch+= (dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth) * 2;
#endif
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned short *)((ps)+
              (((int)((float)y*YS))*lPitch)+
               ((int)((float)x*XS))*2));
         *(pf+0)=(sx&0x1f)<<3;
         *(pf+1)=(sx&0x3e0)>>2;
         *(pf+2)=(sx&0x7c00)>>7;
         pf+=3;
        }
      }
    }
   else
    {
     long lPitch=iResX<<2;
     unsigned long sx;
#ifdef USE_DGA2
     if (!iWindowMode) lPitch+= (dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth) * 4;
#endif
     for(y=0;y<96;y++)
      {
       for(x=0;x<128;x++)
        {
         sx=*((unsigned long *)((ps)+
              (((int)((float)y*YS))*lPitch)+
               ((int)((float)x*XS))*4));
         *(pf+0)=(sx&0xff);
         *(pf+1)=(sx&0xff00)>>8;
         *(pf+2)=(sx&0xff0000)>>16;
         pf+=3;
        }
      }
    }
  }

 /////////////////////////////////////////////////////////////////////
 // generic number/border painter

 pf=pMem+(103*3);                                      // offset to number rect

 for(y=0;y<20;y++)                                     // loop the number rect pixel
  {
   for(x=0;x<6;x++)
    {
     c=cFont[lSelectedSlot][x+y*6];                    // get 4 char dot infos at once (number depends on selected slot)
     PaintPicDot(pf,(c&0xc0)>>6);pf+=3;                // paint the dots into the rect
     PaintPicDot(pf,(c&0x30)>>4);pf+=3;
     PaintPicDot(pf,(c&0x0c)>>2);pf+=3;
     PaintPicDot(pf,(c&0x03));   pf+=3;
    }
   pf+=104*3;                                          // next rect y line
  }

 pf=pMem;                                              // ptr to first pos in 128x96 pic
 for(x=0;x<128;x++)                                    // loop top/bottom line
  {
   *(pf+(95*128*3))=0x00;*pf++=0x00;
   *(pf+(95*128*3))=0x00;*pf++=0x00;                   // paint it red
   *(pf+(95*128*3))=0xff;*pf++=0xff;
  }
 pf=pMem;                                              // ptr to first pos
 for(y=0;y<96;y++)                                     // loop left/right line
  {
   *(pf+(127*3))=0x00;*pf++=0x00;
   *(pf+(127*3))=0x00;*pf++=0x00;                      // paint it red
   *(pf+(127*3))=0xff;*pf++=0xff;
   pf+=127*3;                                          // offset to next line
  }
}
#endif

////////////////////////////////////////////////////////////////////////
// func will be called with 128x96x3 BGR data.
// the plugin has to store the data and display
// it in the upper right corner.
// If the func is called with a NULL ptr, you can
// release your picture data and stop displaying
// the screen pic

void CALLBACK GPUshowScreenPic(unsigned char * pMem)
{
 DestroyPic();                                         // destroy old pic data
 if(pMem==0) return;                                   // done
 CreatePic(pMem);                                      // create new pic... don't free pMem or something like that... just read from it
}

////////////////////////////////////////////////////////////////////////
