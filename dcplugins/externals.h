/***************************************************************************
                        externals.h -  description
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
// 2002/04/20 - linuzappz
// - added iFastFwd var
//
// 2001/12/22 - syo
// - added vsync & transparent vars
//
// 2001/12/16 - Pete
// - added iFPSEInterface variable
//
// 2001/12/05 - syo
// - added iSysMemory and iStopSaver
//
// 2001/10/28 - Pete  
// - generic cleanup for the Peops release
//
//*************************************************************************// 

/////////////////////////////////////////////////////////////////////////////

#define MAXYLINES     512          
#define MAXYLINESMIN1 511
#define MAXTPAGES     32
#define MAXTPAGESMODE 96
#define CLUTMASK      0x7fff
#define CLUTYMASK     0x1ff

#define SHADETEXBIT(x) ((x>>24) & 0x1)
#define SEMITRANSBIT(x) ((x>>25) & 0x1)
#define PSXRGB(r,g,b) ((g<<10)|(b<<5)|r)

#define DATAREGISTERMODES unsigned short

#define DR_NORMAL        0
#define DR_VRAMTRANSFER  1


#define GPUSTATUS_ODDLINES            0x80000000
#define GPUSTATUS_DMABITS             0x60000000 // Two bits
#define GPUSTATUS_READYFORCOMMANDS    0x10000000
#define GPUSTATUS_READYFORVRAM        0x08000000
#define GPUSTATUS_IDLE                0x04000000
#define GPUSTATUS_DISPLAYDISABLED     0x00800000
#define GPUSTATUS_INTERLACED          0x00400000
#define GPUSTATUS_RGB24               0x00200000
#define GPUSTATUS_PAL                 0x00100000
#define GPUSTATUS_DOUBLEHEIGHT        0x00080000
#define GPUSTATUS_WIDTHBITS           0x00070000 // Three bits
#define GPUSTATUS_MASKENABLED         0x00001000
#define GPUSTATUS_MASKDRAWN           0x00000800
#define GPUSTATUS_DRAWINGALLOWED      0x00000400
#define GPUSTATUS_DITHER              0x00000200

#define GPUIsBusy (lGPUstatusRet &= ~GPUSTATUS_IDLE)
#define GPUIsIdle (lGPUstatusRet |= GPUSTATUS_IDLE)

#define GPUIsNotReadyForCommands (lGPUstatusRet &= ~GPUSTATUS_READYFORCOMMANDS)
#define GPUIsReadyForCommands (lGPUstatusRet |= GPUSTATUS_READYFORCOMMANDS)

/////////////////////////////////////////////////////////////////////////////

typedef struct VRAMLOADTTAG
{
 short x;
 short y;
 short Width;
 short Height;
 short RowsRemaining;
 short ColsRemaining;
 unsigned short *ImagePtr;
} VRAMLoad_t;

/////////////////////////////////////////////////////////////////////////////

typedef struct PSXPOINTTAG
{
 long x;
 long y;
} PSXPoint_t;

typedef struct PSXSPOINTTAG
{
 short x;
 short y;
} PSXSPoint_t;

typedef struct PSXRECTTAG
{
 short x0;
 short x1;
 short y0;
 short y1;
} PSXRect_t;

#ifdef _WINDOWS

typedef struct SDXTAG
{
 LPDIRECTDRAW                   DD;

 LPDIRECTDRAWSURFACE            DDSPrimary;
 LPDIRECTDRAWSURFACE            DDSRender;
 LPDIRECTDRAWSURFACE            DDSHelper;
 LPDIRECTDRAWSURFACE            DDSScreenPic;
 HWND                           hWnd;
} sDX;

#else
// linux defines for some windows stuff

#define FALSE 0
#define TRUE 1
#define BOOL unsigned short
#define LOWORD(l)           ((unsigned short)(l))
#define HIWORD(l)           ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define DWORD unsigned long
#define __int64 long long int 

typedef struct RECTTAG
{
 int left;
 int top;
 int right;
 int bottom;
}RECT;

#endif

/////////////////////////////////////////////////////////////////////////////

typedef struct TWINTAG
{
 PSXRect_t  Position;
} TWin_t;

/////////////////////////////////////////////////////////////////////////////

typedef struct PSXDISPLAYTAG
{
 PSXPoint_t  DisplayModeNew;
 PSXPoint_t  DisplayMode;
 PSXPoint_t  DisplayPosition;
 PSXPoint_t  DisplayEnd;
 
 long        Double;
 long        Height;
 long        PAL;
 long        InterlacedNew;
 long        Interlaced;
 long        RGB24New;
 long        RGB24;
 PSXSPoint_t DrawOffset;
 long        Disabled;
 PSXRect_t   Range;

} PSXDisplay_t;

/////////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
extern HINSTANCE hInst;
#endif
                                
/////////////////////////////////////////////////////////////////////////////

// draw.cpp

#ifndef _IN_DRAW

#ifdef _WINDOWS
extern sDX            DX;
extern HWND           hWGPU; 
extern GUID           guiDev; 
extern int            iRefreshRate;
extern BOOL           bVsync;
extern BOOL           bVsync_Key;
#else
extern char *         pCaptionText;
#endif

extern int            iResX;
extern int            iResY;
extern long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP;
extern long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;
extern short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;
extern long           lLowerpart;
extern BOOL           bIsFirstFrame;
extern int            iWinSize;
extern BOOL           bCheckMask;
extern unsigned short sSetMask;
extern unsigned long  lSetMask;
extern BOOL           bDeviceOK;
extern short          g_m1;
extern short          g_m2;
extern short          g_m3;
extern short          DrawSemiTrans;
extern int            iUseGammaVal;
extern int            iUseScanLines;
extern int            iDesktopCol;
extern int            iUseNoStrechBlt;
extern int            iShowFPS;
extern int            iFastFwd;

#ifdef _WINDOWS
extern int            iSysMemory;
extern int            iFPSEInterface;
#endif

#endif

// prim.cpp

#ifndef _IN_PRIMDRAW

extern BOOL           bUsingTWin;
extern TWin_t         TWin;
extern unsigned long  clutid;
extern unsigned short (*primTableJ[256])(unsigned char *);
extern unsigned short (*primTableSkip[256])(unsigned char *);
extern unsigned short  usMirror;
extern unsigned long  dwCfgFixes;
extern unsigned long  dwActFixes;
extern int            iUseFixes;
extern BOOL           bDoVSyncUpdate;
extern long           drawX;
extern long           drawY;
extern long           drawW;
extern long           drawH;

#endif

// gpu.cpp

#ifndef _IN_GPU

extern VRAMLoad_t     VRAMWrite;
extern VRAMLoad_t     VRAMRead;
extern DATAREGISTERMODES DataWriteMode;
extern DATAREGISTERMODES DataReadMode;
extern int            iColDepth;
extern int            iWindowMode;
extern char           szDispBuf[];
extern char           szMenuBuf[];
extern char           szDebugText[];
extern short          sDispWidths[];
extern BOOL           bDebugText;
extern unsigned int   iMaxDMACommandCounter;
extern unsigned long  dwDMAChainStop;
extern PSXDisplay_t   PSXDisplay;
extern PSXDisplay_t   PreviousPSXDisplay;
extern BOOL           bSkipNextFrame;
extern long           lGPUstatusRet;
extern long           drawingLines;
extern unsigned char  * psxVub;
extern signed char    * psxVsb;
extern unsigned short * psxVuw;
extern signed short   * psxVsw;
extern unsigned long  * psxVul;
extern signed long    * psxVsl;
extern BOOL           bChangeWinMode;
extern long           lSelectedSlot;
extern BOOL           bInitCap;

#endif

// menu.cpp

#ifndef _IN_MENU

extern unsigned long dwCoreFlags;

#ifdef _WINDOWS
extern HFONT hGFont;
extern int   iMPos;
extern BOOL  bTransparent;
#endif

#endif

// key.cpp

#ifndef _IN_KEY

extern unsigned long  ulKeybits;

#ifdef _WINDOWS
extern char           szGPUKeys[];
#endif

#endif

// fps.cpp

#ifndef _IN_FPS

extern int            UseFrameLimit;
extern int            UseFrameSkip;
extern int            iFrameRate;
extern int            iFrameLimit;
extern float          fFrameRateHz;
extern float          fps_skip;
extern float          fps_cur;
#ifdef _WINDOWS
extern int			  iStopSaver;
#endif

#endif

// key.cpp

#ifndef _IN_KEY

#ifndef _WINDOWS
extern char * pConfigFile;
#endif

#endif

