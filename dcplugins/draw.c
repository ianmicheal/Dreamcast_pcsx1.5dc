/***************************************************************************
                          draw.c  -  description
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
// 2002/03/10 - lu
// - Initial SDL-only blitting function
// - Initial SDL stretch function (using an undocumented SDL 1.2 func)
// - Boht are triggered by -D_SDL -D_SDL2
// 2002/02/18 - linuzappz
// - NoStretch, PIC and Scanlines support for DGA2 (32bit modes untested)
// - Fixed PIC colors in CreatePic for 16/15 bit modes
//
// 2002/02/17 - linuzappz
// - Added DGA2 support, support only with no strecthing disabled (also no FPS display)
//
// 2002/01/13 - linuzappz
// - Added timing for the szDebugText (to 2 secs)
//
// 2002/01/05 - Pete
// - fixed linux stretch centering (no more garbled screens)
//
// 2001/12/30 - Pete
// - Added linux fullscreen desktop switching (non-SDL version, define USE_XF86VM in Makefile)
//
// 2001/12/19 - syo
// - support refresh rate change
// - added  wait VSYNC
//
// 2001/12/16 - Pete
// - Added Windows FPSE RGB24 mode switch
//
// 2001/12/05 - syo (syo68k@geocities.co.jp)
// - modified for "Use system memory" option
//   (Pete: fixed "system memory" save state pic surface)
//
// 2001/11/11 - lu
// - SDL additions
//
// 2001/10/28 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_DRAW

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "prim.h"
#include "menu.h"

////////////////////////////////////////////////////////////////////////////////////
// misc globals
////////////////////////////////////////////////////////////////////////////////////

int            iResX;
int            iResY;
long           lLowerpart;
BOOL           bIsFirstFrame = TRUE;
BOOL           bCheckMask=FALSE;
unsigned short sSetMask=0;
unsigned long  lSetMask=0;
int            iDesktopCol=16;
int            iShowFPS=1;
int            iWinSize;
int            iUseScanLines=0;
int            iUseNoStrechBlt=0;
int            iFastFwd=0;

#ifdef _WINDOWS

////////////////////////////////////////////////////////////////////////
// own swap buffer func (window/fullscreen)
////////////////////////////////////////////////////////////////////////

sDX            DX;
static DDSURFACEDESC       ddsd;
GUID           guiDev;
BOOL           bDeviceOK;
HWND           hWGPU;
int			   iSysMemory=0;
int            iFPSEInterface=0;
int			   iRefreshRate;
BOOL		   bVsync=FALSE;
BOOL		   bVsync_Key=FALSE;

void (*BlitScreen) (unsigned char *,long,long);

////////////////////////////////////////////////////////////////////////

static __inline void WaitVBlank(void)
{
    if(bVsync_Key)
    {
        IDirectDraw2_WaitForVerticalBlank(DX.DD,DDWAITVB_BLOCKBEGIN,0);
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen32(unsigned char * surf,long x,long y)  // BLIT IN 32bit COLOR MODE
{
    unsigned char * pD;unsigned long lu;unsigned short s;
    unsigned int startxy;
    short row,column;
    short dx=(short)PreviousPSXDisplay.Range.x1;
    short dy=(short)PreviousPSXDisplay.DisplayMode.y;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*ddsd.lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    surf+=PreviousPSXDisplay.Range.x0<<2;

    if(PSXDisplay.RGB24)
    {
        if(iFPSEInterface)
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;
                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned long *)((surf)+(column*ddsd.lPitch)+row*4))=
                        0xff000000|(BLUE(lu)<<16)|(GREEN(lu)<<8)|(RED(lu));
                    pD+=3;
                }
            }
        }
        else
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;
                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned long *)((surf)+(column*ddsd.lPitch)+row*4))=
                        0xff000000|(RED(lu)<<16)|(GREEN(lu)<<8)|(BLUE(lu));
                    pD+=3;
                }
            }
        }
    }
    else
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            for(row=0;row<dx;row++)
            {
                s=psxVuw[startxy++];
                *((unsigned long *)((surf)+(column*ddsd.lPitch)+row*4))=
                    ((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen16(unsigned char * surf,long x,long y)  // BLIT IN 16bit COLOR MODE
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=(unsigned short)PreviousPSXDisplay.Range.x1;
    unsigned short dy=(unsigned short)PreviousPSXDisplay.DisplayMode.y;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*ddsd.lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;

        if(iFPSEInterface)
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;

                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*ddsd.lPitch)+(row<<1)))=
                        (unsigned short)(((BLUE(lu)<<8)&0xf800)|((GREEN(lu)<<3)&0x7e0)|(RED(lu)>>3));
                    pD+=3;
                }
            }
        }
        else
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;

                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*ddsd.lPitch)+(row<<1)))=
                        (unsigned short)(((RED(lu)<<8)&0xf800)|((GREEN(lu)<<3)&0x7e0)|(BLUE(lu)>>3));
                    pD+=3;
                }
            }
        }
    }
    else
    {
        unsigned short LineOffset,SurfOffset;
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);
        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (ddsd.lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;

                *DSTPtr++=
                    ((lu<<11)&0xf800f800)|((lu<<1)&0x7c007c0)|((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen15(unsigned char * surf,long x,long y)  // BLIT IN 16bit COLOR MODE
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=(unsigned short)PreviousPSXDisplay.Range.x1;
    unsigned short dy=(unsigned short)PreviousPSXDisplay.DisplayMode.y;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*ddsd.lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;

        if(iFPSEInterface)
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;

                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*ddsd.lPitch)+(row<<1)))=
                        (unsigned short)
                        (((BLUE(lu)<<7)&0x7c00)|
                         ((GREEN(lu)<<2)&0x3e0)|
                         (RED(lu)>>3));
                    pD+=3;
                }
            }
        }
        else
        {
            for(column=0;column<dy;column++)
            {
                startxy=((1024)*(column+y))+x;

                pD=(unsigned char *)&psxVuw[startxy];

                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*ddsd.lPitch)+(row<<1)))=
                        (unsigned short)
                        (((RED(lu)<<7)&0x7c00)|
                         ((GREEN(lu)<<2)&0x3e0)|
                         (BLUE(lu)>>3));
                    pD+=3;
                }
            }
        }
    }
    else
    {
        unsigned short LineOffset,SurfOffset;
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);

        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (ddsd.lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;

                *DSTPtr++=
                    ((lu<<10)&0x7c007c00)|
                    ((lu)&0x3e003e0)|
                    ((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void DoClearScreenBuffer(void)                         // CLEAR DX BUFFER
{
    DDBLTFX     ddbltfx;

    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0x00000000;

    IDirectDrawSurface_Blt(DX.DDSRender,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
}

////////////////////////////////////////////////////////////////////////

void DoClearFrontBuffer(void)                         // CLEAR PRIMARY BUFFER
{
    DDBLTFX     ddbltfx;

    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0x00000000;

    IDirectDrawSurface_Blt(DX.DDSPrimary,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
}

////////////////////////////////////////////////////////////////////////

void NoStretchedBlit(void)
{
    static int iOldDX=0;
    static int iOldDY=0;

    int iDX,iDY;
    int iX=iResX-PreviousPSXDisplay.DisplayMode.x;
    int iY=iResY-PreviousPSXDisplay.DisplayMode.y;

    /*
    float fXS,fYS,fS;
    fXS=(float)iResX/(float)PreviousPSXDisplay.DisplayMode.x;
    fYS=(float)iResY/(float)PreviousPSXDisplay.DisplayMode.y;
    if(fXS<fYS) fS=fXS; else fS=fYS;
    */

    if(iX<0) {iX=0;iDX=iResX;}
    else     {iX=iX/2;iDX=PreviousPSXDisplay.DisplayMode.x;}

    if(iY<0) {iY=0;iDY=iResY;}
    else     {iY=iY/2;iDY=PreviousPSXDisplay.DisplayMode.y;}

    if(iOldDX!=iDX || iOldDY!=iDY)
    {
        DDBLTFX     ddbltfx;
        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0x00000000;
        IDirectDrawSurface_Blt(DX.DDSPrimary,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
        iOldDX=iDX;iOldDY=iDY;
    }

    if(iWindowMode)
    {
        RECT ScreenRect,ViewportRect;
        POINT Point={0,0};
        ClientToScreen(DX.hWnd,&Point);
        Point.x+=iX;Point.y+=iY;

        ScreenRect.left     = Point.x;
        ScreenRect.top      = Point.y;
        ScreenRect.right    = iDX+Point.x;
        ScreenRect.bottom   = iDY+Point.y;

        ViewportRect.left   = 0;
        ViewportRect.top    = 0;
        ViewportRect.right  = iDX;
        ViewportRect.bottom = iDY;

        WaitVBlank();
        IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                               DDBLT_WAIT,NULL);
    }
    else
    {
        RECT ScreenRect,ViewportRect;

        ScreenRect.left     = iX;
        ScreenRect.top      = iY;
        ScreenRect.right    = iDX+iX;
        ScreenRect.bottom   = iDY+iY;

        ViewportRect.left   = 0;
        ViewportRect.top    = 0;
        ViewportRect.right  = iDX;
        ViewportRect.bottom = iDY;

        WaitVBlank();
        IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                               DDBLT_WAIT,NULL);
    }
    if(DX.DDSScreenPic) DisplayPic();
}

void NoStretchedBlitEx(void)
{
    static int iOldDX=0;
    static int iOldDY=0;

    int iDX,iDY,iX,iY;float fXS,fYS,fS;

    if(!PreviousPSXDisplay.DisplayMode.x) return;
    if(!PreviousPSXDisplay.DisplayMode.y) return;

    fXS=(float)iResX/(float)PreviousPSXDisplay.DisplayMode.x;
    fYS=(float)iResY/(float)PreviousPSXDisplay.DisplayMode.y;
    if(fXS<fYS) fS=fXS; else fS=fYS;

    iDX=(int)(PreviousPSXDisplay.DisplayMode.x*fS);
    iDY=(int)(PreviousPSXDisplay.DisplayMode.y*fS);

    iX=iResX-iDX;
    iY=iResY-iDY;

    if(iX<0) iX=0;
    else     iX=iX/2;

    if(iY<0) iY=0;
    else     iY=iY/2;

    if(iOldDX!=iDX || iOldDY!=iDY)
    {
        DDBLTFX     ddbltfx;
        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0x00000000;
        IDirectDrawSurface_Blt(DX.DDSPrimary,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
        iOldDX=iDX;iOldDY=iDY;
    }

    if(iWindowMode)
    {
        RECT ScreenRect,ViewportRect;
        POINT Point={0,0};
        ClientToScreen(DX.hWnd,&Point);
        Point.x+=iX;Point.y+=iY;

        ScreenRect.left     = Point.x;
        ScreenRect.top      = Point.y;
        ScreenRect.right    = iDX+Point.x;
        ScreenRect.bottom   = iDY+Point.y;

        ViewportRect.left   = 0;
        ViewportRect.top    = 0;
        ViewportRect.right  = PreviousPSXDisplay.DisplayMode.x;
        ViewportRect.bottom = PreviousPSXDisplay.DisplayMode.y;

        WaitVBlank();
        IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                               DDBLT_WAIT,NULL);
    }
    else
    {
        RECT ScreenRect,ViewportRect;

        ScreenRect.left     = iX;
        ScreenRect.top      = iY;
        ScreenRect.right    = iDX+iX;
        ScreenRect.bottom   = iDY+iY;

        ViewportRect.left   = 0;
        ViewportRect.top    = 0;
        ViewportRect.right  = PreviousPSXDisplay.DisplayMode.x;
        ViewportRect.bottom = PreviousPSXDisplay.DisplayMode.y;

        WaitVBlank();
        IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                               DDBLT_WAIT,NULL);
    }
    if(DX.DDSScreenPic) DisplayPic();
}

////////////////////////////////////////////////////////////////////////

void DoBufferSwap(void)                                // SWAP BUFFERS
{                                                      // (we don't swap... we blit only)
    HRESULT ddrval;long x,y;

    ddrval=IDirectDrawSurface_Lock(DX.DDSRender,NULL, &ddsd, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL);

    if(ddrval==DDERR_SURFACELOST)
    {
        IDirectDrawSurface_Restore(DX.DDSRender);
    }

    if(ddrval!=DD_OK)
    {
        IDirectDrawSurface_Unlock(DX.DDSRender,&ddsd);
        return;
    }

    //----------------------------------------------------//

    x=PSXDisplay.DisplayPosition.x;
    y=PSXDisplay.DisplayPosition.y;

    //----------------------------------------------------//

    BlitScreen((unsigned char *)ddsd.lpSurface,x,y);      // fill DDSRender surface

    IDirectDrawSurface_Unlock(DX.DDSRender,&ddsd);

    if(ulKeybits&KEY_SHOWFPS) DisplayText();              // paint menu text

if(iUseNoStrechBlt==1) {NoStretchedBlit();return;}
    if(iUseNoStrechBlt==2) {NoStretchedBlitEx();return;}

    if(iWindowMode)
    {
        RECT ScreenRect,ViewportRect;
        POINT Point={0,0};
        ClientToScreen(DX.hWnd,&Point);

        ScreenRect.left     = Point.x;
        ScreenRect.top      = Point.y;
        ScreenRect.right    = iResX+Point.x;
        ScreenRect.bottom   = iResY+Point.y;

        ViewportRect.left   = 0;
        ViewportRect.top    = 0;
        ViewportRect.right  = PreviousPSXDisplay.DisplayMode.x;
        ViewportRect.bottom = PreviousPSXDisplay.DisplayMode.y;

        if(iUseScanLines==2)                                // stupid nvidia scanline mode
        {
            RECT HelperRect={0,0,iResX,iResY};

            WaitVBlank();

            IDirectDrawSurface_Blt(DX.DDSHelper,&HelperRect,DX.DDSRender,&ViewportRect,
                                   DDBLT_WAIT,NULL);
            IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSHelper,&HelperRect,
                                   DDBLT_WAIT,NULL);
        }
        else
        {
            WaitVBlank();
            IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                                   DDBLT_WAIT,NULL);
        }
    }
    else
    {
        RECT ScreenRect={0,0,iResX,iResY},
                        ViewportRect={0,0,PreviousPSXDisplay.DisplayMode.x,
                                      PreviousPSXDisplay.DisplayMode.y};

        if(iUseScanLines==2)                                // stupid nvidia scanline mode
        {
            WaitVBlank();

            IDirectDrawSurface_Blt(DX.DDSHelper,&ScreenRect,DX.DDSRender,&ViewportRect,
                                   DDBLT_WAIT,NULL);
            IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSHelper,&ScreenRect,
                                   DDBLT_WAIT,NULL);
        }
        else
        {
            WaitVBlank();
            IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSRender,&ViewportRect,
                                   DDBLT_WAIT,NULL );
        }
    }

    if(DX.DDSScreenPic) DisplayPic();
}

////////////////////////////////////////////////////////////////////////
// GAMMA
////////////////////////////////////////////////////////////////////////

int iUseGammaVal=2048;

void DXSetGamma(void)
{
    float g;
    if(iUseGammaVal==2048) return;

    g=(float)iUseGammaVal;
    if(g>512) g=((g-512)*2)+512;
    g=0.5f+((g)/1024.0f);

    // some cards will cheat... so we don't trust the caps here
    // if (DD_Caps.dwCaps2 & DDCAPS2_PRIMARYGAMMA)
    {
        float f;DDGAMMARAMP ramp;int i;
        LPDIRECTDRAWGAMMACONTROL DD_Gamma = NULL;

        if FAILED(IDirectDrawSurface_QueryInterface(DX.DDSPrimary,&IID_IDirectDrawGammaControl,(void**)&DD_Gamma))
            return;

        for (i=0;i<256;i++)
        {
            f=(((float)(i*256))*g);
            if(f>65535) f=65535;
            ramp.red[i]=ramp.green[i]=ramp.blue[i]=(WORD)f;
        }

        IDirectDrawGammaControl_SetGammaRamp(DD_Gamma,0,&ramp);
        IDirectDrawGammaControl_Release(DD_Gamma);
    }
}

////////////////////////////////////////////////////////////////////////
// SCAN LINE STUFF
////////////////////////////////////////////////////////////////////////

void SetScanLineList(LPDIRECTDRAWCLIPPER Clipper)
{
    LPRGNDATA lpCL;RECT * pr;int y;POINT Point={0,0};

    IDirectDrawClipper_SetClipList(Clipper,NULL,0);

    lpCL=(LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+((iResY/2)+1)*sizeof(RECT));
    if(iWindowMode) ClientToScreen(DX.hWnd,&Point);

    lpCL->rdh.dwSize=sizeof(RGNDATAHEADER);
    lpCL->rdh.iType=RDH_RECTANGLES;
    lpCL->rdh.nCount=iResY/2;
    lpCL->rdh.nRgnSize=0;
    lpCL->rdh.rcBound.left=Point.x;
    lpCL->rdh.rcBound.top=Point.y;
    lpCL->rdh.rcBound.bottom=Point.y+iResY;
    lpCL->rdh.rcBound.right=Point.x+iResX;

    pr=(RECT *)lpCL->Buffer;
    for(y=0;y<iResY;y+=2)
    {
        pr->left=Point.x;
        pr->top=Point.y+y;
        pr->right=Point.x+iResX;
        pr->bottom=Point.y+y+1;
        pr++;
    }

    IDirectDrawClipper_SetClipList(Clipper,lpCL,0);

    free(lpCL);
}

////////////////////////////////////////////////////////////////////////

void MoveScanLineArea(HWND hwnd)
{
    LPDIRECTDRAWCLIPPER Clipper;HRESULT h;
    DDBLTFX ddbltfx;RECT r;

    if(FAILED(h=IDirectDraw_CreateClipper(DX.DD,0,&Clipper,NULL)))
        return;

    IDirectDrawSurface_SetClipper(DX.DDSPrimary,NULL);

    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0x00000000;

    GetClientRect(hwnd,&r);
    ClientToScreen(hwnd,(LPPOINT)&r.left);
    r.right+=r.left;
    r.bottom+=r.top;

    IDirectDrawSurface_Blt(DX.DDSPrimary,&r,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

    SetScanLineList(Clipper);

    IDirectDrawSurface_SetClipper(DX.DDSPrimary,Clipper);
    IDirectDrawClipper_Release(Clipper);
}

////////////////////////////////////////////////////////////////////////
// MAIN DIRECT DRAW INIT
////////////////////////////////////////////////////////////////////////

BOOL ReStart=FALSE;

int DXinitialize()
{
    LPDIRECTDRAW DD;int i;
    LPDIRECTDRAWCLIPPER Clipper;HRESULT h;
    GUID FAR * guid=0;
    unsigned char * c;
    DDSCAPS ddscaps;
    DDBLTFX ddbltfx;
    DDPIXELFORMAT dd;

    // init some DX vars
    DX.hWnd = (HWND)hWGPU;
    DX.DDSHelper=0;
    DX.DDSScreenPic=0;

    // make guid !
    c=(unsigned char *)&guiDev;
    for(i=0;i<sizeof(GUID);i++,c++)
    {if(*c) {guid=&guiDev;break;}}

    // create dd
    if(DirectDrawCreate(guid,&DD,0))
    {
        MessageBox(NULL, "This GPU requires DirectX!", "Error", MB_OK);
        return 0;
    }

    DX.DD=DD;

    //////////////////////////////////////////////////////// co-op level

    if(iWindowMode)                                       // win mode?
    {
        if(IDirectDraw_SetCooperativeLevel(DX.DD,DX.hWnd,DDSCL_NORMAL))
            return 0;
    }
    else
    {
        if(ReStart)
        {
            if(IDirectDraw_SetCooperativeLevel(DX.DD,DX.hWnd, DDSCL_NORMAL | DDSCL_FULLSCREEN | DDSCL_FPUSETUP))
                return 0;
        }
        else
        {
            if(IDirectDraw_SetCooperativeLevel(DX.DD,DX.hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_FPUSETUP))
                return 0;
        }

        if(iRefreshRate)
        {
            LPDIRECTDRAW2 DD2;
            IDirectDraw_QueryInterface(DX.DD,&IID_IDirectDraw2,(LPVOID *)&DD2);
            if(IDirectDraw2_SetDisplayMode(DD2,iResX,iResY,iColDepth,iRefreshRate,0))
                return 0;
        }
        else
        {
            if(IDirectDraw_SetDisplayMode(DX.DD,iResX,iResY,iColDepth))
                return 0;
        }
    }

    //////////////////////////////////////////////////////// main surfaces

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    memset(&ddscaps, 0, sizeof(DDSCAPS));
    ddsd.dwSize = sizeof(DDSURFACEDESC);

    ddsd.dwFlags = DDSD_CAPS;                             // front buffer

    if(iSysMemory)
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
    else
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

    //ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;//|DDSCAPS_VIDEOMEMORY;
    if(IDirectDraw_CreateSurface(DX.DD,&ddsd, &DX.DDSPrimary, NULL))
        return 0;

    //----------------------------------------------------//
    if(iSysMemory && iUseScanLines==2) iUseScanLines=1;   // pete: nvidia hack not needed on system mem

    if(iUseScanLines==2)                                  // special nvidia hack
    {
        memset(&ddsd, 0, sizeof(DDSURFACEDESC));
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        ddsd.dwWidth        = iResX;
        ddsd.dwHeight       = iResY;

        if(IDirectDraw_CreateSurface(DX.DD,&ddsd, &DX.DDSHelper, NULL))
            return 0;
    }
    //----------------------------------------------------//

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));              // back buffer
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    // ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;//|DDSCAPS_VIDEOMEMORY;
    if(iSysMemory)
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    else
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

    ddsd.dwWidth        = 320; //reaper2k2
    ddsd.dwHeight       = 512;

    if(IDirectDraw_CreateSurface(DX.DD,&ddsd, &DX.DDSRender, NULL))
        return 0;

    // check for desktop color depth
    dd.dwSize=sizeof(DDPIXELFORMAT);
    IDirectDrawSurface_GetPixelFormat(DX.DDSRender,&dd);

    if(dd.dwRBitMask==0x00007c00 &&
            dd.dwGBitMask==0x000003e0 &&
            dd.dwBBitMask==0x0000001f)
    {BlitScreen=BlitScreen15;iDesktopCol=15;}
    else
        if(dd.dwRBitMask==0x0000f800 &&
                dd.dwGBitMask==0x000007e0 &&
                dd.dwBBitMask==0x0000001f)
        {BlitScreen=BlitScreen16;iDesktopCol=16;}
        else {BlitScreen=BlitScreen32;iDesktopCol=32;}

    //////////////////////////////////////////////////////// clipper init

    if(FAILED(h=IDirectDraw_CreateClipper(DX.DD,0,&Clipper,NULL)))
        return 0;

    if(iUseScanLines)
        SetScanLineList(Clipper);
    else IDirectDrawClipper_SetHWnd(Clipper,0,DX.hWnd);

    IDirectDrawSurface_SetClipper(DX.DDSPrimary,Clipper);
    IDirectDrawClipper_Release(Clipper);

    //////////////////////////////////////////////////////// small screen clean up

    DXSetGamma();

    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = 0x00000000;

    IDirectDrawSurface_Blt(DX.DDSPrimary,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
    IDirectDrawSurface_Blt(DX.DDSRender,NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

    //////////////////////////////////////////////////////// finish init

    bUsingTWin=FALSE;

    InitMenu();                                           // menu init

    if(iShowFPS)                                          // fps on startup
    {
        ulKeybits|=KEY_SHOWFPS;
        szDispBuf[0]=0;
        BuildDispMenu(0);
    }

    bIsFirstFrame = FALSE;                                // done

    return 0;
}

////////////////////////////////////////////////////////////////////////
// clean up DX stuff
////////////////////////////////////////////////////////////////////////

void DXcleanup()                                       // DX CLEANUP
{
    CloseMenu();                                          // bye display lists

    if(!bIsFirstFrame)
    {
        if(DX.DDSHelper) IDirectDrawSurface_Release(DX.DDSHelper);
        DX.DDSHelper=0;
        if(DX.DDSScreenPic) IDirectDrawSurface_Release(DX.DDSScreenPic);
        DX.DDSScreenPic=0;
        IDirectDrawSurface_Release(DX.DDSRender);
        IDirectDrawSurface_Release(DX.DDSPrimary);
        IDirectDraw_SetCooperativeLevel(DX.DD,DX.hWnd, DDSCL_NORMAL);
        IDirectDraw_RestoreDisplayMode(DX.DD);
        IDirectDraw_Release(DX.DD);

        ReStart=TRUE;
        bIsFirstFrame = TRUE;
    }
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

DWORD  dwGPUStyle=0;                                   // vars to store some wimdows stuff
HANDLE hGPUMenu=NULL;

unsigned long ulInitDisplay(void)
{
    HDC hdc;RECT r;

    if(iWindowMode)                                       // win mode?
    {
        DWORD dw=GetWindowLong(hWGPU, GWL_STYLE);    // -> adjust wnd style
        dwGPUStyle=dw;
        dw&=~WS_THICKFRAME;
        dw|=WS_BORDER|WS_CAPTION;
        SetWindowLong(hWGPU, GWL_STYLE, dw);

        iResX=LOWORD(iWinSize);iResY=HIWORD(iWinSize);
        ShowWindow(hWGPU,SW_SHOWNORMAL);

        if(iUseScanLines)
            SetWindowPos(hWGPU,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

        MoveWindow(hWGPU,                            // -> move wnd
                   GetSystemMetrics(SM_CXFULLSCREEN)/2-iResX/2,
                   GetSystemMetrics(SM_CYFULLSCREEN)/2-iResY/2,
                   iResX+GetSystemMetrics(SM_CXFIXEDFRAME)+3,
                   iResY+GetSystemMetrics(SM_CYFIXEDFRAME)+GetSystemMetrics(SM_CYCAPTION)+3,
                   TRUE);
        UpdateWindow(hWGPU);                         // -> let windows do some update
    }
    else                                                  // no window mode:
    {
        DWORD dw=GetWindowLong(hWGPU, GWL_STYLE);    // -> adjust wnd style
        dwGPUStyle=dw;
        hGPUMenu=GetMenu(hWGPU);

        dw&=~(WS_THICKFRAME|WS_BORDER|WS_CAPTION);
        SetWindowLong(hWGPU, GWL_STYLE, dw);
        SetMenu(hWGPU,NULL);

        ShowWindow(hWGPU,SW_SHOWMAXIMIZED);          // -> max mode
    }

    r.left=r.top=0;r.right=iResX;r.bottom=iResY;          // init bkg with black
    hdc = GetDC(hWGPU);
    FillRect(hdc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));
    ReleaseDC(hWGPU, hdc);

    DXinitialize();                                       // init direct draw (not D3D... oh, well)

    if(!iWindowMode)                                      // fullscreen mode?
        ShowWindow(hWGPU,SW_SHOWMAXIMIZED);           // -> maximize again (fixes strange DX behavior)

    return 1;
}

////////////////////////////////////////////////////////////////////////

void CloseDisplay(void)
{
    DXcleanup();                                          // cleanup dx

    SetWindowLong(hWGPU, GWL_STYLE,dwGPUStyle);    // repair window
    if(hGPUMenu) SetMenu(hWGPU,(HMENU)hGPUMenu);
}

////////////////////////////////////////////////////////////////////////

void CreatePic(unsigned char * pMem)
{
    DDSURFACEDESC xddsd;HRESULT ddrval;
    unsigned char * ps;int x,y;

    memset(&xddsd, 0, sizeof(DDSURFACEDESC));
    xddsd.dwSize = sizeof(DDSURFACEDESC);
    xddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    // xddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    if(iSysMemory)
        xddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    else
        xddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

    xddsd.dwWidth        = 128;
    xddsd.dwHeight       = 96;

    if(IDirectDraw_CreateSurface(DX.DD,&xddsd, &DX.DDSScreenPic, NULL))
    {DX.DDSScreenPic=0;return;}

    ddrval=IDirectDrawSurface_Lock(DX.DDSScreenPic,NULL, &xddsd, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL);

    if(ddrval==DDERR_SURFACELOST)
    {
        IDirectDrawSurface_Restore(DX.DDSScreenPic);
    }

    if(ddrval!=DD_OK)
    {
        IDirectDrawSurface_Unlock(DX.DDSScreenPic,&xddsd);
        return;
    }

    ps=(unsigned char *)xddsd.lpSurface;

    if(iDesktopCol==16)
    {
        unsigned short s;
        for(y=0;y<96;y++)
        {
            for(x=0;x<128;x++)
            {
                s=(*(pMem+0))>>3;
                s|=((*(pMem+1))&0xfc)<<3;
                s|=((*(pMem+2))&0xf8)<<8;
                pMem+=3;
                *((unsigned short *)(ps+y*xddsd.lPitch+x*2))=s;
            }
        }
    }
    else
        if(iDesktopCol==15)
        {
            unsigned short s;
            for(y=0;y<96;y++)
            {
                for(x=0;x<128;x++)
                {
                    s=(*(pMem+0))>>3;
                    s|=((*(pMem+1))&0xfc)<<2;
                    s|=((*(pMem+2))&0xf8)<<7;
                    pMem+=3;
                    *((unsigned short *)(ps+y*xddsd.lPitch+x*2))=s;
                }
            }
        }
        else
            if(iDesktopCol==32)
            {
                unsigned long l;
                for(y=0;y<96;y++)
                {
                    for(x=0;x<128;x++)
                    {
                        l=  *(pMem+0);
                        l|=(*(pMem+1))<<8;
                        l|=(*(pMem+2))<<16;
                        pMem+=3;
                        *((unsigned long *)(ps+y*xddsd.lPitch+x*4))=l;
                    }
                }
            }

    IDirectDrawSurface_Unlock(DX.DDSScreenPic,&xddsd);
}

///////////////////////////////////////////////////////////////////////////////////////

void DestroyPic(void)
{
    if(DX.DDSScreenPic)
    {
        RECT ScreenRect={iResX-128,0,iResX,96};
        DDBLTFX     ddbltfx;

        if(iWindowMode)
        {
            POINT Point={0,0};
            ClientToScreen(DX.hWnd,&Point);
            ScreenRect.left+=Point.x;
            ScreenRect.right+=Point.x;
            ScreenRect.top+=Point.y;
            ScreenRect.bottom+=Point.y;
        }

        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0x00000000;
        IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

        IDirectDrawSurface_Release(DX.DDSScreenPic);
        DX.DDSScreenPic=0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////

void DisplayPic(void)
{
    RECT ScreenRect={iResX-128,0,iResX,96},
                    HelperRect={0,0,128,96};
    if(iWindowMode)
    {
        POINT Point={0,0};
        ClientToScreen(DX.hWnd,&Point);
        ScreenRect.left+=Point.x;
        ScreenRect.right+=Point.x;
        ScreenRect.top+=Point.y;
        ScreenRect.bottom+=Point.y;
    }

    // ??? eh... no need to wait here!
    // WaitVBlank();

    IDirectDrawSurface_Blt(DX.DDSPrimary,&ScreenRect,DX.DDSScreenPic,&HelperRect,
                           DDBLT_WAIT,NULL);
}

///////////////////////////////////////////////////////////////////////////////////////

void ShowGpuPic(void)
{
    HRSRC hR;HGLOBAL hG;unsigned long * pRMem;
    unsigned char * pMem;int x,y;unsigned long * pDMem;

    // turn off any screen pic, if it does already exist
    if(DX.DDSScreenPic) {DestroyPic();return;}

    if(ulKeybits&KEY_SHOWFPS) {ShowTextGpuPic();return;}

    // load and lock the bitmap (lock is kinda obsolete in win32)
    hR=FindResource(hInst,MAKEINTRESOURCE(IDB_GPU),RT_BITMAP);
    hG=LoadResource(hInst,hR);

    // get long ptr to bmp data
    pRMem=((unsigned long *)LockResource(hG))+10;

    // change the data upside-down
    pMem=(unsigned char *)malloc(128*96*3);

    for(y=0;y<96;y++)
    {
        pDMem=(unsigned long *)(pMem+(95-y)*128*3);
        for(x=0;x<96;x++) *pDMem++=*pRMem++;
    }

    // show the pic
    CreatePic(pMem);

    // clean up
    free(pMem);
    DeleteObject(hG);
}

////////////////////////////////////////////////////////////////////////

void ShowTextGpuPic(void)                              // CREATE TEXT SCREEN PIC
{                                                      // gets an Text and paints
    unsigned char * pMem;BITMAPINFO bmi;                  // it into a rgb24 bitmap
    HDC hdc,hdcMem;HBITMAP hBmp,hBmpMem;HFONT hFontMem;   // to display it in the gpu
    HBRUSH hBrush,hBrushMem;HPEN hPen,hPenMem;
    char szB[256];
    RECT r={0,0,128,96};                                  // size of bmp... don't change that
    COLORREF crFrame = RGB(128,255,128);                  // some example color inits
    COLORREF crBkg   = RGB(0,0,0);
    COLORREF crText  = RGB(0,255,0);

    if(DX.DDSScreenPic) DestroyPic();

    //----------------------------------------------------// creation of the dc & bitmap

    hdc   =GetDC(NULL);                                   // create a dc
    hdcMem=CreateCompatibleDC(hdc);
    ReleaseDC(NULL,hdc);

    memset(&bmi,0,sizeof(BITMAPINFO));                    // create a 24bit dib
    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=128;
    bmi.bmiHeader.biHeight=-96;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=24;
    bmi.bmiHeader.biCompression=BI_RGB;
    hBmp=CreateDIBSection(hdcMem,&bmi,DIB_RGB_COLORS,
                          (void **)&pMem,NULL,0);         // pMem will point to 128x96x3 bitmap data

    hBmpMem   = (HBITMAP)SelectObject(hdcMem,hBmp);       // sel the bmp into the dc

    //----------------------------------------------------// ok, the following is just a drawing example... change it...
    // create & select an additional font... whatever you want to paint, paint it in the dc :)
    hBrush=CreateSolidBrush(crBkg);
    hPen=CreatePen(PS_SOLID,0,crFrame);

    hBrushMem = (HBRUSH)SelectObject(hdcMem,hBrush);
    hPenMem   = (HPEN)SelectObject(hdcMem,hPen);
    hFontMem  = (HFONT)SelectObject(hdcMem,hGFont);

    SetTextColor(hdcMem,crText);
    SetBkColor(hdcMem,crBkg);

    Rectangle(hdcMem,r.left,r.top,r.right,r.bottom);      // our example: fill rect and paint border
    InflateRect(&r,-3,-2);                                // reduce the text area

    LoadString(hInst,IDS_INFO0+iMPos,szB,255);
    DrawText(hdcMem,szB,strlen(szB),&r,                   // paint the text (including clipping and word break)
             DT_LEFT|DT_WORDBREAK);

    //----------------------------------------------------// ok, now store the pMem data, or just call the gpu func

    CreatePic(pMem);

    //----------------------------------------------------// finished, now we clean up... needed, or you will get resource leaks :)

    SelectObject(hdcMem,hBmpMem);                         // sel old mem dc objects
    SelectObject(hdcMem,hBrushMem);
    SelectObject(hdcMem,hPenMem);
    SelectObject(hdcMem,hFontMem);
    DeleteDC(hdcMem);                                     // delete mem dcs
    DeleteObject(hBmp);
    DeleteObject(hBrush);                                 // delete created objects
    DeleteObject(hPen);
}

////////////////////////////////////////////////////////////////////////

#else

#ifndef _SDL
////////////////////////////////////////////////////////////////////////
// X STUFF :)
////////////////////////////////////////////////////////////////////////

#ifdef USE_XF86VM

#include <X11/extensions/xf86vmode.h>
static XF86VidModeModeInfo **modes=0;
static int           iOldMode=0;
static int           bModeChanged=0;

#endif

#ifdef USE_DGA2

#include <X11/extensions/xf86dga.h>
XDGADevice        *dgaDev;
static XDGAMode   *dgaModes;
static int         dgaNModes=0;
static char       *Xpic;
#endif

static Cursor        cursor;
XVisualInfo          vi;
static XVisualInfo   *myvisual;
Display              *display;
static Colormap      colormap;
static Window        window;
static GC            hGC;
static XImage      * Ximage;
static XImage      * XCimage;
static XImage      * XFimage;
static XImage      * XPimage=0 ;
static int           Xpitch;
char *               Xpixels;
char *               pCaptionText;

typedef struct {
#define MWM_HINTS_DECORATIONS   2
    long flags;
    long functions;
    long decorations;
    long input_mode;
} MotifWmHints;

static int fx=0;

// close display

void DestroyDisplay(void)
{
    if(display)
    {
#ifdef USE_DGA2
        if(iWindowMode)
        {
#endif
            XFreeColormap(display, colormap);
            if(hGC)
            {
                XFreeGC(display,hGC);
                hGC = 0;
            }
            if(Ximage)
            {
                XDestroyImage(Ximage);
                Ximage=0;
            }
            if(XCimage)
            {
                XDestroyImage(XCimage);
                XCimage=0;
            }
            if(XFimage)
            {
                XDestroyImage(XFimage);
                XFimage=0;
            }

            XSync(display,False);

#ifdef USE_DGA2
        }
#endif

#ifdef USE_XF86VM

#ifdef USE_DGA2
        if (!iWindowMode)
        {
            XFree(dgaModes);
            XDGACloseFramebuffer(display, DefaultScreen(display));

            XUngrabKeyboard(display, CurrentTime);
        }
#endif
        if(bModeChanged)                                    // -> repair screen mode
        {
            int myscreen=DefaultScreen(display);
            XF86VidModeSwitchToMode(display,myscreen,         // --> switch mode back
                                    modes[iOldMode]);
            XF86VidModeSetViewPort(display,myscreen,0,0);     // --> set viewport upperleft
            free(modes);                                      // --> finally kill mode infos
            bModeChanged=0;                                   // --> done
        }

#endif

        XCloseDisplay(display);
    }
}

int depth=0;

#ifdef USE_DGA2
void XClearScreen()
{
    int y;
    char *ptr = dgaDev->data;

    for (y=0; y<iResY; y++)
    {
        memset(ptr, 0, iResX * (dgaDev->mode.bitsPerPixel / 8));
        ptr+= dgaDev->mode.imageWidth * (dgaDev->mode.bitsPerPixel / 8);
    }
}
#endif

// Create display

void CreateDisplay(void)
{
    XSetWindowAttributes winattr;
    int                  myscreen;
    Screen *             screen;
    XEvent               event;
    XSizeHints           hints;
    XWMHints             wm_hints;
    MotifWmHints         mwmhints;
    Atom                 mwmatom;
    int                  root_window_id=0;
    XGCValues            gcv;
    static int depth_list[] = { 16, 15, 32, 24 };
    int i;

    // Open display
    display=XOpenDisplay(NULL);

    if(!display)
    {
        fprintf (stderr,"Failed to open display!!!\n");
        DestroyDisplay();
        return;
    }

    myscreen=DefaultScreen(display);

    // desktop fullscreen switch
#ifndef USE_XF86VM
    if(!iWindowMode) fx=1;
#else
if(!iWindowMode)
{
    XF86VidModeModeLine mode;
    int nmodes,iC;

    fx=1;                                               // raise flag
    XF86VidModeGetModeLine(display,myscreen,&iC,&mode); // get actual mode info
    if(mode.privsize) XFree(mode.private);              // no need for private stuff
    bModeChanged=0;                                     // init mode change flag
#ifndef USE_DGA2
    if(iResX!=mode.hdisplay || iResY!=mode.vdisplay)    // wanted mode is different?
    {
#endif
        XF86VidModeGetAllModeLines(display,myscreen,      // -> enum all mode infos
                                   &nmodes,&modes);
        if(modes)                                         // -> infos got?
        {
            for(iC=0;iC<nmodes;++iC)                        // -> loop modes
            {
                if(mode.hdisplay==modes[iC]->hdisplay &&      // -> act mode found?
                        mode.vdisplay==modes[iC]->vdisplay)        //    if yes: store mode id
                    iOldMode=iC;

                if(iResX==modes[iC]->hdisplay &&              // -> wanted mode found?
                        iResY==modes[iC]->vdisplay)
                {
#ifndef USE_DGA2
                    XF86VidModeSwitchToMode(display,myscreen,   // --> switch to mode
                                            modes[iC]);
                    XF86VidModeSetViewPort(display,myscreen,0,0);

                    bModeChanged=1;                             // --> raise flag for repairing mode on close
#else
int Evb, Errb;
int Major, Minor;

if (!XDGAQueryExtension(display, &Evb, &Errb))
{
    printf("DGA Extension not Available\n");
    return;
}

if (!XDGAQueryVersion(display, &Major, &Minor) || Major < 2)
{
    printf("DGA Version 2 not Available\n");
    return;
}

dgaModes = XDGAQueryModes(display, myscreen, &dgaNModes);

for (i=0; i<dgaNModes; i++)
{
    if (iResX == dgaModes[i].viewportWidth &&
            iResY == dgaModes[i].viewportHeight)
        break;
}

if (i == dgaNModes) break;

if (XDGAOpenFramebuffer(display, myscreen) == False) break;

dgaDev = XDGASetMode(display, myscreen, dgaModes[i].num);
if (dgaDev == NULL) break;

if (!(dgaDev->mode.flags & XDGAConcurrentAccess)) break;

XDGASetViewport(display, myscreen, 0, 0, XDGAFlipImmediate);
XDGASync(display, myscreen);

XClearScreen();

XGrabKeyboard(display, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync, CurrentTime);
XSelectInput(display, DefaultRootWindow(display), KeyPressMask | KeyReleaseMask);

bModeChanged=1;                             // --> raise flag for repairing mode on close
#endif
                }
            }

            if(bModeChanged==0)                             // -> no mode found?
            {
                free(modes);                                  // --> free infos
                printf("No proper fullscreen mode found!\n"); // --> some info output
            }
        }
#ifndef USE_DGA2
    }
#endif
}
#endif

    screen=DefaultScreenOfDisplay(display);

    myvisual = 0;

    for(i=0;i<4;i++)
        if(XMatchVisualInfo(display,myscreen, depth_list[i], TrueColor, &vi))
        {myvisual = &vi;depth=depth_list[i];break;}

    if(!myvisual)
    {
        fprintf(stderr,"Failed to obtain visual!!!\n");
        DestroyDisplay();
        return;
    }

    if(myvisual->red_mask==0x00007c00 &&
            myvisual->green_mask==0x000003e0 &&
            myvisual->blue_mask==0x0000001f)
    {iColDepth=15;}
    else
        if(myvisual->red_mask==0x0000f800 &&
                myvisual->green_mask==0x000007e0 &&
                myvisual->blue_mask==0x0000001f)
        {iColDepth=16;}
        else
            if(myvisual->red_mask==0x00ff0000 &&
                    myvisual->green_mask==0x0000ff00 &&
                    myvisual->blue_mask==0x000000ff)
            {iColDepth=32;}
            else
            {
                fprintf(stderr,"COLOR DEPTH NOT SUPPORTED!\n");
                fprintf(stderr,"r: %08lx\n",myvisual->red_mask);
                fprintf(stderr,"g: %08lx\n",myvisual->green_mask);
                fprintf(stderr,"b: %08lx\n",myvisual->blue_mask);
                DestroyDisplay();
                return;
            }

    root_window_id=RootWindow(display,DefaultScreen(display));

#ifdef USE_DGA2
    if(!iWindowMode)
    {
        Xpixels = dgaDev->data; return;
    }
#endif

    // pffff... much work for a simple blank cursor... oh, well...
    if(iWindowMode) cursor=XCreateFontCursor(display,XC_trek);
    else
    {
        Pixmap p1,p2;XImage * img;
        XColor b,w;unsigned char * idata;
        XGCValues GCv;
        GC        GCc;

        memset(&b,0,sizeof(XColor));
        memset(&w,0,sizeof(XColor));
        idata=(unsigned char *)malloc(8);
        memset(idata,0,8);

        p1=XCreatePixmap(display,RootWindow(display,myvisual->screen),8,8,1);
        p2=XCreatePixmap(display,RootWindow(display,myvisual->screen),8,8,1);

        img = XCreateImage(display,myvisual->visual,
                           1,XYBitmap,0,idata,8,8,8,1);

        GCv.function   = GXcopy;
        GCv.foreground = ~0;
        GCv.background =  0;
        GCv.plane_mask = AllPlanes;
        GCc = XCreateGC(display,p1,
                        (GCFunction|GCForeground|GCBackground|GCPlaneMask),&GCv);

        XPutImage(display, p1,GCc,img,0,0,0,0,8,8);
        XPutImage(display, p2,GCc,img,0,0,0,0,8,8);
        XFreeGC(display, GCc);

        cursor = XCreatePixmapCursor(display,p1,p2,&b,&w,0,0);

        XFreePixmap(display,p1);
        XFreePixmap(display,p2);
        XDestroyImage(img); // will free idata as well
    }

    colormap=XCreateColormap(display,root_window_id,
                             myvisual->visual,AllocNone);

    winattr.background_pixel=0;
    winattr.border_pixel=WhitePixelOfScreen(screen);
    winattr.bit_gravity=ForgetGravity;
    winattr.win_gravity=NorthWestGravity;
    winattr.backing_store=NotUseful;

    winattr.override_redirect=False;
    winattr.save_under=False;
    winattr.event_mask=0;
    winattr.do_not_propagate_mask=0;
    winattr.colormap=colormap;
    winattr.cursor=None;

    window=XCreateWindow(display,root_window_id,
                         0,0,iResX,iResY,
                         0,myvisual->depth,
                         InputOutput,myvisual->visual,
                         CWBorderPixel | CWBackPixel |
                         CWEventMask | CWDontPropagate |
                         CWColormap | CWCursor,
                         &winattr);

    if(!window)
    {
        fprintf(stderr,"Failed in XCreateWindow()!!!\n");
        DestroyDisplay();
        return;
    }

    hints.flags=PMinSize|PMaxSize;

    if(fx) hints.flags|=USPosition|USSize;
    else   hints.flags|=PSize;

    hints.min_width   = hints.max_width = hints.base_width = iResX;
    hints.min_height= hints.max_height = hints.base_height = iResY;

    wm_hints.input=1;
    wm_hints.flags=InputHint;

    XSetWMHints(display,window,&wm_hints);
    XSetWMNormalHints(display,window,&hints);
    if(pCaptionText)
        XStoreName(display,window,pCaptionText);
    else XStoreName(display,window,"P.E.Op.S SoftX PSX Gpu");

    XDefineCursor(display,window,cursor);

    // hack to get rid of window title bar

    if(fx)
    {
        mwmhints.flags=MWM_HINTS_DECORATIONS;
        mwmhints.decorations=0;
        mwmatom=XInternAtom(display,"_MOTIF_WM_HINTS",0);
        XChangeProperty(display,window,mwmatom,mwmatom,32,
                        PropModeReplace,(unsigned char *)&mwmhints,4);
    }

    // key stuff

    XSelectInput(display,
                 window,
                 FocusChangeMask | ExposureMask |
                 KeyPressMask | KeyReleaseMask
                );

    XMapRaised(display,window);
    XClearWindow(display,window);
    XWindowEvent(display,window,ExposureMask,&event);

    if(fx)                                               // fullscreen
    {
        XResizeWindow(display,window,screen->width,screen->height);

        hints.min_width   = hints.max_width = hints.base_width = screen->width;
        hints.min_height= hints.max_height = hints.base_height = screen->height;

        XSetWMNormalHints(display,window,&hints);
    }

    gcv.graphics_exposures = False;
    hGC = XCreateGC(display,window,
                    GCGraphicsExposures, &gcv);
    if(!hGC)
    {
        fprintf(stderr,"No gfx context!!!\n");
        DestroyDisplay();
    }

    Xpixels = (char *)malloc(220*15*4);
    memset(Xpixels,255,220*15*4);
    XFimage = XCreateImage(display,myvisual->visual,
                           depth, ZPixmap, 0,
                           (char *)Xpixels,
                           220, 15,
                           depth>16 ? 32 : 16,
                           0);

    Xpixels = (char *)malloc(iResY*iResX*4);
    memset(Xpixels,0,iResY*iResX*4);
    XCimage = XCreateImage(display,myvisual->visual,
                           depth, ZPixmap, 0,
                           (char *)Xpixels,
                           iResX, iResY,
                           depth>16 ? 32 : 16,
                           0);

    Xpixels = (char *)malloc(iResY*iResX*4);

    Ximage = XCreateImage(display,myvisual->visual,
                          depth, ZPixmap, 0,
                          (char *)Xpixels,
                          iResX, iResY,
                          depth>16 ? 32 : 16,
                          0);
    Xpitch = Ximage->bytes_per_line;
}
#else  //SDL
////////////////////////////////////////////////////////////////////////
// SDL Stuff ^^
////////////////////////////////////////////////////////////////////////

int           Xpitch,depth=16; //32 reaper2k2
char *        Xpixels;
char *        pCaptionText;

SDL_Surface *display,*XFimage,*XPimage=NULL;
#ifndef _SDL2
SDL_Surface *Ximage=NULL,*XCimage=NULL;
#else
SDL_Surface *Ximage16,*Ximage24;
#endif
//static Uint32 sdl_mask=SDL_HWSURFACE|SDL_HWACCEL;/*place or remove some flags*/
Uint32 sdl_mask=SDL_SWSURFACE | SDL_FULLSCREEN;
//Uint32 sdl_mask= SDL_SWSURFACE | SDL_DOUBLEBUF; 

SDL_Rect rectdst,rectsrc;



void DestroyDisplay(void)
{
    if(display){
#ifdef _SDL2
        if(Ximage16) SDL_FreeSurface(Ximage16);
        if(Ximage24) SDL_FreeSurface(Ximage24);
#else
if(XCimage) SDL_FreeSurface(XCimage);
if(Ximage) SDL_FreeSurface(Ximage);
#endif
        if(XFimage) SDL_FreeSurface(XFimage);

        SDL_FreeSurface(display);//the display is also a surface in SDL
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
void SetDisplay(void){
    if(iWindowMode)
        display = SDL_SetVideoMode(iResX,iResY,depth,sdl_mask);
   // else display = SDL_SetVideoMode(iResX,iResY,depth,SDL_FULLSCREEN|sdl_mask);
   // display = SDL_SetVideoMode(iResX,iResY,depth,SDL_FULLSCREEN|SDL_ANYFORMAT|sdl_mask);
}

void CreateDisplay(void)
{

    if(SDL_InitSubSystem(SDL_INIT_VIDEO)<0)
    {
        fprintf (stderr,"(x) Failed to Init SDL!!!\n");
        return;
    }

    //display = SDL_SetVideoMode(iResX,iResY,depth,sdl_mask);
    display = SDL_SetVideoMode(iResX,iResY,depth,!iWindowMode*SDL_FULLSCREEN|sdl_mask);
#ifndef _SDL2					   
    Ximage = SDL_CreateRGBSurface(sdl_mask,iResX,iResY,depth,0x00ff0000,0x0000ff00,0x000000ff,0);
    XCimage= SDL_CreateRGBSurface(sdl_mask,iResX,iResY,depth,0x00ff0000,0x0000ff00,0x000000ff,0);
#else
//Ximage16= SDL_CreateRGBSurface(sdl_mask,iResX,iResY,16,0x1f,0x1f<<5,0x1f<<10,0);

Ximage16= SDL_CreateRGBSurfaceFrom((void*)psxVub, 1024,512,16,2048 ,0x1f,0x1f<<5,0x1f<<10,0);
Ximage24= SDL_CreateRGBSurfaceFrom((void*)psxVub, 1024*2/3,512 ,24,2048 ,0xFF0000,0xFF00,0xFF,0);
#endif


    XFimage= SDL_CreateRGBSurface(sdl_mask,170,15,depth,0x00ff0000,0x0000ff00,0x000000ff,0);

    iColDepth=depth;
    //memset(XFimage->pixels,255,170*15*4);//really needed???
    //memset(Ximage->pixels,0,ResX*ResY*4);
#ifndef _SDL2
    memset(XCimage->pixels,0,iResX*iResY*4);
#endif

    //Xpitch=iResX*32; no more use
#ifndef _SDL2
    Xpixels=(char *)Ximage->pixels;
#endif
    if(pCaptionText)
        SDL_WM_SetCaption(pCaptionText,NULL);
    else SDL_WM_SetCaption("FPSE Display - P.E.Op.S SoftSDL PSX Gpu",NULL);
}
#endif


////////////////////////////////////////////////////////////////////////
#ifndef _SDL2
void (*BlitScreen) (unsigned char *,long,long);
void (*BlitScreenNS) (unsigned char *,long,long);
void (*XStretchBlt)(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy);

unsigned char * pBackBuffer=0;
#endif
////////////////////////////////////////////////////////////////////////

#ifndef _SDL2

void BlitScreen32(unsigned char * surf,long x,long y)
{
    unsigned char * pD;
    unsigned int startxy;
    unsigned long lu;unsigned short s;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    long lPitch=(dx+PreviousPSXDisplay.Range.x0)<<2;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    surf+=PreviousPSXDisplay.Range.x0<<2;

    if(PSXDisplay.RGB24)
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                    0xff000000|(RED(lu)<<16)|(GREEN(lu)<<8)|(BLUE(lu));
                pD+=3;
            }
        }
    }
    else
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            for(row=0;row<dx;row++)
            {
                s=psxVuw[startxy++];
                *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                    ((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen32NS(unsigned char * surf,long x,long y)
{
    unsigned char * pD;
    unsigned int startxy;
    unsigned long lu;unsigned short s;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    long lPitch=iResX<<2;
#ifdef USE_DGA2
    if (!iWindowMode && (char*)surf == Xpixels)
        lPitch = (dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth) * 4;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    surf+=PreviousPSXDisplay.Range.x0<<2;

    if(PSXDisplay.RGB24)
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                    0xff000000|(RED(lu)<<16)|(GREEN(lu)<<8)|(BLUE(lu));
                pD+=3;
            }
        }
    }
    else
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            for(row=0;row<dx;row++)
            {
                s=psxVuw[startxy++];
                *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                    ((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen32NSSL(unsigned char * surf,long x,long y)
{
    unsigned char * pD;
    unsigned int startxy;
    unsigned long lu;unsigned short s;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    long lPitch=iResX<<2;
#ifdef USE_DGA2
    if (!iWindowMode && (char*)surf == Xpixels)
        lPitch = (dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth) * 4;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    surf+=PreviousPSXDisplay.Range.x0<<2;

    if(PSXDisplay.RGB24)
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            pD=(unsigned char *)&psxVuw[startxy];
            if(column&1)
                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                        0xff000000|(RED(lu)<<16)|(GREEN(lu)<<8)|(BLUE(lu));
                    pD+=3;
                }
        }
    }
    else
    {
        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;
            if(column&1)
                for(row=0;row<dx;row++)
                {
                    s=psxVuw[startxy++];
                    *((unsigned long *)((surf)+(column*lPitch)+(row<<2)))=
                        ((((s<<19)&0xf80000)|((s<<6)&0xf800)|((s>>7)&0xf8))&0xffffff)|0xff000000;
                }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen15(unsigned char * surf,long x,long y)
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*((dx+PreviousPSXDisplay.Range.x0)<<1);
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;
        unsigned short * DSTPtr =(unsigned short *)surf;
        DSTPtr+=PreviousPSXDisplay.Range.x0;

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *DSTPtr++=
                    ((RED(lu)<<7)&0x7c00)|
                    ((GREEN(lu)<<2)&0x3e0)|
                    (BLUE(lu)>>3);
                pD+=3;
            }
            DSTPtr+=PreviousPSXDisplay.Range.x0;
        }
    }
    else
    {
        unsigned long * SRCPtr,* DSTPtr;

        SurfOffset=PreviousPSXDisplay.Range.x0>>1;

        SRCPtr = (unsigned long *)(psxVuw +
                                   (y<<10) + x);
        DSTPtr = ((unsigned long *)surf) + SurfOffset;

        dx>>=1;
        LineOffset = 512 - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;
                *DSTPtr++=
                    ((lu<<10)&0x7c007c00)|
                    ((lu)&0x3e003e0)|
                    ((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen15NS(unsigned char * surf,long x,long y)
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;
    long lPitch=iResX<<1;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)surf == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;
#ifdef USE_DGA2
        if (DGA2fix) lPitch+= dga2Fix*2;
#endif

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *((unsigned short *)((surf)+(column*lPitch)+(row<<1)))=
                    ((RED(lu)<<7)&0x7c00)|
                    ((GREEN(lu)<<2)&0x3e0)|
                    (BLUE(lu)>>3);
                pD+=3;
            }
        }
    }
    else
    {
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);

        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;

                *DSTPtr++=
                    ((lu<<10)&0x7c007c00)|
                    ((lu)&0x3e003e0)|
                    ((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
#ifdef USE_DGA2
            if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen15NSSL(unsigned char * surf,long x,long y)
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;
    long lPitch=iResX<<1;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)surf == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;
#ifdef USE_DGA2
        if (DGA2fix) lPitch+= dga2Fix*2;
#endif

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];
            if(column&1)
                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*lPitch)+(row<<1)))=
                        ((RED(lu)<<7)&0x7c00)|
                        ((GREEN(lu)<<2)&0x3e0)|
                        (BLUE(lu)>>3);
                    pD+=3;
                }
        }
    }
    else
    {
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);

        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            if(column&1)
            {
                for(row=0;row<dx;row++)
                {
                    lu=*SRCPtr++;

                    *DSTPtr++=
                        ((lu<<10)&0x7c007c00)|
                        ((lu)&0x3e003e0)|
                        ((lu>>10)&0x1f001f);
                }
                SRCPtr += LineOffset;
                DSTPtr += SurfOffset;
#ifdef USE_DGA2
                if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
            }
            else
            {
#ifdef USE_DGA2
                if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
                DSTPtr+=iResX>>1;
                SRCPtr+=512;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen16(unsigned char * surf,long x,long y)  // BLIT IN 16bit COLOR MODE
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*((dx+PreviousPSXDisplay.Range.x0)<<1);
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;
        unsigned short * DSTPtr =(unsigned short *)surf;
        DSTPtr+=PreviousPSXDisplay.Range.x0;

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *DSTPtr++=
                    ((RED(lu)<<8)&0xf800)|((GREEN(lu)<<3)&0x7e0)|(BLUE(lu)>>3);
                pD+=3;
            }

            DSTPtr+=PreviousPSXDisplay.Range.x0;
        }
    }
    else
    {
        unsigned long * SRCPtr,* DSTPtr;

        SurfOffset=PreviousPSXDisplay.Range.x0>>1;

        SRCPtr = (unsigned long *)(psxVuw +
                                   (y<<10) + x);
        DSTPtr = ((unsigned long *)surf) + SurfOffset;

        dx>>=1;
        LineOffset = 512 - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;
                *DSTPtr++=
                    ((lu<<11)&0xf800f800)|((lu<<1)&0x7c007c0)|((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen16NS(unsigned char * surf,long x,long y)
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;
    long lPitch=iResX<<1;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)surf == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;
#ifdef USE_DGA2
        if (DGA2fix) lPitch+= dga2Fix*2;
#endif

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];

            for(row=0;row<dx;row++)
            {
                lu=*((unsigned long *)pD);
                *((unsigned short *)((surf)+(column*lPitch)+(row<<1)))=
                    ((RED(lu)<<8)&0xf800)|((GREEN(lu)<<3)&0x7e0)|(BLUE(lu)>>3);
                pD+=3;
            }
        }
    }
    else
    {
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);

        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            for(row=0;row<dx;row++)
            {
                lu=*SRCPtr++;

                *DSTPtr++=
                    ((lu<<11)&0xf800f800)|((lu<<1)&0x7c007c0)|((lu>>10)&0x1f001f);
            }
            SRCPtr += LineOffset;
            DSTPtr += SurfOffset;
#ifdef USE_DGA2
            if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////

void BlitScreen16NSSL(unsigned char * surf,long x,long y)
{
    unsigned long lu;
    unsigned short row,column;
    unsigned short dx=PreviousPSXDisplay.Range.x1;
    unsigned short dy=PreviousPSXDisplay.DisplayMode.y;
    unsigned short LineOffset,SurfOffset;
    long lPitch=iResX<<1;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)surf == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    if(PreviousPSXDisplay.Range.y0)                       // centering needed?
    {
        surf+=PreviousPSXDisplay.Range.y0*lPitch;
        dy-=PreviousPSXDisplay.Range.y0;
    }

    if(PSXDisplay.RGB24)
    {
        unsigned char * pD;unsigned int startxy;

        surf+=PreviousPSXDisplay.Range.x0<<1;
#ifdef USE_DGA2
        if (DGA2fix) lPitch+= dga2Fix*2;
#endif

        for(column=0;column<dy;column++)
        {
            startxy=((1024)*(column+y))+x;

            pD=(unsigned char *)&psxVuw[startxy];

            if(column&1)
                for(row=0;row<dx;row++)
                {
                    lu=*((unsigned long *)pD);
                    *((unsigned short *)((surf)+(column*lPitch)+(row<<1)))=
                        ((RED(lu)<<8)&0xf800)|((GREEN(lu)<<3)&0x7e0)|(BLUE(lu)>>3);
                    pD+=3;
                }
        }
    }
    else
    {
        unsigned long * SRCPtr = (unsigned long *)(psxVuw +
                                 (y<<10) + x);

        unsigned long * DSTPtr =
            ((unsigned long *)surf)+(PreviousPSXDisplay.Range.x0>>1);

        dx>>=1;

        LineOffset = 512 - dx;
        SurfOffset = (lPitch>>2) - dx;

        for(column=0;column<dy;column++)
        {
            if(column&1)
            {
                for(row=0;row<dx;row++)
                {
                    lu=*SRCPtr++;

                    *DSTPtr++=
                        ((lu<<11)&0xf800f800)|((lu<<1)&0x7c007c0)|((lu>>10)&0x1f001f);
                }
                DSTPtr += SurfOffset;
                SRCPtr += LineOffset;
#ifdef USE_DGA2
                if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
            }
            else
            {
                DSTPtr+=iResX>>1;
                SRCPtr+=512;
#ifdef USE_DGA2
                if (DGA2fix) DSTPtr+= dga2Fix/2;
#endif
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void XStretchBlt16(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy)
{
    unsigned short * pSrc=(unsigned short *)pBackBuffer;
    unsigned short * pSrcR=NULL;
    unsigned short * pDst=(unsigned short *)pBB;
    unsigned long * pDstR=NULL;
    int x,y,cyo=-1,cy;
    int xpos, xinc;unsigned short pixel;
    int ypos, yinc,ddx2=ddx>>1;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)pBB == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    xinc = (sdx << 16) / ddx;
    pixel=0;

    ypos=0;
    yinc = (sdy << 16) / ddy;

    for(y=0;y<ddy;y++,ypos+=yinc)
    {
        cy=(ypos>>16);

        if(cy==cyo)
        {
#ifndef USE_DGA2
            pDstR=(unsigned long *)(pDst-ddx);
#else
pDstR=(unsigned long *)(pDst-(ddx+dga2Fix));
#endif
            for(x=0;x<ddx2;x++) *((unsigned long*)pDst)++=*pDstR++;
        }
        else
        {
            cyo=cy;
            pSrcR=pSrc+(cy*sdx);
            xpos = 0x10000L;
            for(x=ddx;x>0;--x)
            {
                while(xpos>=0x10000L)
                {
                    pixel = *pSrcR++;
                    xpos -= 0x10000L;
                }
                *pDst++=pixel;
                xpos += xinc;
            }
        }
#ifdef USE_DGA2
        if (DGA2fix) pDst+= dga2Fix;
#endif
    }
}

////////////////////////////////////////////////////////////////////////

void XStretchBlt16SL(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy)
{
    unsigned short * pSrc=(unsigned short *)pBackBuffer;
    unsigned short * pSrcR=NULL;
    unsigned short * pDst=(unsigned short *)pBB;
    int x,y,cy;
    int xpos, xinc;unsigned short pixel;
    int ypos, yinc;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)pBB == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    xinc = (sdx << 16) / ddx;
    pixel=0;

    ypos=0;
    yinc = ((sdy << 16) / ddy)<<1;

    for(y=0;y<ddy;y+=2,ypos+=yinc)
    {
        cy=(ypos>>16);
        pSrcR=pSrc+(cy*sdx);
        xpos = 0x10000L;
        for(x=ddx;x>0;--x)
        {
            while(xpos>=0x10000L)
            {
                pixel = *pSrcR++;
                xpos -= 0x10000L;
            }
            *pDst++=pixel;
            xpos += xinc;
        }
#ifdef USE_DGA2
        if (DGA2fix) pDst+= dga2Fix*2;
#endif
        pDst+=iResX;
    }
}

////////////////////////////////////////////////////////////////////////

void XStretchBlt32(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy)
{
    unsigned long * pSrc=(unsigned long *)pBackBuffer;
    unsigned long * pSrcR=NULL;
    unsigned long * pDst=(unsigned long *)pBB;
    unsigned long * pDstR=NULL;
    int x,y,cyo=-1,cy;
    int xpos, xinc;unsigned long pixel;
    int ypos, yinc;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)pBB == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    xinc = (sdx << 16) / ddx;
    pixel=0;

    ypos=0;
    yinc = (sdy << 16) / ddy;

    for(y=0;y<ddy;y++,ypos+=yinc)
    {
        cy=(ypos>>16);

        if(cy==cyo)
        {
#ifndef USE_DGA2
            pDstR=pDst-ddx;
#else
pDstR=pDst-(ddx+dga2Fix);
#endif
            for(x=0;x<ddx;x++) *pDst++=*pDstR++;
        }
        else
        {
            cyo=cy;
            pSrcR=pSrc+(cy*sdx);
            xpos = 0x10000L;
            for(x=ddx;x>0;--x)
            {
                while(xpos>=0x10000L)
                {
                    pixel = *pSrcR++;
                    xpos -= 0x10000L;
                }
                *pDst++=pixel;
                xpos += xinc;
            }
        }
#ifdef USE_DGA2
        if (DGA2fix) pDst+= dga2Fix/2;
#endif
    }
}

////////////////////////////////////////////////////////////////////////

void XStretchBlt32SL(unsigned char * pBB,int sdx,int sdy,int ddx,int ddy)
{
    unsigned long * pSrc=(unsigned long *)pBackBuffer;
    unsigned long * pSrcR=NULL;
    unsigned long * pDst=(unsigned long *)pBB;
    int x,y,cy;
    int xpos, xinc;unsigned long pixel;
    int ypos, yinc;
#ifdef USE_DGA2
    int DGA2fix;
    int dga2Fix;
    if (!iWindowMode)
    {
        DGA2fix = (char*)pBB == Xpixels;
        dga2Fix = dgaDev->mode.imageWidth - dgaDev->mode.viewportWidth;
    } else DGA2fix = dga2Fix = 0;
#endif

    xinc = (sdx << 16) / ddx;
    pixel=0;

    ypos=0;
    yinc = ((sdy << 16) / ddy)<<1;

    for(y=0;y<ddy;y+=2,ypos+=yinc)
    {
        cy=(ypos>>16);
        pSrcR=pSrc+(cy*sdx);
        xpos = 0x10000L;
        for(x=ddx;x>0;--x)
        {
            while(xpos>=0x10000L)
            {
                pixel = *pSrcR++;
                xpos -= 0x10000L;
            }
            *pDst++=pixel;
            xpos += xinc;
        }
#ifdef USE_DGA2
        if (DGA2fix) pDst+= dga2Fix;
#endif
        pDst+=iResX;
    }
}

////////////////////////////////////////////////////////////////////////

#ifdef USE_DGA2

void XDGABlit(unsigned char *pSrc, int sw, int sh, int dx, int dy)
{
    unsigned char *pDst;
    int bytesPerPixel = dgaDev->mode.bitsPerPixel / 8;

    for(;dy<sh;dy++)
    {
        pDst = dgaDev->data + dgaDev->mode.imageWidth * dy * bytesPerPixel + dx * bytesPerPixel;
        memcpy(pDst, pSrc, sw * bytesPerPixel);
        pSrc+= sw * bytesPerPixel;
    }
}

#endif
#endif //SDL2
////////////////////////////////////////////////////////////////////////

#include <time.h>
extern time_t tStart;

void NoStretchSwap(void)
{
    static int iOldDX=0;
    static int iOldDY=0;

    int iDX,iDY,iODX=0,iODY=0;
    int iX=iResX-(PreviousPSXDisplay.Range.x1+PreviousPSXDisplay.Range.x0);
    int iY=iResY-PreviousPSXDisplay.DisplayMode.y;

    if(iX<0)
    {
        iX=0;iDX=iResX;
        iODX=PreviousPSXDisplay.Range.x1;
        PreviousPSXDisplay.Range.x1=iResX-PreviousPSXDisplay.Range.x0;
    }
    else {iX=iX/2;iDX=PreviousPSXDisplay.Range.x1+PreviousPSXDisplay.Range.x0;}

    if(iY<0)
    {
        iY=0;iDY=iResY;
        iODY=PreviousPSXDisplay.DisplayMode.y;
        PreviousPSXDisplay.DisplayMode.y=iResY;
    }
    else {iY=iY/2;iDY=PreviousPSXDisplay.DisplayMode.y;}

    if(iOldDX!=iDX || iOldDY!=iDY)
    {
#ifndef _SDL2
        memset(Xpixels,0,iResY*iResX*4);
#endif
#ifndef _SDL
#ifdef USE_DGA2
        if(iWindowMode)
#endif
            XPutImage(display,window,hGC, XCimage,
                      0, 0, 0, 0, iResX,iResY);
#else
rectdst.x=iX;
rectdst.y=iY;
rectdst.w=iDX;
rectdst.h=iDY;
//   SDL_BlitSurface(XCimage,NULL,display,NULL);

SDL_FillRect(display,NULL,0);
#endif

        iOldDX=iDX;iOldDY=iDY;
    }
#ifndef _SDL2
    BlitScreenNS((unsigned char *)Xpixels,
                 PSXDisplay.DisplayPosition.x,
                 PSXDisplay.DisplayPosition.y);
#else
rectsrc.x=PSXDisplay.DisplayPosition.x;
rectsrc.y=PSXDisplay.DisplayPosition.y;
rectsrc.h=PreviousPSXDisplay.DisplayMode.y;
if(PSXDisplay.RGB24)
{

    rectsrc.w=PreviousPSXDisplay.Range.x1/*2/3*/;
    SDL_BlitSurface(Ximage24,&rectsrc,display,&rectdst);
}
else
{

    rectsrc.w=PreviousPSXDisplay.Range.x1;
    SDL_BlitSurface(Ximage16,&rectsrc,display,&rectdst);
}
#endif
    if(iODX) PreviousPSXDisplay.Range.x1=iODX;
    if(iODY) PreviousPSXDisplay.DisplayMode.y=iODY;

#ifndef _SDL
#ifdef USE_DGA2
    if(iWindowMode)
#endif
        XPutImage(display,window,hGC, Ximage,
                  0, 0, iX, iY, iDX,iDY);
#else

#ifndef _SDL2
SDL_BlitSurface(Ximage,NULL,display,&rectdst);
#endif

#endif

    if(ulKeybits&KEY_SHOWFPS) //DisplayText();               // paint menu text
    {
        if(szDebugText[0] && ((time(NULL) - tStart) < 2))
        {
            strcpy(szDispBuf,szDebugText);
        }
        else
        {
            szDebugText[0]=0;
            strcat(szDispBuf,szMenuBuf);
        }
#ifndef _SDL
#ifdef USE_DGA2
        if(iWindowMode)
        {
#endif
            XPutImage(display,window,hGC, XFimage,
                      0, 0, 0, 0, 220,15);
            XDrawString(display,window,hGC,2,13,szDispBuf,strlen(szDispBuf));
#ifdef USE_DGA2
        }
#endif
#else
SDL_WM_SetCaption(szDispBuf,NULL); //just a quick fix,
#endif
    }

    if(XPimage) DisplayPic();

#ifndef _SDL
    XSync(display,False);
#else
SDL_Flip(display);
#endif

}

////////////////////////////////////////////////////////////////////////

void DoBufferSwap(void)                                // SWAP BUFFERS
{                                                      // (we don't swap... we blit only)
 #ifdef _SDL2
    SDL_Surface *buf;
 #endif
    if(iUseNoStrechBlt ||
            (PreviousPSXDisplay.Range.x1+PreviousPSXDisplay.Range.x0 == iResX &&
             PreviousPSXDisplay.DisplayMode.y == iResY))
    {NoStretchSwap();return;}

#ifndef _SDL2
    BlitScreen(pBackBuffer,
               PSXDisplay.DisplayPosition.x,
               PSXDisplay.DisplayPosition.y);

    //----------------------------------------------------//

    XStretchBlt((unsigned char *)Xpixels,
                PreviousPSXDisplay.Range.x1+PreviousPSXDisplay.Range.x0,
                PreviousPSXDisplay.DisplayMode.y,
                iResX,iResY);

    //----------------------------------------------------//
#else

rectdst.x=0;
rectdst.y=0;
rectdst.w=iResX;
rectdst.h=iResY;

rectsrc.x=PSXDisplay.DisplayPosition.x;
rectsrc.y=PSXDisplay.DisplayPosition.y;
rectsrc.h=PreviousPSXDisplay.DisplayMode.y;
rectsrc.w=PreviousPSXDisplay.Range.x1;
if(PSXDisplay.RGB24)
{

    SDL_SoftStretch(buf=SDL_DisplayFormat(Ximage24), &rectsrc,
                    display, &rectdst);
}
else
{
    SDL_SoftStretch(buf=SDL_DisplayFormat(Ximage16), &rectsrc,
                    display, &rectdst);
}
SDL_FreeSurface(buf);
#endif
#ifndef _SDL2  
#ifndef _SDL
#ifdef USE_DGA2
    if (iWindowMode)
#endif
        XPutImage(display,window,hGC, Ximage,
                  0, 0, 0, 0,
                  iResX, iResY);
#else
SDL_BlitSurface(Ximage,NULL,display,NULL);
#endif
#endif
    if(ulKeybits&KEY_SHOWFPS) //DisplayText();               // paint menu text
    {
        if(szDebugText[0] && ((time(NULL) - tStart) < 2))
        {
            strcpy(szDispBuf,szDebugText);
        }
        else
        {
            szDebugText[0]=0;
            strcat(szDispBuf,szMenuBuf);
        }
#ifndef _SDL
#ifdef USE_DGA2
        if (iWindowMode)
        {
#endif
            XPutImage(display,window,hGC, XFimage,
                      0, 0, 0, 0, 220,15);
            XDrawString(display,window,hGC,2,13,szDispBuf,strlen(szDispBuf));
#ifdef USE_DGA2
        }
#endif
#else
SDL_WM_SetCaption(szDispBuf,NULL); //just a quick fix,
#endif
    }

    if(XPimage) DisplayPic();

#ifndef _SDL
    XSync(display,False);
#else
SDL_Flip(display);
#endif

}

////////////////////////////////////////////////////////////////////////

void DoClearScreenBuffer(void)                         // CLEAR DX BUFFER
{
#ifndef _SDL
#ifdef USE_DGA2
    if (iWindowMode)
    {
#endif
        XPutImage(display,window,hGC, XCimage,
                  0, 0, 0, 0, iResX, iResY);
        XSync(display,False);
#ifdef USE_DGA2
    }
#endif
#else
/*
SDL_BlitSurface(XCimage,NULL,display,NULL);*/
SDL_FillRect(display,NULL,0);
SDL_Flip(display);
#endif
}

////////////////////////////////////////////////////////////////////////

void DoClearFrontBuffer(void)                          // CLEAR DX BUFFER
{
#ifndef _SDL
#ifdef USE_DGA2
    if (iWindowMode)
    {
#endif
        XPutImage(display,window,hGC, XCimage,
                  0, 0, 0, 0, iResX, iResY);
        XSync(display,False);
#ifdef USE_DGA2
    }
#endif
#else
SDL_FillRect(display,NULL,0);
SDL_Flip(display);
#endif
}


////////////////////////////////////////////////////////////////////////

int Xinitialize()
{
#ifndef _SDL2
    pBackBuffer=(unsigned char *)malloc(640*512*sizeof(unsigned long)); // reaper2k2
    memset(pBackBuffer,0,640*512*sizeof(unsigned long)); // reaper2k2

    // if(iUseNoStrechBlt)
    //  {
    if(iColDepth==16)
    {
        if(iUseScanLines) BlitScreenNS=BlitScreen16NSSL;
        else              BlitScreenNS=BlitScreen16NS;
        iDesktopCol=16;
    }
    else
        if(iColDepth==15)
        {
            if(iUseScanLines) BlitScreenNS=BlitScreen15NSSL;
            else              BlitScreenNS=BlitScreen15NS;
            iDesktopCol=15;
        }
        else
        {
            if(iUseScanLines) BlitScreenNS=BlitScreen32NSSL;
            else              BlitScreenNS=BlitScreen32NS;
            iDesktopCol=32;
        }
    //  }
    // else
    //  {
    if(iColDepth==16)
    {
        BlitScreen=BlitScreen16;iDesktopCol=16;
        if(iUseScanLines) XStretchBlt=XStretchBlt16SL;
        else              XStretchBlt=XStretchBlt16;
    }
    else
        if(iColDepth==15)
        {
            BlitScreen=BlitScreen15;iDesktopCol=15;
            if(iUseScanLines) XStretchBlt=XStretchBlt16SL;
            else              XStretchBlt=XStretchBlt16;
        }
        else
        {
            BlitScreen=BlitScreen32;iDesktopCol=32;
            if(iUseScanLines) XStretchBlt=XStretchBlt32SL;
            else              XStretchBlt=XStretchBlt32;
        }

    // }
#endif
    bUsingTWin=FALSE;

    InitMenu();

    bIsFirstFrame = FALSE;                                // done

    if(iShowFPS)
    {
        iShowFPS=0;
        ulKeybits|=KEY_SHOWFPS;
        szDispBuf[0]=0;
        BuildDispMenu(0);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////

void Xcleanup()                                        // X CLEANUP
{
    CloseMenu();
#ifndef _SDL2
    if(pBackBuffer)  free(pBackBuffer);
    pBackBuffer=0;
#endif
}

////////////////////////////////////////////////////////////////////////

unsigned long ulInitDisplay(void)
{
    CreateDisplay();                                      // x stuff
    Xinitialize();                                        // init x
    return (unsigned long)display;
}

////////////////////////////////////////////////////////////////////////

void CloseDisplay(void)
{
    Xcleanup();                                           // cleanup dx
    DestroyDisplay();
}

////////////////////////////////////////////////////////////////////////

void CreatePic(unsigned char * pMem)
{
    unsigned char * p=(unsigned char *)malloc(128*96*4);
    unsigned char * ps; int x,y;

    ps=p;

    if(iDesktopCol==16)
    {
        unsigned short s;
        for(y=0;y<96;y++)
        {
            for(x=0;x<128;x++)
            {
                s=(*(pMem+0))>>3;
                s|=((*(pMem+1))&0xfc)<<3;
                s|=((*(pMem+2))&0xf8)<<8;
                pMem+=3;
                *((unsigned short *)(ps+y*256+x*2))=s;
            }
        }
    }
    else
        if(iDesktopCol==15)
        {
            unsigned short s;
            for(y=0;y<96;y++)
            {
                for(x=0;x<128;x++)
                {
                    s=(*(pMem+0))>>3;
                    s|=((*(pMem+1))&0xfc)<<2;
                    s|=((*(pMem+2))&0xf8)<<7;
                    pMem+=3;
                    *((unsigned short *)(ps+y*256+x*2))=s;
                }
            }
        }
        else
            if(iDesktopCol==32)
            {
                unsigned long l;
                for(y=0;y<96;y++)
                {
                    for(x=0;x<128;x++)
                    {
                        l=  *(pMem+0);
                        l|=(*(pMem+1))<<8;
                        l|=(*(pMem+2))<<16;
                        pMem+=3;
                        *((unsigned long *)(ps+y*512+x*4))=l;
                    }
                }
            }

#ifndef _SDL
#ifdef USE_DGA2
    if (!iWindowMode) { Xpic = p; XPimage = (XImage*)1; }
    else
#endif
        XPimage = XCreateImage(display,myvisual->visual,
                               depth, ZPixmap, 0,
                               (char *)p,
                               128, 96,
                               depth>16 ? 32 : 16,
                               0);
#else
XPimage = SDL_CreateRGBSurfaceFrom((void *)p,128,96,
                                   depth,depth*16,
                                   0x00ff0000,0x0000ff00,0x000000ff,
                                   0);/*hmm what about having transparency?
*Set a nonzero value here.
*and set the ALPHA flag ON
*/
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void DestroyPic(void)
{
    if(XPimage)
    {
#ifndef _SDL
#ifdef USE_DGA2
        if (iWindowMode)
        {
#endif
            XPutImage(display,window,hGC, XCimage,
                      0, 0, 0, 0, iResX, iResY);
            XDestroyImage(XPimage);
#ifdef USE_DGA2
        }
#endif
#else
SDL_FillRect(display,NULL,0);
SDL_FreeSurface(XPimage);
#endif
        XPimage=0;
    }
}    

///////////////////////////////////////////////////////////////////////////////////////

void DisplayPic(void)
{
#ifndef _SDL
#ifdef USE_DGA2
    if (!iWindowMode) XDGABlit(Xpic, 128, 96, iResX-128, 0);
    else
    {
#endif
        XPutImage(display,window,hGC, XPimage,
                  0, 0, iResX-128, 0,128,96);
#ifdef USE_DGA2
    }
#endif
#else
rectdst.x=iResX-128;
rectdst.y=0;
rectdst.w=128;
rectdst.h=96;
SDL_BlitSurface(XPimage,NULL,display,&rectdst);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////

void ShowGpuPic(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////

void ShowTextGpuPic(void)
{
}

#endif
