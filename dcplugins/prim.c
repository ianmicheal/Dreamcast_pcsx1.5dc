/***************************************************************************
                          prim.c  -  description
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
// 2002/05/19 - Pete
// - mdec mask bit problem in FF9 fixed
//
// 2002/05/14 - Pete
// - new coord check
//
// 2002/03/29 - Pete
// - tex window coord adjustment - thanx to E}I{
// - faster generic coord check - thanx to E}I{
// - StoreImage wrap (Devilsummoner Soul Hackers)
//
// 2002/03/27 - Pete
// - improved sprite texture wrapping func on _very_ big sprites
//
// 2002/02/23 - Pete
// - added Lunar "ignore blending color" fix
//
// 2002/02/12 - Pete
// - removed "no sprite transparency" and "black poly" fixes
//
// 2002/02/10 - Pete
// - additional Load/MoveImage checks for a few FF9/BOF4 effects
//
// 2001/12/10 - Pete
// - additional coord checks for Nascar and SF2 (and more...?)
//
// 2001/11/08 - Linuzappz
// - BGR24to16 converted to nasm, C version still works: define __i386_ 
//   to use the asm version 
//
// 2001/10/28 - Pete  
// - generic cleanup for the Peops release
//
//*************************************************************************// 

#include "stdafx.h"

#define _IN_PRIMDRAW

#include "externals.h"
#include "gpu.h"
#include "draw.h"
#include "soft.h"

////////////////////////////////////////////////////////////////////////                                          
// globals
////////////////////////////////////////////////////////////////////////

BOOL           bUsingTWin=FALSE;                        
TWin_t         TWin;
unsigned long  clutid;                                 // global clut
unsigned short usMirror=0;                             // sprite mirror
long           drawX;
long           drawY;
long           drawW;
long           drawH;
unsigned long  dwCfgFixes;
unsigned long  dwActFixes=0;
int            iUseFixes;
BOOL           bDoVSyncUpdate=FALSE;

////////////////////////////////////////////////////////////////////////                                          
// Some ASM color convertion by LEWPY
////////////////////////////////////////////////////////////////////////

#ifdef __i386__

#define BGR24to16 i386_BGR24to16
__inline unsigned short BGR24to16 (unsigned long BGR);

#else

__inline unsigned short BGR24to16 (unsigned long BGR)
{
 return (unsigned short)(((BGR>>3)&0x1f)|((BGR&0xf80000)>>9)|((BGR&0xf800)>>6));
}

#endif

////////////////////////////////////////////////////////////////////////                                          
// Update global TP infos
////////////////////////////////////////////////////////////////////////

__inline void UpdateGlobalTP(unsigned short gdata)
{
 GlobalTextAddrX = (gdata << 6) & 0x3c0;               // texture addr
 GlobalTextAddrY = (gdata << 4) & 0x100;

 usMirror=gdata&0x1000;

 GlobalTextTP = (gdata >> 7) & 0x3;                    // tex mode (4,8,15)

 if(GlobalTextTP==3) GlobalTextTP=2;                   // seen in Wild9 :(

 GlobalTextABR = (gdata >> 5) & 0x3;                   // blend mode
     
 lGPUstatusRet&=~0x07ff;                                   // Clear the necessary bits
 lGPUstatusRet|=(gdata & 0x07ff);                          // set the necessary bits
}

////////////////////////////////////////////////////////////////////////                                          

__inline void SetRenderMode(unsigned long DrawAttributes)
{
 DrawSemiTrans = (SEMITRANSBIT(DrawAttributes)) ? TRUE : FALSE;

 if(SHADETEXBIT(DrawAttributes)) 
  {g_m1=g_m2=g_m3=128;}
 else
  {
   if((dwActFixes&4) && ((DrawAttributes&0x00ffffff)==0))
    DrawAttributes|=0x007f7f7f;

   g_m1=(short)(DrawAttributes&0xff);
   g_m2=(short)((DrawAttributes>>8)&0xff);
   g_m3=(short)((DrawAttributes>>16)&0xff);
  }
}

////////////////////////////////////////////////////////////////////////

void AdjustCoord4()
{
 lx0=(short)(((int)lx0<<20)>>20);
 lx1=(short)(((int)lx1<<20)>>20);
 lx2=(short)(((int)lx2<<20)>>20);
 lx3=(short)(((int)lx3<<20)>>20);
 ly0=(short)(((int)ly0<<20)>>20);
 ly1=(short)(((int)ly1<<20)>>20);
 ly2=(short)(((int)ly2<<20)>>20);
 ly3=(short)(((int)ly3<<20)>>20);
}

void AdjustCoord3()
{
 lx0=(short)(((int)lx0<<20)>>20);
 lx1=(short)(((int)lx1<<20)>>20);
 lx2=(short)(((int)lx2<<20)>>20);
 ly0=(short)(((int)ly0<<20)>>20);
 ly1=(short)(((int)ly1<<20)>>20);
 ly2=(short)(((int)ly2<<20)>>20);
}

void AdjustCoord2()
{
 lx0=(short)(((int)lx0<<20)>>20);
 lx1=(short)(((int)lx1<<20)>>20);
 ly0=(short)(((int)ly0<<20)>>20);
 ly1=(short)(((int)ly1<<20)>>20);
}

void AdjustCoord1()
{
 lx0=(short)(((int)lx0<<20)>>20);
 ly0=(short)(((int)ly0<<20)>>20);
}

////////////////////////////////////////////////////////////////////////
// special checks... nascar, syphon filter 2, mgs
////////////////////////////////////////////////////////////////////////

__inline BOOL CheckCoord4()
{
 if ((lx3 <= -1024) ||
     (lx2 <= -1024) ||
     (lx1 <= -1024) ||
     (lx0 <= -1024) ||
     (ly3 <= -1024) ||
     (ly2 <= -1024) ||
     (ly1 <= -1024) ||
     (ly0 <= -1024) || 
     (ly3 >= 1023)  ||
     (ly2 >= 1023)  ||
     (ly1 >= 1023)  ||
     (ly0 >= 1023)  ||
     (lx3 >= 1024)  ||
     (lx2 >= 1024)  ||
     (lx1 >= 1024)  ||
     (lx0 >= 1024)) 
     
  {
   return TRUE;
  }

 if((lx0 == 1023) && ((ly0+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx1 == 1023) && ((ly1+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx2 == 1023) && ((ly2+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx3 == 1023) && ((ly3+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 return FALSE;
}

__inline BOOL CheckCoord3()
{
 if ((lx2 <= -1024) ||
     (lx1 <= -1024) ||
     (lx0 <= -1024) ||
     (ly2 <= -1024) ||
     (ly1 <= -1024) ||
     (ly0 <= -1024) || 
     (ly2 >= 1023)  ||
     (ly1 >= 1023)  ||
     (ly0 >= 1023)  || 
     (lx2 >= 1024)  ||
     (lx1 >= 1024)  ||
     (lx0 >= 1024))
  {
   return TRUE;
  }

 if((lx0 == 1023) && ((ly0+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx1 == 1023) && ((ly1+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx2 == 1023) && ((ly2+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 return FALSE;
}


__inline BOOL CheckCoord2()
{
 if ((lx1 <= -1024) ||
     (ly1 <= -1024) ||
     (lx0 <= -1024) ||
     (ly0 <= -1024) || 
     (ly1 >= 1023)  ||
     (ly0 >= 1023)  ||
     (lx1 >= 1024)  ||
     (lx0 >= 1024))
  {
   return TRUE;
  }

 if((lx0 == 1023) && ((ly0+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((lx1 == 1023) && ((ly1+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 return FALSE;
}

__inline BOOL CheckCoordL(short slx0,short sly0,short slx1,short sly1)
{
 if ((slx1 <= -1024) ||
     (sly1 <= -1024) ||
     (slx0 <= -1024) ||
     (sly0 <= -1024) || 
     (sly1 >= 1023)  ||
     (sly0 >= 1023)  ||
     (slx1 >= 1024)  ||
     (slx0 >= 1024))
  {
   return TRUE;
  }

 if((slx0 == 1023) && ((sly0+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 if((slx1 == 1023) && ((sly1+PSXDisplay.DrawOffset.y)<drawY))
  return TRUE;
 return FALSE;
}


////////////////////////////////////////////////////////////////////////
// mask stuff... used in silent hill
////////////////////////////////////////////////////////////////////////

unsigned short cmdSTP(unsigned char * baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];

 lGPUstatusRet&=~0x1800;                                   // Clear the necessary bits
 lGPUstatusRet|=((gdata & 0x02) << 11);                    // Set the necessary bits

 if(gdata&3) {sSetMask=0x8000;lSetMask=0x80008000;}
 else        {sSetMask=0;     lSetMask=0;}

 if(gdata&2) bCheckMask=TRUE;
 else        bCheckMask=FALSE;

 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: Set texture page infos
////////////////////////////////////////////////////////////////////////

unsigned short cmdTexturePage(unsigned char * baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];

 UpdateGlobalTP((unsigned short)gdata);
 GlobalTextREST = (gdata&0x00ffffff)>>9;

 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: turn on/off texture window
////////////////////////////////////////////////////////////////////////

unsigned short cmdTextureWindow(unsigned char *baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];
 unsigned long Align;

 if(dwActFixes&1)            Align=0x1e;               // FF7 fix... needed by ALL emus, so is it a gpu issue???
 else                        Align=0x1f;

 TWin.Position.y0 = (short)(((gdata >> 15) & Align) << 3);
 TWin.Position.x0 = (short)(((gdata >> 10) & Align) << 3);
 TWin.Position.y1 = (short)((gdata >>  5) & 0x1f);
 TWin.Position.x1 = (short)(gdata & 0x1f);
 TWin.Position.y1 = 256-(TWin.Position.y1 << 3);
 TWin.Position.x1 = 256-(TWin.Position.x1 << 3);

 if((TWin.Position.x0 == 0 &&                          // tw turned off
     TWin.Position.y0 == 0 &&
     TWin.Position.x1 == 0 &&
     TWin.Position.y1 == 0) ||  
     (TWin.Position.x1 == 256 &&
      TWin.Position.y1 == 256))
  {
   bUsingTWin = FALSE;                                 // -> just do it
  }                                                    
 else                                                  // tw turned on
  {
   if((TWin.Position.x0+TWin.Position.x1)>256)
    TWin.Position.x0=256-TWin.Position.x1;
   if((TWin.Position.y0+TWin.Position.y1)>256)
    TWin.Position.y0=256-TWin.Position.y1;

   bUsingTWin = TRUE;
  }

 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: start of drawing area... primitives will be clipped inside
////////////////////////////////////////////////////////////////////////

unsigned short cmdDrawAreaStart(unsigned char * baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];

 drawX  = gdata & 0x3ff;                               // for soft drawing
 drawY  = (gdata>>10)&0x3ff;

 if(drawY>=512) drawY=511;                             // some security

 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: end of drawing area... primitives will be clipped inside
////////////////////////////////////////////////////////////////////////

unsigned short cmdDrawAreaEnd(unsigned char * baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];

 drawW  = gdata & 0x3ff;                               // for soft drawing
 drawH  = (gdata>>10)&0x3ff;
 if(drawH>=512) drawH=511;                             // some security
 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: draw offset... will be added to prim coords
////////////////////////////////////////////////////////////////////////

unsigned short cmdDrawOffset(unsigned char * baseAddr)
{
 unsigned long gdata = ((unsigned long*)baseAddr)[0];

 PreviousPSXDisplay.DrawOffset.y = 
  PSXDisplay.DrawOffset.y = (short)((gdata>>11) & 0x7ff);
 PreviousPSXDisplay.DrawOffset.x = 
  PSXDisplay.DrawOffset.x = (short)(gdata & 0x7ff);

 PSXDisplay.DrawOffset.y=(short)(((int)PSXDisplay.DrawOffset.y<<21)>>21);
 PSXDisplay.DrawOffset.x=(short)(((int)PSXDisplay.DrawOffset.x<<21)>>21);

/*
 if (PSXDisplay.DrawOffset.y & 0x400)                  // take care of sign
     PSXDisplay.DrawOffset.y |= 0xf800;
 if (PSXDisplay.DrawOffset.x & 0x400)
     PSXDisplay.DrawOffset.x |= 0xf800;
*/
 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: load image to vram
////////////////////////////////////////////////////////////////////////

unsigned short primLoadImage(unsigned char * baseAddr)
{
 unsigned short *sgpuData = ((unsigned short *) baseAddr);

 VRAMWrite.x      = sgpuData[2]&0x3ff;
 VRAMWrite.y      = sgpuData[3]&0x1ff;
 VRAMWrite.Width  = sgpuData[4];
 VRAMWrite.Height = sgpuData[5];

 DataWriteMode = DR_VRAMTRANSFER;

 VRAMWrite.ImagePtr = psxVuw + (VRAMWrite.y<<10) + VRAMWrite.x;
 VRAMWrite.RowsRemaining = VRAMWrite.Width;
 VRAMWrite.ColsRemaining = VRAMWrite.Height;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: vram -> psx mem
////////////////////////////////////////////////////////////////////////

unsigned short primStoreImage(unsigned char * baseAddr)
{
 unsigned short *sgpuData = ((unsigned short *) baseAddr);

 VRAMRead.x      = sgpuData[2]&0x03ff;
 VRAMRead.y      = sgpuData[3]&0x01ff;
 VRAMRead.Width  = sgpuData[4];
 VRAMRead.Height = sgpuData[5];

 VRAMRead.ImagePtr = psxVuw + (VRAMRead.y<<10) + VRAMRead.x;
 VRAMRead.RowsRemaining = VRAMRead.Width;
 VRAMRead.ColsRemaining = VRAMRead.Height;

 DataReadMode = DR_VRAMTRANSFER;

 lGPUstatusRet |= GPUSTATUS_READYFORVRAM;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: blkfill - NO primitive! Doesn't care about draw areas...
////////////////////////////////////////////////////////////////////////

unsigned short primBlkFill(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);
          
         
 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sW = sgpuData[4] & 0x3ff;
 short sH = sgpuData[5] & 0x3ff;

 // Increase H & W if they are one short of full values, because they never can be full values
 if (sH >= 1023) sH=1024;
 if (sW >= 1023) sW=1024; 
        
 // x and y of end pos
 sW+=sX;
 sH+=sY;

 FillSoftwareArea(sX, sY, sW, sH, BGR24to16(gpuData[0]));

 bDoVSyncUpdate=TRUE;

 return 3;
}
 
////////////////////////////////////////////////////////////////////////
// cmd: move image vram -> vram
////////////////////////////////////////////////////////////////////////

unsigned short primMoveImage(unsigned char * baseAddr)
{
 short *sgpuData = ((short *) baseAddr);

 short imageY0,imageX0,imageY1,imageX1,imageSX,imageSY,i,j;

 imageX0 = sgpuData[2]&0x03ff;
 imageY0 = sgpuData[3]&0x01ff;
 imageX1 = sgpuData[4]&0x03ff;
 imageY1 = sgpuData[5]&0x01ff;
 imageSX = sgpuData[6];
 imageSY = sgpuData[7];

 if((imageX0 == imageX1) && (imageY0 == imageY1)) return 4; 
 if(imageSX<=0)  return 4;
 if(imageSY<=0)  return 4;

 if((imageY0+imageSY)>MAXYLINES ||
    (imageX0+imageSX)>1024      ||
    (imageY1+imageSY)>MAXYLINES ||
    (imageX1+imageSX)>1024)
  {
   int i,j;
   for(j=0;j<imageSY;j++)
    for(i=0;i<imageSX;i++)
     psxVuw [(1024*((imageY1+j)&0x1ff))+((imageX1+i)&0x3ff)]=
      psxVuw[(1024*((imageY0+j)&0x1ff))+((imageX0+i)&0x3ff)];

   bDoVSyncUpdate=TRUE;
 
   return 4;
  }
 
 if(1) // imageSX&1)                                         // not dword aligned? slower func
  {
   unsigned short *SRCPtr, *DSTPtr;
   unsigned short LineOffset;

   SRCPtr = psxVuw + (1024*imageY0) + imageX0;
   DSTPtr = psxVuw + (1024*imageY1) + imageX1;

   LineOffset = 1024 - imageSX;

   for(j=0;j<imageSY;j++)
    {
     for(i=0;i<imageSX;i++) *DSTPtr++ = *SRCPtr++;
     SRCPtr += LineOffset;
     DSTPtr += LineOffset;
    }
  }
 else                                                  // dword aligned
  {
   unsigned long *SRCPtr, *DSTPtr;
   unsigned short LineOffset;
   int dx=imageSX>>1;

   SRCPtr = (unsigned long *)(psxVuw + (1024*imageY0) + imageX0);
   DSTPtr = (unsigned long *)(psxVuw + (1024*imageY1) + imageX1);

   LineOffset = 512 - dx;

   for(j=0;j<imageSY;j++)
    {
     for(i=0;i<dx;i++) {
//     	*DSTPtr++ = *SRCPtr++;
       	 *((unsigned short*)DSTPtr)=*((unsigned short*)SRCPtr);
       	 *(((unsigned short*)DSTPtr)+1)=*(((unsigned short*)SRCPtr)+1);
       	 DSTPtr++; SRCPtr++;
	 }
     SRCPtr += LineOffset;
     DSTPtr += LineOffset;
    }
  }

 imageSX+=imageX1;
 imageSY+=imageY1;

 if(!PSXDisplay.Interlaced)                            // stupid frame skip stuff
  {
   if(UseFrameSkip &&
      imageX1<PSXDisplay.DisplayEnd.x &&
      imageSX>=PSXDisplay.DisplayPosition.x &&
      imageY1<PSXDisplay.DisplayEnd.y &&
      imageSY>=PSXDisplay.DisplayPosition.y)
    updateDisplay();
  }

 bDoVSyncUpdate=TRUE;

 return 4;
}

////////////////////////////////////////////////////////////////////////
// cmd: draw free-size Tile 
////////////////////////////////////////////////////////////////////////

unsigned short primTileS(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long*)baseAddr);
 short *sgpuData = ((short *) baseAddr);

 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sW = sgpuData[4] & 0x3ff;
 short sH = sgpuData[5] & 0x1ff;

 // x and y of start

 ly0 = ly1 = sY    +PSXDisplay.DrawOffset.y;
 ly2 = ly3 = sY+sH +PSXDisplay.DrawOffset.y;
 lx0 = lx3 = sX    +PSXDisplay.DrawOffset.x;
 lx1 = lx2 = sX+sW +PSXDisplay.DrawOffset.x;

 if(!(dwActFixes&8)) AdjustCoord4();

 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 FillSoftwareAreaTrans(lx0,ly0,lx2,ly2,
                       BGR24to16(gpuData[0]));          // Takes Start and Offset

 bDoVSyncUpdate=TRUE;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: draw 1 dot Tile (point)
////////////////////////////////////////////////////////////////////////

unsigned short primTile1(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long*)baseAddr);
 short *sgpuData = ((short *) baseAddr);

 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sH = 1;
 short sW = 1;

 // x and y of start
 ly0 = ly1 = sY    +PSXDisplay.DrawOffset.y;
 ly2 = ly3 = sY+sH +PSXDisplay.DrawOffset.y;
 lx0 = lx3 = sX    +PSXDisplay.DrawOffset.x;
 lx1 = lx2 = sX+sW +PSXDisplay.DrawOffset.x;

 if(!(dwActFixes&8)) AdjustCoord4();

 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 FillSoftwareAreaTrans(lx0,ly0,lx2,ly2,
                       BGR24to16(gpuData[0]));          // Takes Start and Offset

 bDoVSyncUpdate=TRUE;

 return 2;
}

////////////////////////////////////////////////////////////////////////
// cmd: draw 8 dot Tile (small rect)
////////////////////////////////////////////////////////////////////////

unsigned short primTile8(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long*)baseAddr);
 short *sgpuData = ((short *) baseAddr);

 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sH = 8;
 short sW = 8;

 // x and y of start
 ly0 = ly1 = sY    +PSXDisplay.DrawOffset.y;
 ly2 = ly3 = sY+sH +PSXDisplay.DrawOffset.y;
 lx0 = lx3 = sX    +PSXDisplay.DrawOffset.x;
 lx1 = lx2 = sX+sW +PSXDisplay.DrawOffset.x;

 if(!(dwActFixes&8)) AdjustCoord4();

 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 FillSoftwareAreaTrans(lx0,ly0,lx2,ly2,
                       BGR24to16(gpuData[0]));          // Takes Start and Offset

 bDoVSyncUpdate=TRUE;

 return 2;
}

////////////////////////////////////////////////////////////////////////
// cmd: draw 16 dot Tile (medium rect)
////////////////////////////////////////////////////////////////////////

unsigned short primTile16(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long*)baseAddr);
 short *sgpuData = ((short *) baseAddr);

 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sH = 16;
 short sW = 16;

 // x and y of start
 ly0 = ly1 = sY    +PSXDisplay.DrawOffset.y;
 ly2 = ly3 = sY+sH +PSXDisplay.DrawOffset.y;
 lx0 = lx3 = sX    +PSXDisplay.DrawOffset.x;
 lx1 = lx2 = sX+sW +PSXDisplay.DrawOffset.x;

 if(!(dwActFixes&8)) AdjustCoord4();

 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 FillSoftwareAreaTrans(lx0,ly0,lx2,ly2,
                       BGR24to16(gpuData[0]));          // Takes Start and Offset

 bDoVSyncUpdate=TRUE;

 return 2;
}

////////////////////////////////////////////////////////////////////////
// cmd: small sprite (textured rect)
////////////////////////////////////////////////////////////////////////

unsigned short primSprt8(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];

 if(!(dwActFixes&8)) AdjustCoord1();

 SetRenderMode(gpuData[0]);

 if(bUsingTWin) DrawSoftwareSpriteTWin(baseAddr,8,8);
 else
 if(usMirror)   DrawSoftwareSpriteMirror(baseAddr,8,8);
 else           DrawSoftwareSprite(baseAddr,8,8,
                                   baseAddr[8],
                                   baseAddr[9]);

 bDoVSyncUpdate=TRUE;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: medium sprite (textured rect)
////////////////////////////////////////////////////////////////////////

unsigned short primSprt16(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];

 if(!(dwActFixes&8)) AdjustCoord1();

 SetRenderMode(gpuData[0]);

 if(bUsingTWin) DrawSoftwareSpriteTWin(baseAddr,16,16);
 else
 if(usMirror)   DrawSoftwareSpriteMirror(baseAddr,16,16);
 else           DrawSoftwareSprite(baseAddr,16,16,
                                   baseAddr[8],
                                   baseAddr[9]);

 bDoVSyncUpdate=TRUE;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: free-size sprite (textured rect)
////////////////////////////////////////////////////////////////////////

// func used on texture coord wrap
void primSprtSRest(unsigned char * baseAddr,unsigned short type)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);
 unsigned short sTypeRest=0;

 short s;
 short sX = sgpuData[2];
 short sY = sgpuData[3];
 short sW = sgpuData[6] & 0x3ff;
 short sH = sgpuData[7] & 0x1ff;
 short tX = baseAddr[8];
 short tY = baseAddr[9];

 switch(type)
  {
   case 1:
    s=256-baseAddr[8];
    sW-=s;
    sX+=s;
    tX=0;
    break;
   case 2:
    s=256-baseAddr[9];
    sH-=s;
    sY+=s;
    tY=0;
    break;
   case 3:
    s=256-baseAddr[8];
    sW-=s;
    sX+=s;
    tX=0;
    s=256-baseAddr[9];
    sH-=s;
    sY+=s;
    tY=0;
    break;
   case 4:
    s=512-baseAddr[8];
    sW-=s;
    sX+=s;
    tX=0;
    break;
   case 5:
    s=512-baseAddr[9];
    sH-=s;
    sY+=s;
    tY=0;
    break;
   case 6:
    s=512-baseAddr[8];
    sW-=s;
    sX+=s;
    tX=0;
    s=512-baseAddr[9];
    sH-=s;
    sY+=s;
    tY=0;
    break;
  }

 SetRenderMode(gpuData[0]);

 if(tX+sW>256) {sW=256-tX;sTypeRest+=1;}
 if(tY+sH>256) {sH=256-tY;sTypeRest+=2;}

 lx0 = sX;
 ly0 = sY;

 if(!(dwActFixes&8)) AdjustCoord1();

 DrawSoftwareSprite(baseAddr,sW,sH,tX,tY);

 if(sTypeRest && type<4)  
  {
   if(sTypeRest&1  && type==1)  primSprtSRest(baseAddr,4);
   if(sTypeRest&2  && type==2)  primSprtSRest(baseAddr,5);
   if(sTypeRest==3 && type==3)  primSprtSRest(baseAddr,6);
  }

}

////////////////////////////////////////////////////////////////////////

unsigned short primSprtS(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);
 short sW,sH;

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];

 if(!(dwActFixes&8)) AdjustCoord1();

 sW = sgpuData[6] & 0x3ff;
 sH = sgpuData[7] & 0x1ff;


 SetRenderMode(gpuData[0]);

 if(bUsingTWin) DrawSoftwareSpriteTWin(baseAddr,sW,sH);
 else
 if(usMirror)   DrawSoftwareSpriteMirror(baseAddr,sW,sH);
 else          
  {
   unsigned short sTypeRest=0;
   short tX=baseAddr[8];
   short tY=baseAddr[9];

   if(tX+sW>256) {sW=256-tX;sTypeRest+=1;}
   if(tY+sH>256) {sH=256-tY;sTypeRest+=2;}

   DrawSoftwareSprite(baseAddr,sW,sH,tX,tY);

   if(sTypeRest) 
    {
     if(sTypeRest&1)  primSprtSRest(baseAddr,1);
     if(sTypeRest&2)  primSprtSRest(baseAddr,2);
     if(sTypeRest==3) primSprtSRest(baseAddr,3);
    }

  }

 bDoVSyncUpdate=TRUE;

 return 4;
}

////////////////////////////////////////////////////////////////////////
// cmd: flat shaded Poly4
////////////////////////////////////////////////////////////////////////

unsigned short primPolyF4(unsigned char *baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[4];
 ly1 = sgpuData[5];
 lx2 = sgpuData[6];
 ly2 = sgpuData[7];
 lx3 = sgpuData[8];
 ly3 = sgpuData[9];

 if(!(dwActFixes&8)) 
  {
   AdjustCoord4();
   if(CheckCoord4()) return 5;
  }

 offsetPSX4();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 drawPoly4F(gpuData[0]);

 bDoVSyncUpdate=TRUE;

 return 5;
}

////////////////////////////////////////////////////////////////////////
// cmd: smooth shaded Poly4
////////////////////////////////////////////////////////////////////////

unsigned short primPolyG4(unsigned char * baseAddr)
{
 unsigned long *gpuData = (unsigned long *)baseAddr;
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[6];
 ly1 = sgpuData[7];
 lx2 = sgpuData[10];
 ly2 = sgpuData[11];
 lx3 = sgpuData[14];
 ly3 = sgpuData[15];

 if(!(dwActFixes&8))
  {
   AdjustCoord4();
   if(CheckCoord4()) return 8;
  }

 offsetPSX4();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 drawPoly4G(gpuData[0], gpuData[2], gpuData[4], gpuData[6]);

 bDoVSyncUpdate=TRUE;

 return 8;
}

////////////////////////////////////////////////////////////////////////
// cmd: flat shaded Texture3
////////////////////////////////////////////////////////////////////////

unsigned short primPolyFT3(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[6];
 ly1 = sgpuData[7];
 lx2 = sgpuData[10];
 ly2 = sgpuData[11];

 if(!(dwActFixes&8))
  {
   AdjustCoord3();
   if(CheckCoord3()) return 7;
  }

 lLowerpart=gpuData[4]>>16;
 UpdateGlobalTP((unsigned short)lLowerpart);

 offsetPSX3();
 SetRenderMode(gpuData[0]);

 drawPoly3FT(baseAddr);

 bDoVSyncUpdate=TRUE;

 return 7;
}

////////////////////////////////////////////////////////////////////////
// cmd: flat shaded Texture4
////////////////////////////////////////////////////////////////////////

unsigned short primPolyFT4(unsigned char * baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[6];
 ly1 = sgpuData[7];
 lx2 = sgpuData[10];
 ly2 = sgpuData[11];
 lx3 = sgpuData[14];
 ly3 = sgpuData[15];

 if(!(dwActFixes&8))
  {
   AdjustCoord4();
   if(CheckCoord4()) return 9;
  }

 lLowerpart=gpuData[4]>>16;
 UpdateGlobalTP((unsigned short)lLowerpart);

 offsetPSX4();

 SetRenderMode(gpuData[0]);

 drawPoly4FT(baseAddr);

 bDoVSyncUpdate=TRUE;

 return 9;
}

////////////////////////////////////////////////////////////////////////
// cmd: smooth shaded Texture3
////////////////////////////////////////////////////////////////////////

unsigned short primPolyGT3(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[8];
 ly1 = sgpuData[9];
 lx2 = sgpuData[14];
 ly2 = sgpuData[15];

 if(!(dwActFixes&8))
  {
   AdjustCoord3();
   if(CheckCoord3()) return 9;
  }

 lLowerpart=gpuData[5]>>16;
 UpdateGlobalTP((unsigned short)lLowerpart);
           
 offsetPSX3();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 if(SHADETEXBIT(gpuData[0]))
  {
   gpuData[0]=(gpuData[0]&0xff000000)|0x00808080;
   gpuData[3]=(gpuData[3]&0xff000000)|0x00808080;
   gpuData[6]=(gpuData[6]&0xff000000)|0x00808080;
  }

 drawPoly3GT(baseAddr);

 bDoVSyncUpdate=TRUE;

 return 9;
}

////////////////////////////////////////////////////////////////////////
// cmd: smooth shaded Poly3
////////////////////////////////////////////////////////////////////////

unsigned short primPolyG3(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[6];
 ly1 = sgpuData[7];
 lx2 = sgpuData[10];
 ly2 = sgpuData[11];

 if(!(dwActFixes&8))
  {
   AdjustCoord3();
   if(CheckCoord3()) return 6;
  }

 offsetPSX3();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 drawPoly3G(gpuData[0], gpuData[2], gpuData[4]);

 bDoVSyncUpdate=TRUE;

 return 6;
}

////////////////////////////////////////////////////////////////////////
// cmd: smooth shaded Texture4
////////////////////////////////////////////////////////////////////////

unsigned short primPolyGT4(unsigned char *baseAddr)
{ 
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[8];
 ly1 = sgpuData[9];
 lx2 = sgpuData[14];
 ly2 = sgpuData[15];
 lx3 = sgpuData[20];
 ly3 = sgpuData[21];

 if(!(dwActFixes&8))
  {
   AdjustCoord4();
   if(CheckCoord4()) return 12;
  }

 lLowerpart=gpuData[5]>>16;
 UpdateGlobalTP((unsigned short)lLowerpart);

 offsetPSX4();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 if(SHADETEXBIT(gpuData[0]))
  {
   gpuData[0]=(gpuData[0]&0xff000000)|0x00808080;
   gpuData[3]=(gpuData[3]&0xff000000)|0x00808080;
   gpuData[6]=(gpuData[6]&0xff000000)|0x00808080;
   gpuData[9]=(gpuData[9]&0xff000000)|0x00808080;
  }

 drawPoly4GT(baseAddr);

 bDoVSyncUpdate=TRUE;

 return 12;
}

////////////////////////////////////////////////////////////////////////
// cmd: smooth shaded Poly3
////////////////////////////////////////////////////////////////////////

unsigned short primPolyF3(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[4];
 ly1 = sgpuData[5];
 lx2 = sgpuData[6];
 ly2 = sgpuData[7];

 if(!(dwActFixes&8))
  {
   AdjustCoord3();
   if(CheckCoord3()) return 4;
  }

 offsetPSX3();
 SetRenderMode(gpuData[0]);

 drawPoly3F(gpuData[0]);

 bDoVSyncUpdate=TRUE;

 return 4;
}

////////////////////////////////////////////////////////////////////////
// cmd: skipping shaded polylines
////////////////////////////////////////////////////////////////////////

// note: on polylines we check for 0x50000000 and same lo/hiword...
// that will be more forgiving to gte bugs seen with some emus

unsigned short primLineGSkip(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 unsigned char dmaCount=3;int iMax=255;
 int i=2;

 ly1 = (short)((gpuData[1]>>16) & 0xffff);
 lx1 = (short)(gpuData[1] & 0xffff);

 while(!((gpuData[i]&0x50000000)==0x50000000 && 
         LOWORD(gpuData[i])==HIWORD(gpuData[i]) && i>4))
  {
   dmaCount++;i++;

   ly1 = (short)((gpuData[i]>>16) & 0xffff);
   lx1 = (short)(gpuData[i] & 0xffff);

   dmaCount++;i++;if(i>iMax) break;
  }
 return dmaCount;
}

////////////////////////////////////////////////////////////////////////
// cmd: shaded polylines
////////////////////////////////////////////////////////////////////////

unsigned short primLineGEx(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 unsigned char dmaCount=3;int iMax=255;
 unsigned long lc0,lc1,lc2,lc3;
 short slx0,slx1,sly0,sly1;int i=2;BOOL bDraw=TRUE;

 sly1 = (short)((gpuData[1]>>16) & 0xffff);
 slx1 = (short)(gpuData[1] & 0xffff);

 if(!(dwActFixes&8)) 
  {
   slx1=(short)(((int)slx1<<20)>>20);
   sly1=(short)(((int)sly1<<20)>>20);
  }

 lc0 = gpuData[0];
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 while(!((gpuData[i]&0x50000000)==0x50000000 && 
         LOWORD(gpuData[i])==HIWORD(gpuData[i]) && i>4))
  {
   sly0 = sly1;slx0=slx1;
   lc1=lc2=lc0;
   lc0=lc3=gpuData[i];

   i++;dmaCount++;

   if((gpuData[i]&0x50000000)==0x50000000 &&
      LOWORD(gpuData[i])==HIWORD(gpuData[i]))break;

   sly1 = (short)((gpuData[i]>>16) & 0xffff);
   slx1 = (short)(gpuData[i] & 0xffff);

   if(!(dwActFixes&8))
    {
     slx1=(short)(((int)slx1<<20)>>20);
     sly1=(short)(((int)sly1<<20)>>20);
     if(CheckCoordL(slx0,sly0,slx1,sly1)) bDraw=FALSE;
    }

   if ((lx0 != lx1) || (ly0 != ly1))
    {
     ly0=sly0;
     lx0=slx0;
     ly1=sly1;
     lx1=slx1;
              
     offsetPSXLine();
     if(bDraw) drawPoly4G(lc1,lc0,lc2,lc3);
    }
   dmaCount++;i++;  

   if(i>iMax) break;
  }

 bDoVSyncUpdate=TRUE;

 return dmaCount;
}

////////////////////////////////////////////////////////////////////////
// cmd: shaded polyline2
////////////////////////////////////////////////////////////////////////

unsigned short primLineG2(unsigned char *baseAddr)
{    
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[6];
 ly1 = sgpuData[7];

 if(!(dwActFixes&8))
  {
   AdjustCoord2();
   if(CheckCoord2()) return 4;
  }

 if((lx0 == lx1) && (ly0 == ly1)) {lx1++;ly1++;}
    
 offsetPSXLine();
 DrawSemiTrans = (SEMITRANSBIT(gpuData[0])) ? TRUE : FALSE;

 drawPoly4G(gpuData[0],gpuData[2],gpuData[0],gpuData[2]);

 bDoVSyncUpdate=TRUE;

 return 4;
}

////////////////////////////////////////////////////////////////////////
// cmd: skipping flat polylines
////////////////////////////////////////////////////////////////////////

unsigned short primLineFSkip(unsigned char *baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 unsigned char dmaCount=3;
 int i=2,iMax=255;

 ly1 = (short)((gpuData[1]>>16) & 0xffff);
 lx1 = (short)(gpuData[1] & 0xffff);

 while(!((gpuData[i]&0x50000000)==0x50000000 && 
         LOWORD(gpuData[i])==HIWORD(gpuData[i]) && i>3))
  {
   ly1 = (short)((gpuData[i]>>16) & 0xffff);
   lx1 = (short)(gpuData[i] & 0xffff);
   dmaCount++;i++;if(i>iMax) break;
  }             
 return dmaCount;
}

////////////////////////////////////////////////////////////////////////
// cmd: drawing flat polylines
////////////////////////////////////////////////////////////////////////

unsigned short primLineFEx(unsigned char *baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 unsigned char dmaCount=3;int iMax;
 short slx0,slx1,sly0,sly1;int i=2;BOOL bDraw=TRUE;

 iMax=255;

 sly1 = (short)((gpuData[1]>>16) & 0xffff);
 slx1 = (short)(gpuData[1] & 0xffff);
 if(!(dwActFixes&8))
  {
   slx1=(short)(((int)slx1<<20)>>20);
   sly1=(short)(((int)sly1<<20)>>20);
  }

 SetRenderMode(gpuData[0]);

 while(!((gpuData[i]&0x50000000)==0x50000000 && 
         LOWORD(gpuData[i])==HIWORD(gpuData[i]) && i>3))
  {
   sly0 = sly1;slx0=slx1;
   sly1 = (short)((gpuData[i]>>16) & 0xffff);
   slx1 = (short)(gpuData[i] & 0xffff);
   if(!(dwActFixes&8))
    {
     slx1=(short)(((int)slx1<<20)>>20);
     sly1=(short)(((int)sly1<<20)>>20);

     if(CheckCoordL(slx0,sly0,slx1,sly1)) bDraw=FALSE;
    }

   ly0=sly0;
   lx0=slx0;
   ly1=sly1;
   lx1=slx1;

   offsetPSXLine();
   if(bDraw) drawPoly4F(gpuData[0]);

   dmaCount++;i++;if(i>iMax) break;
  }

 bDoVSyncUpdate=TRUE;

 return dmaCount;
}

////////////////////////////////////////////////////////////////////////
// cmd: drawing flat polyline2
////////////////////////////////////////////////////////////////////////

unsigned short primLineF2(unsigned char *baseAddr)
{
 unsigned long *gpuData = ((unsigned long *) baseAddr);
 short *sgpuData = ((short *) baseAddr);

 lx0 = sgpuData[2];
 ly0 = sgpuData[3];
 lx1 = sgpuData[4];
 ly1 = sgpuData[5];

 if(!(dwActFixes&8))
  {
   AdjustCoord2();
   if(CheckCoord2()) return 3;
  }

 if((lx0 == lx1) && (ly0 == ly1)) {lx1++;ly1++;}
                    
 offsetPSXLine();

 SetRenderMode(gpuData[0]);

 drawPoly4F(gpuData[0]);

 bDoVSyncUpdate=TRUE;

 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd: well, easiest command... not implemented
////////////////////////////////////////////////////////////////////////

unsigned short primNI(unsigned char *bA)
{
 return 1;
}

////////////////////////////////////////////////////////////////////////
// cmd: and the skipping funcs
////////////////////////////////////////////////////////////////////////

unsigned short primTileSSkip(unsigned char * baseAddr)
{
 return 3;
}

unsigned short primTile1Skip(unsigned char * baseAddr)
{
 return 2;
}

unsigned short primTile8Skip(unsigned char * baseAddr)
{
 return 2;
}

unsigned short primTile16Skip(unsigned char * baseAddr)
{
 return 2;
}

unsigned short primSprt8Skip(unsigned char * baseAddr)
{
 return 3;
}

unsigned short primSprt16Skip(unsigned char * baseAddr)
{
 return 3;
}

unsigned short primSprtSSkip(unsigned char * baseAddr)
{
 return 4;
}

unsigned short primPolyF4Skip(unsigned char *baseAddr)
{
 return 5;
}

unsigned short primPolyG4Skip(unsigned char * baseAddr)
{
 return 8;
}

unsigned short primPolyFT3Skip(unsigned char * baseAddr)
{
 return 7;
}

unsigned short primPolyFT4Skip(unsigned char * baseAddr)
{
 return 9;
}

unsigned short primPolyGT3Skip(unsigned char *baseAddr)
{
 return 9;
}

unsigned short primPolyG3Skip(unsigned char *baseAddr)
{
 return 6;
}

unsigned short primPolyGT4Skip(unsigned char *baseAddr)
{
 return 12;
}

unsigned short primPolyF3Skip(unsigned char *baseAddr)
{
 return 4;
}

unsigned short primLineG2Skip(unsigned char *baseAddr)
{
 return 4;
}

unsigned short primLineF2Skip(unsigned char *baseAddr)
{
 return 3;
}

////////////////////////////////////////////////////////////////////////
// cmd func ptr table
////////////////////////////////////////////////////////////////////////


unsigned short (*primTableJ[256])(unsigned char *) = 
{
    // 00
    primNI,primNI,primBlkFill,primNI,primNI,primNI,primNI,primNI,
    // 08
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 10
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 18
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 20
    primPolyF3,primPolyF3,primPolyF3,primPolyF3,primPolyFT3,primPolyFT3,primPolyFT3,primPolyFT3,
    // 28
    primPolyF4,primPolyF4,primPolyF4,primPolyF4,primPolyFT4,primPolyFT4,primPolyFT4,primPolyFT4,
    // 30
    primPolyG3,primPolyG3,primPolyG3,primPolyG3,primPolyGT3,primPolyGT3,primPolyGT3,primPolyGT3,
    // 38
    primPolyG4,primPolyG4,primPolyG4,primPolyG4,primPolyGT4,primPolyGT4,primPolyGT4,primPolyGT4,
    // 40
    primLineF2,primLineF2,primLineF2,primLineF2,primNI,primNI,primNI,primNI,
    // 48
    primLineFEx,primLineFEx,primLineFEx,primLineFEx,primLineFEx,primLineFEx,primLineFEx,primLineFEx,
    // 50
    primLineG2,primLineG2,primLineG2,primLineG2,primNI,primNI,primNI,primNI,
    // 58
    primLineGEx,primLineGEx,primLineGEx,primLineGEx,primLineGEx,primLineGEx,primLineGEx,primLineGEx,
    // 60
    primTileS,primTileS,primTileS,primTileS,primSprtS,primSprtS,primSprtS,primSprtS,
    // 68
    primTile1,primTile1,primTile1,primTile1,primNI,primNI,primNI,primNI,
    // 70
    primTile8,primTile8,primTile8,primTile8,primSprt8,primSprt8,primSprt8,primSprt8,
    // 78
    primTile16,primTile16,primTile16,primTile16,primSprt16,primSprt16,primSprt16,primSprt16,
    // 80
    primMoveImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 88
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 90
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 98
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // a0
    primLoadImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // a8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // b0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // b8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // c0
    primStoreImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // c8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // d0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // d8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // e0
    primNI,cmdTexturePage,cmdTextureWindow,cmdDrawAreaStart,cmdDrawAreaEnd,cmdDrawOffset,cmdSTP,primNI,
    // e8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // f0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // f8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI
};

////////////////////////////////////////////////////////////////////////
// cmd func ptr table for skipping
////////////////////////////////////////////////////////////////////////

unsigned short (*primTableSkip[256])(unsigned char *) = 
{
    // 00
    primNI,primNI,primBlkFill,primNI,primNI,primNI,primNI,primNI,
    // 08
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 10
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 18
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 20
    primPolyF3Skip,primPolyF3Skip,primPolyF3Skip,primPolyF3Skip,primPolyFT3Skip,primPolyFT3Skip,primPolyFT3Skip,primPolyFT3Skip,
    // 28
    primPolyF4Skip,primPolyF4Skip,primPolyF4Skip,primPolyF4Skip,primPolyFT4Skip,primPolyFT4Skip,primPolyFT4Skip,primPolyFT4Skip,
    // 30
    primPolyG3Skip,primPolyG3Skip,primPolyG3Skip,primPolyG3Skip,primPolyGT3Skip,primPolyGT3Skip,primPolyGT3Skip,primPolyGT3Skip,
    // 38
    primPolyG4Skip,primPolyG4Skip,primPolyG4Skip,primPolyG4Skip,primPolyGT4Skip,primPolyGT4Skip,primPolyGT4Skip,primPolyGT4Skip,
    // 40
    primLineF2Skip,primLineF2Skip,primLineF2Skip,primLineF2Skip,primNI,primNI,primNI,primNI,
    // 48
    primLineFSkip,primLineFSkip,primLineFSkip,primLineFSkip,primLineFSkip,primLineFSkip,primLineFSkip,primLineFSkip,
    // 50
    primLineG2Skip,primLineG2Skip,primLineG2Skip,primLineG2Skip,primNI,primNI,primNI,primNI,
    // 58
    primLineGSkip,primLineGSkip,primLineGSkip,primLineGSkip,primLineGSkip,primLineGSkip,primLineGSkip,primLineGSkip,
    // 60
    primTileSSkip,primTileSSkip,primTileSSkip,primTileSSkip,primSprtSSkip,primSprtSSkip,primSprtSSkip,primSprtSSkip,
    // 68
    primTile1Skip,primTile1Skip,primTile1Skip,primTile1Skip,primNI,primNI,primNI,primNI,
    // 70
    primTile8Skip,primTile8Skip,primTile8Skip,primTile8Skip,primSprt8Skip,primSprt8Skip,primSprt8Skip,primSprt8Skip,
    // 78
    primTile16Skip,primTile16Skip,primTile16Skip,primTile16Skip,primSprt16Skip,primSprt16Skip,primSprt16Skip,primSprt16Skip,
    // 80
    primMoveImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 88
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 90
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // 98
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // a0
    primLoadImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // a8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // b0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // b8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // c0
    primStoreImage,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // c8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // d0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // d8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // e0
    primNI,cmdTexturePage,cmdTextureWindow,cmdDrawAreaStart,cmdDrawAreaEnd,cmdDrawOffset,cmdSTP,primNI,
    // e8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // f0
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI,
    // f8
    primNI,primNI,primNI,primNI,primNI,primNI,primNI,primNI
};
