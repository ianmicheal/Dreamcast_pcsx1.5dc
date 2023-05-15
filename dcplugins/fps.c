/***************************************************************************
                          fps.c  -  description
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
// 2001/10/28 - Pete  
// - generic cleanup for the Peops release
//
//*************************************************************************// 

#include "stdafx.h"

#define _IN_FPS

#include "externals.h"
#include "fps.h"

////////////////////////////////////////////////////////////////////////
// FPS stuff
////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
LARGE_INTEGER CPUFrequency, PerformanceCounter;
#endif

float          fFrameRateHz=0;
DWORD          dwFrameRateTicks=16;
int            iFrameRate;
int            iFrameLimit;
int            UseFrameLimit=0;
int            UseFrameSkip=0;

////////////////////////////////////////////////////////////////////////
// FPS skipping / limit
////////////////////////////////////////////////////////////////////////
       
BOOL   bInitCap = TRUE;
float  fps_skip = 0;
float  fps_cur  = 0;

////////////////////////////////////////////////////////////////////////
// WIN VERSION
////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS

BOOL           IsPerformanceCounter = FALSE;

void FrameCap (void)                                   // frame limit func
{
 static DWORD curticks, lastticks, _ticks_since_last_update;
 static DWORD TicksToWait = 0;
 static LARGE_INTEGER  CurrentTime;
 static LARGE_INTEGER  LastTime;
 static BOOL SkipNextWait = FALSE;
 BOOL Waiting = TRUE;

//---------------------------------------------------------
 if(bInitCap)
  {
   bInitCap=FALSE;
   if (IsPerformanceCounter)
    QueryPerformanceCounter(&LastTime);
   lastticks = timeGetTime();
   return;
  }
//---------------------------------------------------------

 if (IsPerformanceCounter)
  {
   QueryPerformanceCounter(&CurrentTime);
   _ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

   //---------------------------------------------------------
   curticks = timeGetTime();
   if(_ticks_since_last_update>(CPUFrequency.LowPart>>1))   
    {
     if(curticks < lastticks)
          _ticks_since_last_update = dwFrameRateTicks+TicksToWait+1;
     else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
    }
   //---------------------------------------------------------

   if ((_ticks_since_last_update > TicksToWait) || 
       (CurrentTime.LowPart < LastTime.LowPart))
    {
     LastTime.HighPart = CurrentTime.HighPart;
     LastTime.LowPart  = CurrentTime.LowPart;

     lastticks=curticks;

     if((_ticks_since_last_update-TicksToWait) > dwFrameRateTicks)
          TicksToWait=0;
     else TicksToWait=dwFrameRateTicks-(_ticks_since_last_update-TicksToWait);
    }
   else
    {
     while (Waiting)
      {
       QueryPerformanceCounter(&CurrentTime);
       _ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

       //---------------------------------------------------------
       curticks = timeGetTime();
       if(_ticks_since_last_update>(CPUFrequency.LowPart>>1))   
        {
         if(curticks < lastticks)
              _ticks_since_last_update = TicksToWait+1;
         else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
        }
       //---------------------------------------------------------

       if ((_ticks_since_last_update > TicksToWait) || 
           (CurrentTime.LowPart < LastTime.LowPart))
        {
         Waiting = FALSE;

         lastticks=curticks;

         LastTime.HighPart = CurrentTime.HighPart;
         LastTime.LowPart = CurrentTime.LowPart;
         TicksToWait = dwFrameRateTicks;
        }
      }
    }
  }
 else
  {
   curticks = timeGetTime();
   _ticks_since_last_update = curticks - lastticks;

   if ((_ticks_since_last_update > TicksToWait) || 
       (curticks < lastticks))
    {
     lastticks = curticks;

     if((_ticks_since_last_update-TicksToWait) > dwFrameRateTicks)
          TicksToWait=0;
     else TicksToWait=dwFrameRateTicks-(_ticks_since_last_update-TicksToWait);
    }
   else
    {
     while (Waiting)
      {
       curticks = timeGetTime();
       _ticks_since_last_update = curticks - lastticks;
       if ((_ticks_since_last_update > TicksToWait) || 
           (curticks < lastticks))
        {
         Waiting = FALSE;
         lastticks = curticks;
         TicksToWait = dwFrameRateTicks;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////

void calcfps(void)                                     // fps calculations
{
 static DWORD curticks,_ticks_since_last_update,lastticks;
 static long   fps_cnt = 0;
 static DWORD  fps_tck = 1;
 static LARGE_INTEGER  CurrentTime;
 static LARGE_INTEGER  LastTime;
 static long   fpsskip_cnt = 0;
 static DWORD  fpsskip_tck = 1;

 if(IsPerformanceCounter)
  {
   QueryPerformanceCounter(&CurrentTime);
   _ticks_since_last_update=CurrentTime.LowPart-LastTime.LowPart;

   //--------------------------------------------------//
   curticks = timeGetTime();
   if(_ticks_since_last_update>(CPUFrequency.LowPart>>1))   
    _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
   lastticks=curticks;
   //--------------------------------------------------//

   if(UseFrameSkip && !UseFrameLimit && _ticks_since_last_update)                                    
    fps_skip=min(fps_skip,(((float)CPUFrequency.LowPart) / ((float)_ticks_since_last_update) +1.0f));

   LastTime.HighPart = CurrentTime.HighPart;
   LastTime.LowPart = CurrentTime.LowPart;
  }
 else
  {
   curticks = timeGetTime();
   _ticks_since_last_update=curticks-lastticks;

   if(UseFrameSkip && !UseFrameLimit && _ticks_since_last_update)
    fps_skip=min(fps_skip,((float)1000/(float)_ticks_since_last_update+1.0f));

   lastticks = curticks;
  }

 if(UseFrameSkip && UseFrameLimit)
  {
   fpsskip_tck += _ticks_since_last_update;

   if(++fpsskip_cnt==2)
    {
     if(IsPerformanceCounter)
      fps_skip = ((float)CPUFrequency.LowPart) / ((float)fpsskip_tck) *2.0f;
     else
      fps_skip = (float)2000/(float)fpsskip_tck;

     fps_skip +=6.0f;

     fpsskip_cnt = 0;
     fpsskip_tck = 1;
    }
  }

 fps_tck += _ticks_since_last_update;

 if(++fps_cnt==10)
  {
   if(IsPerformanceCounter)
    fps_cur = ((float)CPUFrequency.LowPart) / ((float)fps_tck) *10.0f;
   else
    fps_cur = (float)10000/(float)fps_tck;

   fps_cnt = 0;
   fps_tck = 1;

   if(UseFrameLimit && fps_cur>fFrameRateHz)           // optical adjust ;) avoids flickering fps display
    fps_cur=fFrameRateHz;
  }
}

////////////////////////////////////////////////////////////////////////
// PC FPS skipping / limit
////////////////////////////////////////////////////////////////////////

void PCFrameCap (void)
{
 static DWORD curticks, lastticks, _ticks_since_last_update;
 static DWORD TicksToWait = 0;
 static LARGE_INTEGER  CurrentTime;
 static LARGE_INTEGER  LastTime;
 BOOL Waiting = TRUE;

 while (Waiting)
  {
   if (IsPerformanceCounter)
    {
     QueryPerformanceCounter(&CurrentTime);
     _ticks_since_last_update = CurrentTime.LowPart - LastTime.LowPart;

     //------------------------------------------------//
     curticks = timeGetTime();
     if(_ticks_since_last_update>(CPUFrequency.LowPart>>1))   
      {
       if(curticks < lastticks)
            _ticks_since_last_update = TicksToWait+1;
       else _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
      }
     //------------------------------------------------//

     if ((_ticks_since_last_update > TicksToWait) || 
         (CurrentTime.LowPart < LastTime.LowPart))
      {
       Waiting = FALSE;
       lastticks=curticks;
       LastTime.HighPart = CurrentTime.HighPart;
       LastTime.LowPart = CurrentTime.LowPart;
       TicksToWait = (unsigned long)(CPUFrequency.LowPart / fFrameRateHz);
      }
    }
   else
    {
     curticks = timeGetTime();
     _ticks_since_last_update = curticks - lastticks;
     if ((_ticks_since_last_update > TicksToWait) || 
         (curticks < lastticks))
      {
       Waiting = FALSE;
       lastticks = curticks;
       TicksToWait = (1000 / (DWORD)fFrameRateHz);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////

void PCcalcfps(void)
{
 static DWORD curticks,_ticks_since_last_update,lastticks;
 static long  fps_cnt = 0;
 static float fps_acc = 0;
 static LARGE_INTEGER  CurrentTime;
 static LARGE_INTEGER  LastTime;
 float CurrentFPS=0;    
 
 if(IsPerformanceCounter)
  {
   QueryPerformanceCounter(&CurrentTime);
   _ticks_since_last_update=CurrentTime.LowPart-LastTime.LowPart;

   //--------------------------------------------------//
   curticks = timeGetTime();
   if(_ticks_since_last_update>(CPUFrequency.LowPart>>1))   
    _ticks_since_last_update = (CPUFrequency.LowPart * (curticks - lastticks))/1000;
   lastticks=curticks;
   //--------------------------------------------------//

   if(_ticks_since_last_update)
    {
     CurrentFPS = ((float)CPUFrequency.LowPart) / ((float)_ticks_since_last_update);
    }
   else CurrentFPS = 0;
   LastTime.HighPart = CurrentTime.HighPart;
   LastTime.LowPart = CurrentTime.LowPart;
  }
 else
  {
   curticks = timeGetTime();
   if(_ticks_since_last_update=curticks-lastticks)
        CurrentFPS=(float)1000/(float)_ticks_since_last_update;
   else CurrentFPS = 0;
   lastticks = curticks;
  }

 fps_acc += CurrentFPS;

 if(++fps_cnt==10)
  {
   fps_cur = fps_acc / 10;
   fps_acc = 0;
   fps_cnt = 0;
  }
 
 fps_skip=CurrentFPS+1.0f;
}

////////////////////////////////////////////////////////////////////////

void SetAutoFrameCap(void)
{
 if(iFrameLimit==1)
  {
   fFrameRateHz = (float)iFrameRate;
   if(IsPerformanceCounter)
        dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
   else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
   return;
  }

 if(dwActFixes&32)
  {
   if (PSXDisplay.Interlaced)
        fFrameRateHz = PSXDisplay.PAL?50.0f:60.0f;
   else fFrameRateHz = PSXDisplay.PAL?25.0f:30.0f;
  }
 else
  {
   fFrameRateHz = PSXDisplay.PAL?50.0f:59.94f;
   if(IsPerformanceCounter)
        dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
   else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
  }
}

////////////////////////////////////////////////////////////////////////

void SetFPSHandler(void)
{
 if (QueryPerformanceFrequency (&CPUFrequency))        // timer mode
      IsPerformanceCounter = TRUE;
 else IsPerformanceCounter = FALSE;
}

////////////////////////////////////////////////////////////////////////

void InitFPS(void)
{
 if(fFrameRateHz==0) fFrameRateHz=(float)iFrameRate;   // set user framerate

 if(IsPerformanceCounter)
      dwFrameRateTicks=(DWORD)(CPUFrequency.LowPart / fFrameRateHz);
 else dwFrameRateTicks=(1000 / (DWORD)fFrameRateHz);
}

////////////////////////////////////////////////////////////////////////

#else

////////////////////////////////////////////////////////////////////////
// LINUX VERSION
////////////////////////////////////////////////////////////////////////

#define TIMEBASE 100000

unsigned long timeGetTime()
{
 struct timeval tv;
 gettimeofday(&tv, 0);                                 // well, maybe there are better ways
 return tv.tv_sec * 100000 + tv.tv_usec/10;            // to do that, but at least it works
}

void FrameCap (void) 
{ 
 static unsigned long curticks, lastticks, _ticks_since_last_update; 
 static unsigned long TicksToWait = 0; 
 BOOL Waiting = TRUE; 
 
  { 
   curticks = timeGetTime(); 
   _ticks_since_last_update = curticks - lastticks; 

    if((_ticks_since_last_update > TicksToWait) ||          
       (curticks <lastticks))      
    {
     lastticks = curticks; 
 
     if((_ticks_since_last_update-TicksToWait) > dwFrameRateTicks) 
          TicksToWait=0; 
     else TicksToWait=dwFrameRateTicks-(_ticks_since_last_update-TicksToWait); 
    } 
   else 
    { 
     while (Waiting) 
      { 
       curticks = timeGetTime(); 
       _ticks_since_last_update = curticks - lastticks; 
       if ((_ticks_since_last_update > TicksToWait) ||  
           (curticks < lastticks)) 
        { 
         Waiting = FALSE; 
         lastticks = curticks; 
         TicksToWait = dwFrameRateTicks; 
        } 
      } 
    } 
  } 
} 
 
//////////////////////////////////////////////////////////////////////// 
 
void calcfps(void) 
{ 
 static unsigned long curticks,_ticks_since_last_update,lastticks; 
 static long   fps_cnt = 0; 
 static unsigned long  fps_tck = 1; 
 static long          fpsskip_cnt = 0;
 static unsigned long fpsskip_tck = 1;
 
  { 
   curticks = timeGetTime(); 
   _ticks_since_last_update=curticks-lastticks; 
 
   if(UseFrameSkip && !UseFrameLimit && _ticks_since_last_update) 
    fps_skip=min(fps_skip,((float)TIMEBASE/(float)_ticks_since_last_update+1.0f)); 
 
   lastticks = curticks; 
  } 
 
 if(UseFrameSkip && UseFrameLimit)
  {
   fpsskip_tck += _ticks_since_last_update;

   if(++fpsskip_cnt==2)
    {
     fps_skip = (float)2000/(float)fpsskip_tck;
     fps_skip +=6.0f;
     fpsskip_cnt = 0;
     fpsskip_tck = 1;
    }
  }

 fps_tck += _ticks_since_last_update; 
 
 if(++fps_cnt==10) 
  { 
   fps_cur = (float)(TIMEBASE*10)/(float)fps_tck; 
 
   fps_cnt = 0; 
   fps_tck = 1; 
 
   if(UseFrameLimit && fps_cur>fFrameRateHz)           // optical adjust ;) avoids flickering fps display 
    fps_cur=fFrameRateHz; 
  } 

} 

void PCFrameCap (void)
{
 static unsigned long curticks, lastticks, _ticks_since_last_update;
 static unsigned long TicksToWait = 0;
 BOOL Waiting = TRUE;

 while (Waiting)
  {
   curticks = timeGetTime();
   _ticks_since_last_update = curticks - lastticks;
   if ((_ticks_since_last_update > TicksToWait) || 
       (curticks < lastticks))
    {
     Waiting = FALSE;
     lastticks = curticks;
     TicksToWait = (TIMEBASE/ (unsigned long)fFrameRateHz);
    }
  }
}

////////////////////////////////////////////////////////////////////////

void PCcalcfps(void)
{
 static unsigned long curticks,_ticks_since_last_update,lastticks;
 static long  fps_cnt = 0;
 static float fps_acc = 0;
 float CurrentFPS=0;    
 
 curticks = timeGetTime();
 _ticks_since_last_update=curticks-lastticks;
 if(_ticks_since_last_update)
      CurrentFPS=(float)TIMEBASE/(float)_ticks_since_last_update;
 else CurrentFPS = 0;
 lastticks = curticks;

 fps_acc += CurrentFPS;

 if(++fps_cnt==10)
  {
   fps_cur = fps_acc / 10;
   fps_acc = 0;
   fps_cnt = 0;
  }
 
 fps_skip=CurrentFPS+1.0f;
}

////////////////////////////////////////////////////////////////////////

void SetAutoFrameCap(void)
{
 if(iFrameLimit==1)
  {
   fFrameRateHz = iFrameRate;
   dwFrameRateTicks=(TIMEBASE / (unsigned long)fFrameRateHz);
   return;
  }

 if(dwActFixes&32)
  {
   if (PSXDisplay.Interlaced)
        fFrameRateHz = PSXDisplay.PAL?50.0f:60.0f;
   else fFrameRateHz = PSXDisplay.PAL?25.0f:30.0f;
  }
 else
  {
   fFrameRateHz = PSXDisplay.PAL?50.0f:59.94f;
   dwFrameRateTicks=(TIMEBASE / (unsigned long)fFrameRateHz); 
  }
}

////////////////////////////////////////////////////////////////////////

void SetFPSHandler(void)
{
}

////////////////////////////////////////////////////////////////////////

void InitFPS(void)
{
 if(!iFrameRate) iFrameRate=200;
 if(fFrameRateHz==0) fFrameRateHz=(float)iFrameRate;    // set user framerate
 dwFrameRateTicks=(TIMEBASE / (unsigned long)fFrameRateHz); 
}

#endif

