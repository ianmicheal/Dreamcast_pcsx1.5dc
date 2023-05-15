/***************************************************************************
                          cfg.c  -  description
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
// 2002/06/09 - linuzappz
// - fixed linux about dialog
//
// 2002/02/23 - Pete
// - added capcom fighter special game fix
//
// 2002/01/06 - lu
// - Connected the signal "destroy" to gtk_main_quit() in the ConfDlg, it
//   should fix a possible weird behaviour
//
// 2002/01/06 - lu
// - now fpse for linux has a configurator, some cosmetic changes done.
//
// 2001/12/25 - linuzappz
// - added gtk_main_quit(); in linux config
//
// 2001/12/20 - syo
// - added "Transparent menu" switch
//
// 2001/12/18 - syo
// - added "wait VSYNC" switch
// - support refresh rate change
// - modified key configuration (added toggle wait VSYNC key)
//   (Pete: fixed key buffers and added "- default"
//    refresh rate (=0) for cards not supporting setting the 
//    refresh rate) 
//
// 2001/12/18 - Darko Matesic
// - added recording configuration
//
// 2001/12/15 - lu
// - now fpsewp has his save and load routines in fpsewp.c
//
// 2001/12/05 - syo
// - added  "use system memory" switch
// - The bug which fails in the change in full-screen mode from window mode is corrected.
// - added  "Stop screen saver" switch
//
// 2001/11/20 - linuzappz
// - added WriteConfig and rewrite ReadConfigFile
// - added SoftDlgProc and AboutDlgProc for Linux (under gtk+-1.2.5)
//
// 2001/11/11 - lu
// - added some ifdef for FPSE layer
//
// 2001/11/09 - Darko Matesic
// - added recording configuration
//
// 2001/10/28 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#ifdef _WINDOWS

#define _IN_CFG

#include "stdafx.h"
#include "record.h"
#include <vfw.h>

#include "externals.h"
#include "cfg.h"
#include "gpu.h"

/////////////////////////////////////////////////////////////////////////////
// globals

char szKeyDefaults[10]={VK_DELETE,VK_INSERT,VK_HOME,VK_END,VK_PRIOR,VK_NEXT,VK_MULTIPLY,VK_SUBTRACT,VK_ADD,0x00};
char szDevName[128];

////////////////////////////////////////////////////////////////////////
// prototypes

BOOL OnInitCfgDialog(HWND hW);     
void OnCfgOK(HWND hW); 
BOOL OnInitSoftDialog(HWND hW);
void OnSoftOK(HWND hW); 
void OnCfgCancel(HWND hW); 
void OnCfgDef1(HWND hW);
void OnCfgDef2(HWND hW);
void OnBugFixes(HWND hW);

void OnRecording(HWND hW);

void SelectDev(HWND hW);
BOOL bTestModes(void);
void OnKeyConfig(HWND hW);
void GetSettings(HWND hW);
void OnClipboard(HWND hW);
void DoDevEnum(HWND hW);
char * pGetConfigInfos(int iCfg);

////////////////////////////////////////////////////////////////////////
// funcs

BOOL CALLBACK SoftDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_INITDIALOG:
     return OnInitSoftDialog(hW);

   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {
       case IDC_DISPMODE1: 
        {
         CheckDlgButton(hW,IDC_DISPMODE2,FALSE);
         return TRUE;
        }
       case IDC_DISPMODE2: 
        {
         CheckDlgButton(hW,IDC_DISPMODE1,FALSE);
         return TRUE;
        }
       case IDC_DEF1:      OnCfgDef1(hW);   return TRUE;
       case IDC_DEF2:      OnCfgDef2(hW);   return TRUE;
       case IDC_SELFIX:    OnBugFixes(hW);  return TRUE;
       case IDC_KEYCONFIG: OnKeyConfig(hW); return TRUE;
       case IDC_SELDEV:    SelectDev(hW);   return TRUE;
       case IDCANCEL:      OnCfgCancel(hW); return TRUE;
       case IDOK:          OnSoftOK(hW);    return TRUE;
       case IDC_CLIPBOARD: OnClipboard(hW); return TRUE;

       case IDC_RECORDING: OnRecording(hW);  return TRUE;
      }
    }
  }
 return FALSE;
}

////////////////////////////////////////////////////////////////////////
// init dlg
////////////////////////////////////////////////////////////////////////

void ComboBoxAddRes(HWND hWC,char * cs)
{
 int i=ComboBox_FindString(hWC,-1,cs);
 if(i!=CB_ERR) return;
 ComboBox_AddString(hWC,cs);
}

BOOL OnInitSoftDialog(HWND hW) 
{
 HWND hWC;char cs[256];int i;DEVMODE dv;

 ReadConfig();                                         // read registry stuff

 if(szDevName[0])
  SetDlgItemText(hW,IDC_DEVICETXT,szDevName);

 hWC=GetDlgItem(hW,IDC_RESOLUTION);

 memset(&dv,0,sizeof(DEVMODE));
 dv.dmSize=sizeof(DEVMODE);
 i=0;

 while(EnumDisplaySettings(NULL,i,&dv))
  {
   wsprintf(cs,"%4d x %4d - default",dv.dmPelsWidth,dv.dmPelsHeight);
   ComboBoxAddRes(hWC,cs);
   if(dv.dmDisplayFrequency > 40 && dv.dmDisplayFrequency < 200 )
    {
	 wsprintf(cs,"%4d x %4d , %4d Hz",dv.dmPelsWidth,dv.dmPelsHeight,dv.dmDisplayFrequency);
     ComboBoxAddRes(hWC,cs);
    }
   i++;
  }

 ComboBoxAddRes(hWC," 320 x  200 - default");
 ComboBoxAddRes(hWC," 320 x  240 - default");
 ComboBoxAddRes(hWC," 400 x  300 - default");
 ComboBoxAddRes(hWC," 512 x  384 - default");
 ComboBoxAddRes(hWC," 640 x  480 - default");
 ComboBoxAddRes(hWC," 800 x  600 - default");
 ComboBoxAddRes(hWC,"1024 x  768 - default");
 ComboBoxAddRes(hWC,"1152 x  864 - default");
 ComboBoxAddRes(hWC,"1280 x 1024 - default");
 ComboBoxAddRes(hWC,"1600 x 1200 - default");

 if(iRefreshRate)
      wsprintf(cs,"%4d x %4d , %4d Hz",iResX,iResY,iRefreshRate);
 else wsprintf(cs,"%4d x %4d - default",iResX,iResY);

 i=ComboBox_FindString(hWC,-1,cs);
 if(i==CB_ERR) i=0;
 ComboBox_SetCurSel(hWC,i);

 hWC=GetDlgItem(hW,IDC_COLDEPTH);
 ComboBox_AddString(hWC,"16 Bit");
 ComboBox_AddString(hWC,"32 Bit");
 wsprintf(cs,"%d Bit",iColDepth);                      // resolution
 i=ComboBox_FindString(hWC,-1,cs);
 if(i==CB_ERR) i=0;
 ComboBox_SetCurSel(hWC,i);

 hWC=GetDlgItem(hW,IDC_SCANLINES);
 ComboBox_AddString(hWC,"Scanlines disabled");
 ComboBox_AddString(hWC,"Scanlines enabled (standard)");
 ComboBox_AddString(hWC,"Scanlines enabled (double blitting - nVidia fix)");
 ComboBox_SetCurSel(hWC,iUseScanLines);

 SetDlgItemInt(hW,IDC_WINX,LOWORD(iWinSize),FALSE);    // window size
 SetDlgItemInt(hW,IDC_WINY,HIWORD(iWinSize),FALSE);

 if(UseFrameLimit)    CheckDlgButton(hW,IDC_USELIMIT,TRUE);
 if(UseFrameSkip)     CheckDlgButton(hW,IDC_USESKIPPING,TRUE);
 if(iWindowMode)      CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE2);
 else    	          CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
 if(iSysMemory)       CheckDlgButton(hW,IDC_SYSMEMORY,TRUE);
 if(iStopSaver)       CheckDlgButton(hW,IDC_STOPSAVER,TRUE);
 if(iUseFixes)        CheckDlgButton(hW,IDC_GAMEFIX,TRUE);
 if(iShowFPS)         CheckDlgButton(hW,IDC_SHOWFPS,TRUE);
 if(bVsync)           CheckDlgButton(hW,IDC_VSYNC,TRUE);
 if(bTransparent)	  CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);

 hWC=GetDlgItem(hW,IDC_NOSTRETCH);
 ComboBox_AddString(hWC,"Stretch to full window size");
 ComboBox_AddString(hWC,"1:1 (faster with some cards)");
 ComboBox_AddString(hWC,"Scale to window size, keep aspect ratio");
 ComboBox_SetCurSel(hWC,iUseNoStrechBlt);

 if(iFrameLimit==2)                                    // frame limit wrapper
      CheckDlgButton(hW,IDC_FRAMEAUTO,TRUE);
 else CheckDlgButton(hW,IDC_FRAMEMANUELL,TRUE);

 SetDlgItemInt(hW,IDC_FRAMELIM,iFrameRate,FALSE);      // set frame rate

 return TRUE;
}
                         
////////////////////////////////////////////////////////////////////////
// on ok: take vals
////////////////////////////////////////////////////////////////////////

void GetSettings(HWND hW) 
{
 HWND hWC;char cs[256];int i,j;char * p;

 hWC=GetDlgItem(hW,IDC_RESOLUTION);                    // get resolution
 i=ComboBox_GetCurSel(hWC);
 ComboBox_GetLBText(hWC,i,cs);
 iResX=atol(cs);
 p=strchr(cs,'x');
 iResY=atol(p+1);
 p=strchr(cs,',');									   // added by syo
 if(p) iRefreshRate=atol(p+1);						   // get refreshrate
 else  iRefreshRate=0;

 hWC=GetDlgItem(hW,IDC_COLDEPTH);                      // get color depth
 i=ComboBox_GetCurSel(hWC);
 ComboBox_GetLBText(hWC,i,cs);
 iColDepth=atol(cs);

 hWC=GetDlgItem(hW,IDC_SCANLINES);                     // scanlines
 iUseScanLines=ComboBox_GetCurSel(hWC);

 i=GetDlgItemInt(hW,IDC_WINX,NULL,FALSE);              // get win size
 if(i<50) i=50; if(i>20000) i=20000;
 j=GetDlgItemInt(hW,IDC_WINY,NULL,FALSE);
 if(j<50) j=50; if(j>20000) j=20000;
 iWinSize=MAKELONG(i,j);

 if(IsDlgButtonChecked(hW,IDC_DISPMODE2))              // win mode
  iWindowMode=1; else iWindowMode=0;

 if(IsDlgButtonChecked(hW,IDC_USELIMIT))               // fps limit
  UseFrameLimit=1; else UseFrameLimit=0;

 if(IsDlgButtonChecked(hW,IDC_USESKIPPING))            // fps skip
  UseFrameSkip=1; else UseFrameSkip=0;

 if(IsDlgButtonChecked(hW,IDC_GAMEFIX))                // game fix
  iUseFixes=1; else iUseFixes=0;

 if(IsDlgButtonChecked(hW,IDC_SYSMEMORY))              // use system memory
  iSysMemory=1; else iSysMemory=0;

 if(IsDlgButtonChecked(hW,IDC_STOPSAVER))              // stop screen saver
  iStopSaver=1; else iStopSaver=0;

 if(IsDlgButtonChecked(hW,IDC_VSYNC))                  // wait VSYNC
  bVsync=bVsync_Key=TRUE; else bVsync=bVsync_Key=FALSE;

 if(IsDlgButtonChecked(hW,IDC_TRANSPARENT))            // transparent menu
  bTransparent=TRUE; else bTransparent=FALSE;

 if(IsDlgButtonChecked(hW,IDC_SHOWFPS))                // show fps
  iShowFPS=1; else iShowFPS=0;

 hWC=GetDlgItem(hW,IDC_NOSTRETCH);
 iUseNoStrechBlt=ComboBox_GetCurSel(hWC);

 if(IsDlgButtonChecked(hW,IDC_FRAMEAUTO))              // frame rate
      iFrameLimit=2;
 else iFrameLimit=1;
 iFrameRate=GetDlgItemInt(hW,IDC_FRAMELIM,NULL,FALSE);
 if(iFrameRate<10)  iFrameRate=10;
 if(iFrameRate>200) iFrameRate=200;
}

void OnSoftOK(HWND hW)                                 
{
 GetSettings(hW);

 if(!iWindowMode && !bTestModes())                     // check fullscreen sets
  {
   MessageBox(hW,"Resolution/color depth not supported!","Error",MB_ICONERROR|MB_OK);
   return;
  }

 WriteConfig();                                        // write registry

 EndDialog(hW,TRUE);
}

////////////////////////////////////////////////////////////////////////
// on clipboard button
////////////////////////////////////////////////////////////////////////

void OnClipboard(HWND hW)
{
 HWND hWE=GetDlgItem(hW,IDC_CLPEDIT);
 char * pB;
 GetSettings(hW);
 pB=pGetConfigInfos(1);

 if(pB)
  {
   SetDlgItemText(hW,IDC_CLPEDIT,pB);
   SendMessage(hWE,EM_SETSEL,0,-1);
   SendMessage(hWE,WM_COPY,0,0);
   free(pB);
   MessageBox(hW,"Configuration info successfully copied to the clipboard\nJust use the PASTE function in another program to retrieve the data!","Copy Info",MB_ICONINFORMATION|MB_OK);
  }
}

////////////////////////////////////////////////////////////////////////
// Cancel
////////////////////////////////////////////////////////////////////////

void OnCfgCancel(HWND hW) 
{
 EndDialog(hW,FALSE);
}

////////////////////////////////////////////////////////////////////////
// Bug fixes
////////////////////////////////////////////////////////////////////////

BOOL CALLBACK BugFixesDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_INITDIALOG:
    {
     int i;
     
     for(i=0;i<32;i++)
      {
       if(dwCfgFixes&(1<<i))
        CheckDlgButton(hW,IDC_FIX1+i,TRUE);
      }
    }

   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {
       case IDCANCEL: EndDialog(hW,FALSE);return TRUE;

       case IDOK:     
        {
         int i;
         dwCfgFixes=0;
         for(i=0;i<32;i++)
          {
           if(IsDlgButtonChecked(hW,IDC_FIX1+i))
            dwCfgFixes|=(1<<i);
          }
         EndDialog(hW,TRUE);
         return TRUE;
        }
      }
    }
  }
 return FALSE;
}

void OnBugFixes(HWND hW)
{
 DialogBox(hInst,MAKEINTRESOURCE(IDD_FIXES),
           hW,(DLGPROC)BugFixesDlgProc);
}

////////////////////////////////////////////////////////////////////////
// Recording options
////////////////////////////////////////////////////////////////////////

void RefreshCodec(HWND hW)
{
char buffer[255];
union {
	char chFCC[5];
	DWORD dwFCC;
	} fcc;
ICINFO icinfo;
memset(&icinfo,0,sizeof(icinfo));
icinfo.dwSize = sizeof(icinfo);
strcpy(fcc.chFCC,"VIDC");
RECORD_COMPRESSION1.hic = ICOpen(fcc.dwFCC,RECORD_COMPRESSION1.fccHandler,ICMODE_QUERY);
if(RECORD_COMPRESSION1.hic)
	{
	ICGetInfo(RECORD_COMPRESSION1.hic,&icinfo,sizeof(icinfo));
	ICClose(RECORD_COMPRESSION1.hic);
	wsprintf(buffer,"16 bit Compression: %ws",icinfo.szDescription);
	}
else
	wsprintf(buffer,"16 bit Compression: Full Frames (Uncompressed)");
SetDlgItemText(hW,IDC_COMPRESSION1,buffer);

memset(&icinfo,0,sizeof(icinfo));
icinfo.dwSize = sizeof(icinfo);
RECORD_COMPRESSION2.hic = ICOpen(fcc.dwFCC,RECORD_COMPRESSION2.fccHandler,ICMODE_QUERY);
if(RECORD_COMPRESSION2.hic)
	{
	ICGetInfo(RECORD_COMPRESSION2.hic,&icinfo,sizeof(icinfo));
	ICClose(RECORD_COMPRESSION2.hic);
	wsprintf(buffer,"24 bit Compression: %ws",icinfo.szDescription);
	}
else
	wsprintf(buffer,"24 bit Compression: Full Frames (Uncompressed)");
SetDlgItemText(hW,IDC_COMPRESSION2,buffer);
}


BOOL CALLBACK RecordingDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_INITDIALOG:
    {
	 HWND hWC;
     CheckDlgButton(hW,IDC_REC_MODE1,RECORD_RECORDING_MODE==0);
     CheckDlgButton(hW,IDC_REC_MODE2,RECORD_RECORDING_MODE==1);
     hWC = GetDlgItem(hW,IDC_VIDEO_SIZE);
     ComboBox_ResetContent(hWC);
     ComboBox_AddString(hWC,"Full");
     ComboBox_AddString(hWC,"Half");
     ComboBox_AddString(hWC,"Quarter");
     ComboBox_SetCurSel(hWC,RECORD_VIDEO_SIZE);

     SetDlgItemInt(hW,IDC_REC_WIDTH,RECORD_RECORDING_WIDTH,FALSE);
     SetDlgItemInt(hW,IDC_REC_HEIGHT,RECORD_RECORDING_HEIGHT,FALSE);

     hWC = GetDlgItem(hW,IDC_FRAME_RATE);
     ComboBox_ResetContent(hWC);
     ComboBox_AddString(hWC,"1");
     ComboBox_AddString(hWC,"2");
     ComboBox_AddString(hWC,"3");
     ComboBox_AddString(hWC,"4");
     ComboBox_AddString(hWC,"5");
     ComboBox_AddString(hWC,"6");
     ComboBox_AddString(hWC,"7");
     ComboBox_AddString(hWC,"8");
     ComboBox_SetCurSel(hWC,RECORD_FRAME_RATE_SCALE);
     CheckDlgButton(hW,IDC_COMPRESSION1,RECORD_COMPRESSION_MODE==0);
     CheckDlgButton(hW,IDC_COMPRESSION2,RECORD_COMPRESSION_MODE==1);
     RefreshCodec(hW);
    }

   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {
       case IDC_RECCFG:
	    {
		if(IsDlgButtonChecked(hW,IDC_COMPRESSION1))
			{
			BITMAPINFOHEADER bitmap = {40,640,480,1,16,0,640*480*2,2048,2048,0,0};
			if(!ICCompressorChoose(hW,ICMF_CHOOSE_DATARATE|ICMF_CHOOSE_KEYFRAME,&bitmap,NULL,&RECORD_COMPRESSION1,"16 bit Compression")) return TRUE;
			if(RECORD_COMPRESSION1.cbState>sizeof(RECORD_COMPRESSION_STATE1))
				{
				memset(&RECORD_COMPRESSION1,0,sizeof(RECORD_COMPRESSION1));
				memset(&RECORD_COMPRESSION_STATE1,0,sizeof(RECORD_COMPRESSION_STATE1));
				RECORD_COMPRESSION1.cbSize	= sizeof(RECORD_COMPRESSION1);
				}
			else
				{
				if(RECORD_COMPRESSION1.lpState!=RECORD_COMPRESSION_STATE1)
					memcpy(RECORD_COMPRESSION_STATE1,RECORD_COMPRESSION1.lpState,RECORD_COMPRESSION1.cbState);
				}
			RECORD_COMPRESSION1.lpState = RECORD_COMPRESSION_STATE1;
			}
		else
			{
			BITMAPINFOHEADER bitmap = {40,640,480,1,24,0,640*480*3,2048,2048,0,0};
			if(!ICCompressorChoose(hW,ICMF_CHOOSE_DATARATE|ICMF_CHOOSE_KEYFRAME,&bitmap,NULL,&RECORD_COMPRESSION2,"24 bit Compression")) return TRUE;
			if(RECORD_COMPRESSION2.cbState>sizeof(RECORD_COMPRESSION_STATE2))
				{
				memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
				memset(&RECORD_COMPRESSION_STATE2,0,sizeof(RECORD_COMPRESSION_STATE2));
				RECORD_COMPRESSION2.cbSize	= sizeof(RECORD_COMPRESSION2);
				}
			else
				{
				if(RECORD_COMPRESSION2.lpState!=RECORD_COMPRESSION_STATE2)
					memcpy(RECORD_COMPRESSION_STATE2,RECORD_COMPRESSION2.lpState,RECORD_COMPRESSION2.cbState);
				}
			RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;
			}
		RefreshCodec(hW);
		return TRUE;
		}
       case IDCANCEL: EndDialog(hW,FALSE);return TRUE;

       case IDOK:     
        {
		HWND hWC;
		if(IsDlgButtonChecked(hW,IDC_REC_MODE1))	RECORD_RECORDING_MODE = 0;
		else										RECORD_RECORDING_MODE = 1;
		hWC = GetDlgItem(hW,IDC_VIDEO_SIZE);
		RECORD_VIDEO_SIZE = ComboBox_GetCurSel(hWC);
		RECORD_RECORDING_WIDTH = GetDlgItemInt(hW,IDC_REC_WIDTH,NULL,FALSE);
		RECORD_RECORDING_HEIGHT = GetDlgItemInt(hW,IDC_REC_HEIGHT,NULL,FALSE);
		hWC = GetDlgItem(hW,IDC_FRAME_RATE);
		RECORD_FRAME_RATE_SCALE = ComboBox_GetCurSel(hWC);
		if(IsDlgButtonChecked(hW,IDC_COMPRESSION1))	RECORD_COMPRESSION_MODE = 0;
		else										RECORD_COMPRESSION_MODE = 1;
        EndDialog(hW,TRUE);
        return TRUE;
        }
      }
    }
  }
 return FALSE;
}

void OnRecording(HWND hW)
{
 DialogBox(hInst,MAKEINTRESOURCE(IDD_RECORDING),
           hW,(DLGPROC)RecordingDlgProc);

}


////////////////////////////////////////////////////////////////////////
// default 1: fast
////////////////////////////////////////////////////////////////////////

void OnCfgDef1(HWND hW) 
{
 HWND hWC;
 
 hWC=GetDlgItem(hW,IDC_RESOLUTION);
 ComboBox_SetCurSel(hWC,1);
 hWC=GetDlgItem(hW,IDC_COLDEPTH);
 ComboBox_SetCurSel(hWC,0);
 hWC=GetDlgItem(hW,IDC_SCANLINES);
 ComboBox_SetCurSel(hWC,0);
 CheckDlgButton(hW,IDC_USELIMIT,FALSE);
 CheckDlgButton(hW,IDC_USESKIPPING,TRUE);
 CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
 CheckDlgButton(hW,IDC_FRAMEAUTO,FALSE);
 CheckDlgButton(hW,IDC_FRAMEMANUELL,TRUE);
 CheckDlgButton(hW,IDC_SHOWFPS,FALSE);
 hWC=GetDlgItem(hW,IDC_NOSTRETCH);
 ComboBox_SetCurSel(hWC,1);
 SetDlgItemInt(hW,IDC_FRAMELIM,200,FALSE);
 SetDlgItemInt(hW,IDC_WINX,320,FALSE);
 SetDlgItemInt(hW,IDC_WINY,240,FALSE);
 CheckDlgButton(hW,IDC_VSYNC,FALSE);
 CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);
}                

////////////////////////////////////////////////////////////////////////
// default 2: nice
////////////////////////////////////////////////////////////////////////
                
void OnCfgDef2(HWND hW) 
{
 HWND hWC;
 
 hWC=GetDlgItem(hW,IDC_RESOLUTION);
 ComboBox_SetCurSel(hWC,2);
 hWC=GetDlgItem(hW,IDC_COLDEPTH);
 ComboBox_SetCurSel(hWC,0);
 hWC=GetDlgItem(hW,IDC_SCANLINES);
 ComboBox_SetCurSel(hWC,0);
 CheckDlgButton(hW,IDC_USELIMIT,TRUE);
 CheckDlgButton(hW,IDC_USESKIPPING,FALSE);
 CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
 CheckDlgButton(hW,IDC_FRAMEAUTO,TRUE);
 CheckDlgButton(hW,IDC_FRAMEMANUELL,FALSE);
 CheckDlgButton(hW,IDC_SHOWFPS,FALSE);
 CheckDlgButton(hW,IDC_VSYNC,FALSE);
 CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);
 hWC=GetDlgItem(hW,IDC_NOSTRETCH);
 ComboBox_SetCurSel(hWC,0);

 SetDlgItemInt(hW,IDC_FRAMELIM,200,FALSE);
 SetDlgItemInt(hW,IDC_WINX,640,FALSE);
 SetDlgItemInt(hW,IDC_WINY,480,FALSE);
}
                
////////////////////////////////////////////////////////////////////////
// read registry
////////////////////////////////////////////////////////////////////////

void ReadConfig(void)
{
 HKEY myKey;
 DWORD temp;
 DWORD type;               
 DWORD size;

 // predefines
 iResX=640;iResY=480;
 iColDepth=16;
 iWindowMode=0;
 UseFrameLimit=0;
 UseFrameSkip=0;
 iFrameLimit=2;
 iFrameRate=200;
 iWinSize=MAKELONG(320,240);
 dwCfgFixes=0;
 iUseFixes=0;
 iUseGammaVal=2048;
 iUseScanLines=0;
 iUseNoStrechBlt=0;
 iShowFPS=0;
 iSysMemory=0;
 iStopSaver=0;
 bVsync=FALSE;
 bTransparent=FALSE;
 iRefreshRate=0;
 lstrcpy(szGPUKeys,szKeyDefaults);

 memset(szDevName,0,128);
 memset(&guiDev,0,sizeof(GUID));

 if (RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Vision Thing\\PSEmu Pro\\GPU\\PeteSoft",0,KEY_ALL_ACCESS,&myKey)==ERROR_SUCCESS)
  {
   size = 4;
   if(RegQueryValueEx(myKey,"ResX",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iResX=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"ResY",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iResY=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"RefreshRate",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iRefreshRate=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"WinSize",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iWinSize=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"WindowMode",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iWindowMode=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"ColDepth",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iColDepth=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseFrameLimit",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    UseFrameLimit=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseFrameSkip",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    UseFrameSkip=(int)temp;
   size = 4;                     
   if(RegQueryValueEx(myKey,"FrameLimit",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iFrameLimit=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"CfgFixes",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    dwCfgFixes=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseFixes",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iUseFixes=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseScanLines",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iUseScanLines=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"ShowFPS",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iShowFPS=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseNoStrechBlt",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iUseNoStrechBlt=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseGamma",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iUseGammaVal=(int)temp;
   if(!iFrameLimit) {UseFrameLimit=0;UseFrameSkip=0;iFrameLimit=2;}
   size = 4;
   if(RegQueryValueEx(myKey,"FrameRate",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iFrameRate=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"UseSysMemory",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iSysMemory=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"StopSaver",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iStopSaver=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"WaitVSYNC",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    bVsync=bVsync_Key=(BOOL)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"Transparent",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    bTransparent=(BOOL)temp;
   size=10;
   RegQueryValueEx(myKey,"GPUKeys",0,&type,(LPBYTE)&szGPUKeys,&size);
   size=128;
   RegQueryValueEx(myKey,"DeviceName",0,&type,(LPBYTE)szDevName,&size);
   size=sizeof(GUID);
   RegQueryValueEx(myKey,"GuiDev",0,&type,(LPBYTE)&guiDev,&size);

//
// Recording options
//
#define GetDWORD(xa,xb) size=4;if(RegQueryValueEx(myKey,xa,0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS) xb=(unsigned long)temp;
#define GetBINARY(xa,xb) size=sizeof(xb);RegQueryValueEx(myKey,xa,0,&type,(LPBYTE)&xb,&size);
	
GetDWORD("RecordingMode",				RECORD_RECORDING_MODE);
GetDWORD("RecordingVideoSize",			RECORD_VIDEO_SIZE);
GetDWORD("RecordingWidth",				RECORD_RECORDING_WIDTH);
GetDWORD("RecordingHeight",				RECORD_RECORDING_HEIGHT);
GetDWORD("RecordingFrameRateScale",		RECORD_FRAME_RATE_SCALE);
GetDWORD("RecordingCompressionMode",	RECORD_COMPRESSION_MODE);
GetBINARY("RecordingCompression1",		RECORD_COMPRESSION1);
GetBINARY("RecordingCompressionState1",	RECORD_COMPRESSION_STATE1);
GetBINARY("RecordingCompression2",		RECORD_COMPRESSION2);
GetBINARY("RecordingCompressionState2",	RECORD_COMPRESSION_STATE2);

if(RECORD_RECORDING_WIDTH>1024) RECORD_RECORDING_WIDTH = 1024;
if(RECORD_RECORDING_HEIGHT>768) RECORD_RECORDING_HEIGHT = 768;
if(RECORD_VIDEO_SIZE>2) RECORD_VIDEO_SIZE = 2;
if(RECORD_FRAME_RATE_SCALE>7) RECORD_FRAME_RATE_SCALE = 7;
if(RECORD_COMPRESSION1.cbSize != sizeof(RECORD_COMPRESSION1))
	{
	memset(&RECORD_COMPRESSION1,0,sizeof(RECORD_COMPRESSION1));
	RECORD_COMPRESSION1.cbSize = sizeof(RECORD_COMPRESSION1);
	}
RECORD_COMPRESSION1.lpState = RECORD_COMPRESSION_STATE1;
if(RECORD_COMPRESSION2.cbSize != sizeof(RECORD_COMPRESSION2))
	{
	memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
	RECORD_COMPRESSION2.cbSize = sizeof(RECORD_COMPRESSION2);
	}
RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;
//
//
//

   RegCloseKey(myKey);
  }

 if(!iColDepth) iColDepth=32;
 if(iUseFixes) dwActFixes=dwCfgFixes;
 SetFixes();

 if(iUseGammaVal<0 || iUseGammaVal>1536) iUseGammaVal=2048;
}

////////////////////////////////////////////////////////////////////////

void ReadWinSizeConfig(void)
{
 HKEY myKey;
 DWORD temp;
 DWORD type;               
 DWORD size;

 iResX=640;iResY=480;
 iWinSize=MAKELONG(320,240);

 if (RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Vision Thing\\PSEmu Pro\\GPU\\PeteSoft",0,KEY_ALL_ACCESS,&myKey)==ERROR_SUCCESS)
  {
   size = 4;
   if(RegQueryValueEx(myKey,"ResX",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iResX=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"ResY",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iResY=(int)temp;
   size = 4;
   if(RegQueryValueEx(myKey,"WinSize",0,&type,(LPBYTE)&temp,&size)==ERROR_SUCCESS)
    iWinSize=(int)temp;

   RegCloseKey(myKey);
  }
}

////////////////////////////////////////////////////////////////////////
// write registry
////////////////////////////////////////////////////////////////////////

void WriteConfig(void)
{
 HKEY myKey;
 DWORD myDisp;
 DWORD temp;
  
 RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\Vision Thing\\PSEmu Pro\\GPU\\PeteSoft",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&myKey,&myDisp);
 temp=iResX;
 RegSetValueEx(myKey,"ResX",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iResY;
 RegSetValueEx(myKey,"ResY",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iRefreshRate;
 RegSetValueEx(myKey,"RefreshRate",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iWinSize;
 RegSetValueEx(myKey,"WinSize",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iWindowMode;
 RegSetValueEx(myKey,"WindowMode",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iColDepth;
 RegSetValueEx(myKey,"ColDepth",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=UseFrameLimit;
 RegSetValueEx(myKey,"UseFrameLimit",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=UseFrameSkip;
 RegSetValueEx(myKey,"UseFrameSkip",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=dwCfgFixes;
 RegSetValueEx(myKey,"CfgFixes",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iUseFixes;
 RegSetValueEx(myKey,"UseFixes",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iUseScanLines;
 RegSetValueEx(myKey,"UseScanLines",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iShowFPS;
 RegSetValueEx(myKey,"ShowFPS",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iUseNoStrechBlt;
 RegSetValueEx(myKey,"UseNoStrechBlt",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iFrameLimit;
 RegSetValueEx(myKey,"FrameLimit",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iUseGammaVal;
 RegSetValueEx(myKey,"UseGamma",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iFrameRate;
 RegSetValueEx(myKey,"FrameRate",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=bVsync;
 RegSetValueEx(myKey,"WaitVSYNC",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=bTransparent;
 RegSetValueEx(myKey,"Transparent",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iSysMemory;
 RegSetValueEx(myKey,"UseSysMemory",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 temp=iStopSaver;
 RegSetValueEx(myKey,"StopSaver",0,REG_DWORD,(LPBYTE) &temp,sizeof(temp));
 RegSetValueEx(myKey,"GPUKeys",0,REG_BINARY,(LPBYTE)szGPUKeys,10);
 RegSetValueEx(myKey,"DeviceName",0,REG_BINARY,(LPBYTE)szDevName,128);  
 RegSetValueEx(myKey,"GuiDev",0,REG_BINARY,(LPBYTE)&guiDev,sizeof(GUID));

//
// Recording options
//
if(RECORD_COMPRESSION1.cbState>sizeof(RECORD_COMPRESSION_STATE1) || RECORD_COMPRESSION1.lpState!=RECORD_COMPRESSION_STATE1)
	{
	memset(&RECORD_COMPRESSION1,0,sizeof(RECORD_COMPRESSION1));
	memset(&RECORD_COMPRESSION_STATE1,0,sizeof(RECORD_COMPRESSION_STATE1));
	RECORD_COMPRESSION1.cbSize	= sizeof(RECORD_COMPRESSION1);
	RECORD_COMPRESSION1.lpState = RECORD_COMPRESSION_STATE1;
	}
if(RECORD_COMPRESSION2.cbState>sizeof(RECORD_COMPRESSION_STATE2) || RECORD_COMPRESSION2.lpState!=RECORD_COMPRESSION_STATE2)
	{
	memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
	memset(&RECORD_COMPRESSION_STATE2,0,sizeof(RECORD_COMPRESSION_STATE2));
	RECORD_COMPRESSION2.cbSize	= sizeof(RECORD_COMPRESSION2);
	RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;
	}

#define SetDWORD(xa,xb) RegSetValueEx(myKey,xa,0,REG_DWORD,(LPBYTE)&xb,sizeof(xb));
#define SetBINARY(xa,xb) RegSetValueEx(myKey,xa,0,REG_BINARY,(LPBYTE)&xb,sizeof(xb));

SetDWORD("RecordingMode",				RECORD_RECORDING_MODE);
SetDWORD("RecordingVideoSize",			RECORD_VIDEO_SIZE);
SetDWORD("RecordingWidth",				RECORD_RECORDING_WIDTH);
SetDWORD("RecordingHeight",				RECORD_RECORDING_HEIGHT);
SetDWORD("RecordingFrameRateScale",		RECORD_FRAME_RATE_SCALE);
SetDWORD("RecordingCompressionMode",	RECORD_COMPRESSION_MODE);
SetBINARY("RecordingCompression1",		RECORD_COMPRESSION1);
SetBINARY("RecordingCompressionState1",	RECORD_COMPRESSION_STATE1);
SetBINARY("RecordingCompression2",		RECORD_COMPRESSION2);
SetBINARY("RecordingCompressionState2",	RECORD_COMPRESSION_STATE2);
//
//
//
 RegCloseKey(myKey);
}

////////////////////////////////////////////////////////////////////////

HWND gHWND;

static HRESULT WINAPI Enum3DDevicesCallback( GUID* pGUID, LPSTR strDesc,
                                LPSTR strName, LPD3DDEVICEDESC pHALDesc, 
                                LPD3DDEVICEDESC pHELDesc, LPVOID pvContext )
{
 BOOL IsHardware;

 // Check params
 if( NULL==pGUID || NULL==pHALDesc || NULL==pHELDesc)
  return D3DENUMRET_CANCEL;

 // Handle specific device GUIDs. NullDevice renders nothing
 if( IsEqualGUID( pGUID, &IID_IDirect3DNullDevice ) )
  return D3DENUMRET_OK;

 IsHardware = ( 0 != pHALDesc->dwFlags );
 if(!IsHardware) return D3DENUMRET_OK;
 
 bDeviceOK=TRUE;

 return D3DENUMRET_OK;
}

static BOOL WINAPI DirectDrawEnumCallbackEx( GUID FAR* pGUID, LPSTR strDesc,
                                             LPSTR strName, VOID* pV,
                                             HMONITOR hMonitor )
{
 // Use the GUID to create the DirectDraw object, so that information
 // can be extracted from it.

 LPDIRECTDRAW pDD;
 LPDIRECTDRAW4 g_pDD;
 LPDIRECT3D3 pD3D;

 if( FAILED( DirectDrawCreate( pGUID, &pDD, 0L ) ) )
  {
   return D3DENUMRET_OK;
  }

 // Query the DirectDraw driver for access to Direct3D.
 if( FAILED(IDirectDraw_QueryInterface(pDD, &IID_IDirectDraw4, (VOID**)&g_pDD)))
  {
   IDirectDraw_Release(pDD);
   return D3DENUMRET_OK;
  }
 IDirectDraw_Release(pDD);

 // Query the DirectDraw driver for access to Direct3D.

 if( FAILED( IDirectDraw4_QueryInterface(g_pDD,&IID_IDirect3D3, (VOID**)&pD3D)))
  {
   IDirectDraw4_Release(g_pDD);
   return D3DENUMRET_OK;
  }

 bDeviceOK=FALSE;

 // Now, enumerate all the 3D devices
 IDirect3D3_EnumDevices(pD3D,Enum3DDevicesCallback,NULL);

 if(bDeviceOK)
  {
   HWND hWC=GetDlgItem(gHWND,IDC_DEVICE);
   int i=ComboBox_AddString(hWC,strDesc);
   GUID * g=(GUID *)malloc(sizeof(GUID));
   if(NULL != pGUID) *g=*pGUID;
   else              memset(g,0,sizeof(GUID));
   ComboBox_SetItemData(hWC,i,g);
  }

 IDirect3D3_Release(pD3D);
 IDirectDraw4_Release(g_pDD);
 return DDENUMRET_OK;
}

//-----------------------------------------------------------------------------

static BOOL WINAPI DirectDrawEnumCallback( GUID FAR* pGUID, LPSTR strDesc,
                                           LPSTR strName, VOID* pV)
{
 return DirectDrawEnumCallbackEx( pGUID, strDesc, strName, NULL, NULL );
}

//-----------------------------------------------------------------------------

void DoDevEnum(HWND hW)
{
 LPDIRECTDRAWENUMERATEEX pDDrawEnumFn;

 HMODULE hDDrawDLL = GetModuleHandle("DDRAW.DLL");
 if(NULL == hDDrawDLL) return;

 gHWND=hW;   

 pDDrawEnumFn = (LPDIRECTDRAWENUMERATEEX)
   GetProcAddress( hDDrawDLL, "DirectDrawEnumerateExA" );

 if(pDDrawEnumFn)
  pDDrawEnumFn( DirectDrawEnumCallbackEx, NULL,
               DDENUM_ATTACHEDSECONDARYDEVICES |
               DDENUM_DETACHEDSECONDARYDEVICES |
               DDENUM_NONDISPLAYDEVICES );
 else
  DirectDrawEnumerate( DirectDrawEnumCallback, NULL );
}

////////////////////////////////////////////////////////////////////////

void FreeGui(HWND hW)
{
 int i,iCnt;
 HWND hWC=GetDlgItem(hW,IDC_DEVICE);
 iCnt=ComboBox_GetCount(hWC);
 for(i=0;i<iCnt;i++)
  {
   free((GUID *)ComboBox_GetItemData(hWC,i));
  }
}

////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DeviceDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_INITDIALOG:
    {
     HWND hWC;int i;
     DoDevEnum(hW);
     hWC=GetDlgItem(hW,IDC_DEVICE);
     i=ComboBox_FindStringExact(hWC,-1,szDevName);
     if(i==CB_ERR) i=0;
     ComboBox_SetCurSel(hWC,i);
     hWC=GetDlgItem(hW,IDC_GAMMA);
     ScrollBar_SetRange(hWC,0,1024,FALSE);
     if(iUseGammaVal==2048) ScrollBar_SetPos(hWC,512,FALSE);
     else
      {
       ScrollBar_SetPos(hWC,iUseGammaVal,FALSE);
       CheckDlgButton(hW,IDC_USEGAMMA,TRUE);
      }
    }

   case WM_HSCROLL:
    {
     HWND hWC=GetDlgItem(hW,IDC_GAMMA);
     int pos=ScrollBar_GetPos(hWC);
     switch(LOWORD(wParam))
      {
       case SB_THUMBPOSITION:    
        pos=HIWORD(wParam);break; 
       case SB_LEFT:
        pos=0;break;
       case SB_RIGHT:  
        pos=1024;break;
       case SB_LINELEFT:
        pos-=16;break;
       case SB_LINERIGHT:
        pos+=16;break;
       case SB_PAGELEFT:  
        pos-=128;break;
       case SB_PAGERIGHT: 
        pos+=128;break;

      }
     ScrollBar_SetPos(hWC,pos,TRUE);
     return TRUE;
    }

   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {
       case IDCANCEL: FreeGui(hW);
                      EndDialog(hW,FALSE);return TRUE;
       case IDOK:     
        {
         HWND hWC=GetDlgItem(hW,IDC_DEVICE);
         int i=ComboBox_GetCurSel(hWC);
         if(i==CB_ERR) return TRUE;
         guiDev=*((GUID *)ComboBox_GetItemData(hWC,i));
         ComboBox_GetLBText(hWC,i,szDevName);
         FreeGui(hW);

         if(!IsDlgButtonChecked(hW,IDC_USEGAMMA))
          iUseGammaVal=2048;
         else
          iUseGammaVal=ScrollBar_GetPos(GetDlgItem(hW,IDC_GAMMA));

         EndDialog(hW,TRUE);
         return TRUE;
        }
      }
    }
  }
 return FALSE;
}                             

////////////////////////////////////////////////////////////////////////

void SelectDev(HWND hW)
{
 if(DialogBox(hInst,MAKEINTRESOURCE(IDD_DEVICE),
              hW,(DLGPROC)DeviceDlgProc)==IDOK)
  {
   SetDlgItemText(hW,IDC_DEVICETXT,szDevName);
  }
}


////////////////////////////////////////////////////////////////////////

static HRESULT WINAPI EnumDisplayModesCallback( DDSURFACEDESC2* pddsd,
                                                VOID* pvContext )
{
 if(NULL==pddsd) return DDENUMRET_CANCEL;
       
 if(pddsd->ddpfPixelFormat.dwRGBBitCount==(unsigned int)iColDepth &&
    pddsd->dwWidth==(unsigned int)iResX &&
    pddsd->dwHeight==(unsigned int)iResY)
  {
   bDeviceOK=TRUE;
   return DDENUMRET_CANCEL;
  }

 return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////////

BOOL bTestModes(void)
{
 LPDIRECTDRAW pDD;
 LPDIRECTDRAW4 g_pDD;

 GUID FAR * guid=0;
 int i;unsigned char * c=(unsigned char *)&guiDev;
 for(i=0;i<sizeof(GUID);i++,c++)
  {if(*c) {guid=&guiDev;break;}}

 bDeviceOK=FALSE;

 if( FAILED( DirectDrawCreate(guid, &pDD, 0L ) ) )
  return FALSE;

 if(FAILED(IDirectDraw_QueryInterface(pDD, &IID_IDirectDraw4, (VOID**)&g_pDD)))
  {
   IDirectDraw_Release(pDD);
   return FALSE;
  }
 IDirectDraw_Release(pDD);

 IDirectDraw4_EnumDisplayModes(g_pDD,0,NULL,NULL,EnumDisplayModesCallback);

 IDirectDraw4_Release(g_pDD);

 return bDeviceOK;
}

////////////////////////////////////////////////////////////////////////
// define key dialog
////////////////////////////////////////////////////////////////////////

typedef struct KEYSETSTAG
{
 char szName[10];
 char cCode;
}
KEYSETS;

KEYSETS tMKeys[]=
{
 {"SPACE",          0x20},
 {"PRIOR",          0x21},
 {"NEXT",           0x22},
 {"END",            0x23},
 {"HOME",           0x24},
 {"LEFT",           0x25},
 {"UP",             0x26},
 {"RIGHT",          0x27},
 {"DOWN",           0x28},
 {"SELECT",         0x29},
 {"PRINT",          0x2A},
 {"EXECUTE",        0x2B},
 {"SNAPSHOT",       0x2C},
 {"INSERT",         0x2D},
 {"DELETE",         0x2E},
 {"HELP",           0x2F},
 {"NUMPAD0",        0x60},
 {"NUMPAD1",        0x61},
 {"NUMPAD2",        0x62},
 {"NUMPAD3",        0x63},
 {"NUMPAD4",        0x64},
 {"NUMPAD5",        0x65},
 {"NUMPAD6",        0x66},
 {"NUMPAD7",        0x67},
 {"NUMPAD8",        0x68},
 {"NUMPAD9",        0x69},
 {"MULTIPLY",       0x6A},
 {"ADD",            0x6B},
 {"SEPARATOR",      0x6C},
 {"SUBTRACT",       0x6D},
 {"DECIMAL",        0x6E},
 {"DIVIDE",         0x6F},
 {"",               0x00}
};

void SetGPUKey(HWND hWC,char szKey)
{
 int i,iCnt=ComboBox_GetCount(hWC);
 for(i=0;i<iCnt;i++)
  {
   if(ComboBox_GetItemData(hWC,i)==szKey) break;
  }
 if(i!=iCnt) ComboBox_SetCurSel(hWC,i);
}                                 

BOOL CALLBACK KeyDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 switch(uMsg)
  {
   case WM_INITDIALOG:
    {
     int i,j,k;char szB[2];HWND hWC;
     for(i=IDC_KEY1;i<=IDC_KEY9;i++)
      {
       hWC=GetDlgItem(hW,i);

       for(j=0;tMKeys[j].cCode!=0;j++)
        {
         k=ComboBox_AddString(hWC,tMKeys[j].szName);
         ComboBox_SetItemData(hWC,k,tMKeys[j].cCode);
        }
       for(j=0x30;j<=0x39;j++)
        {
         wsprintf(szB,"%c",j);
         k=ComboBox_AddString(hWC,szB);
         ComboBox_SetItemData(hWC,k,j);
        }
       for(j=0x41;j<=0x5a;j++)
        {
         wsprintf(szB,"%c",j);
         k=ComboBox_AddString(hWC,szB);
         ComboBox_SetItemData(hWC,k,j);
        }                              
       SetGPUKey(GetDlgItem(hW,i),szGPUKeys[i-IDC_KEY1]);
      }
    }return TRUE;

   case WM_COMMAND:
    {
     switch(LOWORD(wParam))
      {
       case IDC_DEFAULT:                 
        {
         int i;
         for(i=IDC_KEY1;i<=IDC_KEY9;i++)
          SetGPUKey(GetDlgItem(hW,i),szKeyDefaults[i-IDC_KEY1]);
        }break;

       case IDCANCEL:     EndDialog(hW,FALSE); return TRUE;
       case IDOK:
        {
         HWND hWC;int i;
         for(i=IDC_KEY1;i<=IDC_KEY9;i++)
          {
           hWC=GetDlgItem(hW,i);
           szGPUKeys[i-IDC_KEY1]=(char)ComboBox_GetItemData(hWC,ComboBox_GetCurSel(hWC));
           if(szGPUKeys[i-IDC_KEY1]<0x20) szGPUKeys[i-IDC_KEY1]=0x20;
          }
         EndDialog(hW,TRUE);  
         return TRUE;
        }
      }
    }
  }
 return FALSE;
}

void OnKeyConfig(HWND hW) 
{
 DialogBox(hInst,MAKEINTRESOURCE(IDD_KEYS),
           hW,(DLGPROC)KeyDlgProc);
}

#else

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// LINUX VERSION
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define _IN_CFG

#include <gtk/gtk.h>
#undef FALSE
#undef TRUE

#include "externals.h"
#include "cfg.h"
#include "gpu.h"

char * pConfigFile=NULL;

////////////////////////////////////////////////////////////////////////
// gtk linux stuff
////////////////////////////////////////////////////////////////////////

GtkWidget *ConfDlg;
GtkWidget *FullBtn,*WinBtn;
GtkWidget *StretchBtn;
GtkWidget *ShowFpsBtn,*FpsLimitBtn,*FrameSkipBtn,*AutoFpsBtn;
GtkWidget *ScanLinesBtn,*FixesBtn;
GtkWidget *EditW,*EditH,*EditF;
GtkWidget *ComboModes;
GtkWidget *FixesDlg;
GtkWidget *FixesBtns[32];
GList *Modes;
int NModes;

void OnCfgFixes_Ok()
{
 int i;

 for (i=0; i<32; i++)
  if (FixesBtns[i] != NULL)
  {
   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(FixesBtns[i]))) dwCfgFixes |= 1<<i;
   else dwCfgFixes &= ~(1<<i);
  }
 gtk_widget_hide(FixesDlg);
}

void OnCfgFixes_Cancel()
{
 gtk_widget_hide(FixesDlg);
}

#define AddCfgFixBtn(name, desc) \
 BBox = gtk_hbox_new(0, 5); \
 gtk_box_pack_start(GTK_BOX(Box), BBox, FALSE, FALSE, 0); \
 \
 sprintf(Text, "0x%4.4x:", (1<<i)); \
 Label = gtk_label_new(Text); \
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0); \
 \
 FixesBtns[i] = gtk_check_button_new_with_label(name); \
 gtk_widget_set_usize(FixesBtns[i], 180, 25); \
 gtk_box_pack_start(GTK_BOX(BBox), FixesBtns[i], FALSE, FALSE, 0); \
 \
 Label = gtk_label_new(desc); \
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0); \
 \
 i++;

void OnCfgFixes()
{
 GtkWidget *DlgPacker;
 GtkWidget *Label;
 GtkWidget *Ok,*Cancel;
 GtkWidget *Box,*BBox;
 int i;
 char Text[32];

 FixesDlg = gtk_window_new(GTK_WINDOW_DIALOG);
// gtk_widget_set_usize(FixesDlg, 500, 400);
 gtk_window_set_title(GTK_WINDOW(FixesDlg), "Special game fixes...");
 gtk_window_set_position(GTK_WINDOW(FixesDlg), GTK_WIN_POS_CENTER);
 gtk_container_set_border_width(GTK_CONTAINER(FixesDlg), 5);

 DlgPacker = gtk_packer_new();
 gtk_container_add(GTK_CONTAINER(FixesDlg), DlgPacker);

 Box = gtk_vbox_new(0, 5);
 gtk_container_set_border_width(GTK_CONTAINER(Box), 5);
 gtk_packer_add(GTK_PACKER(DlgPacker), Box, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                GTK_FILL_X, 0, 8, 8, 0, 0);

 Label = gtk_label_new("Activate the following options only if you want to play one of the listed games\n(or if your game is showing similar glitches)!");
 gtk_box_pack_start(GTK_BOX(Box), Label, FALSE, FALSE, 0);

 for (i=0; i<32; i++) FixesBtns[i] = NULL;

 i = 0;
 AddCfgFixBtn("Unused"                   , "");
 AddCfgFixBtn("Expand screen width"      , "2D Capcom fighters");
 AddCfgFixBtn("Ignore brightness color"  , "Black screens in Lunar");
 AddCfgFixBtn("Disable coord check"      , "Old compatibility mode");
 AddCfgFixBtn("Unused"                   , "");
 AddCfgFixBtn("Use PC fps calculation"   , "Better fps limitation with some games");
 AddCfgFixBtn("Lazy screen updates"      , "Maybe faster, needed for Pandemonium2");
 AddCfgFixBtn("Unused"                   , "");

 for (i=0; i<32; i++)
  if (FixesBtns[i] != NULL)
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FixesBtns[i]), dwCfgFixes & (1<<i));

 BBox = gtk_hbutton_box_new();
 gtk_packer_add(GTK_PACKER(DlgPacker), BBox, GTK_SIDE_BOTTOM, GTK_ANCHOR_S,
                GTK_FILL_X | GTK_FILL_Y | GTK_EXPAND, 0, 8, 8, 0, 0);

 Ok = gtk_button_new_with_label("Ok");
 gtk_signal_connect(GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnCfgFixes_Ok), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Ok, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Ok, GTK_CAN_DEFAULT);

 Cancel = gtk_button_new_with_label("Cancel");
 gtk_signal_connect(GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnCfgFixes_Cancel), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Cancel, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Cancel, GTK_CAN_DEFAULT);

 gtk_widget_show_all(FixesDlg);
}

void OnDefFast()
{
 char Text[32];

 iResX = 320; iResY = 240;
 if (!iWindowMode)
 {
  sprintf (Text, "%dx%d", iResX, iResY);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ComboModes)->entry), Text);
 }
 sprintf(Text,"%d", iResX);
 gtk_entry_set_text(GTK_ENTRY(EditW), Text);
 sprintf(Text,"%d", iResY);
 gtk_entry_set_text(GTK_ENTRY(EditH), Text);
 sprintf(Text,"%d", 200);
 gtk_entry_set_text(GTK_ENTRY(EditF), Text);

 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FullBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WinBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(StretchBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ShowFpsBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FpsLimitBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FrameSkipBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(AutoFpsBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ScanLinesBtn), 0);
}

void OnDefNice()
{
 char Text[32];

 iResX = 640; iResY = 480;
 if (!iWindowMode)
 {
  sprintf (Text, "%dx%d", iResX, iResY);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ComboModes)->entry), Text);
 }
 sprintf(Text,"%d", iResX);
 gtk_entry_set_text(GTK_ENTRY(EditW), Text);
 sprintf(Text,"%d", iResY);
 gtk_entry_set_text(GTK_ENTRY(EditH), Text);
 sprintf(Text,"%d", 200);
 gtk_entry_set_text(GTK_ENTRY(EditF), Text);

 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FullBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WinBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(StretchBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ShowFpsBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FpsLimitBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FrameSkipBtn), 0);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(AutoFpsBtn), 1);
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ScanLinesBtn), 0);
}

void OnOk()
{
 char *tmp;

 iWindowMode     = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(FullBtn));
 iUseNoStrechBlt = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(StretchBtn));
 iShowFPS        = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ShowFpsBtn));
 UseFrameLimit   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(FpsLimitBtn));
 UseFrameSkip    = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(FrameSkipBtn));
 iFrameLimit     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(AutoFpsBtn))+1;
 iUseScanLines   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ScanLinesBtn));
 iUseFixes       = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(FixesBtn));

 tmp = gtk_entry_get_text(GTK_ENTRY(EditW));
 iResX = atoi(tmp);
 tmp = gtk_entry_get_text(GTK_ENTRY(EditH));
 iResY = atoi(tmp);
 if (!iWindowMode)
 {
  tmp = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(ComboModes)->entry));
  sscanf(tmp, "%dx%d", &iResX, &iResY);
 }
 tmp = gtk_entry_get_text(GTK_ENTRY(EditF));
 iFrameRate = atoi(tmp);
 gtk_widget_hide(ConfDlg);//really needed?
 gtk_object_destroy((gpointer)ConfDlg);
/* moved by lu_zero
 gtk_main_quit();
 */
 WriteConfig();
}

void OnCancel()
{
 gtk_widget_hide(ConfDlg);
 gtk_object_destroy((gpointer)ConfDlg);
/* moved by lu_zero
 gtk_main_quit();
 */
}

void SoftDlgProc(void)
{
 GtkWidget *DlgPacker;
 GtkWidget *Label;
 GtkWidget *Ok,*Cancel;
 GtkWidget *Box,*Box1,*Box2,*BBox;
 GtkWidget *Btn,*Frame;
 char Text[32];

 ReadConfig();

 ConfDlg = gtk_window_new(GTK_WINDOW_DIALOG);
 gtk_widget_set_usize(ConfDlg, 400, 375);
#ifndef _SDL
 gtk_window_set_title(GTK_WINDOW(ConfDlg), "Configure the P.E.Op.S. PSX X11 Soft Renderer...");
#else
 gtk_window_set_title(GTK_WINDOW(ConfDlg), "Configure the P.E.Op.S. PSX SDL Soft Renderer..."); 
#endif
 gtk_window_set_position(GTK_WINDOW(ConfDlg), GTK_WIN_POS_CENTER);
 gtk_container_set_border_width(GTK_CONTAINER(ConfDlg), 5);

 DlgPacker = gtk_packer_new();
 gtk_container_add(GTK_CONTAINER(ConfDlg), DlgPacker);

 Frame = gtk_frame_new("Resolution");
 gtk_packer_add(GTK_PACKER(DlgPacker), Frame, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                GTK_FILL_X, 0, 8, 8, 0, 0);

 Box = gtk_vbox_new(0, 2);
 gtk_container_set_border_width(GTK_CONTAINER(Box), 5);
 gtk_container_add(GTK_CONTAINER(Frame), Box);

 Box1 = gtk_hbox_new(0, 5);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 FullBtn = gtk_radio_button_new_with_label(NULL, "Fullscreen mode");
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(FullBtn), iWindowMode != 1);
 gtk_box_pack_start(GTK_BOX(Box1), FullBtn, FALSE, FALSE, 0);

 Modes = NULL;
 Modes = g_list_append(Modes, "320x240");
 Modes = g_list_append(Modes, "640x480");
 Modes = g_list_append(Modes, "800x600");
 Modes = g_list_append(Modes, "1024x768");

 NModes = 2;

 ComboModes = gtk_combo_new();
 gtk_widget_set_usize(ComboModes, 100, 20);
 gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(ComboModes)->entry), FALSE);
 gtk_combo_set_popdown_strings(GTK_COMBO(ComboModes), Modes);
 if (!iWindowMode)
 {
  sprintf (Text, "%dx%d", iResX, iResY);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ComboModes)->entry), Text);
 }
 gtk_box_pack_start(GTK_BOX(Box1), ComboModes, FALSE, FALSE, 0);

 Box1 = gtk_hbox_new(0, 0);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 WinBtn = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(FullBtn)), "Windowed mode");
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WinBtn), iWindowMode != 0);
 gtk_box_pack_start(GTK_BOX(Box1), WinBtn, FALSE, FALSE, 0);

 Label = gtk_label_new("Size:");
 gtk_box_pack_start(GTK_BOX(Box1), Label, FALSE, FALSE, 5);

 EditW = gtk_entry_new();
 sprintf(Text,"%d", iResX);
 gtk_entry_set_text(GTK_ENTRY(EditW), Text);
 gtk_widget_set_usize(EditW, 40, 22);
 gtk_box_pack_start(GTK_BOX(Box1), EditW, FALSE, FALSE, 5);

 Label = gtk_label_new("x");
 gtk_box_pack_start(GTK_BOX(Box1), Label, FALSE, FALSE, 5);

 EditH = gtk_entry_new();
 sprintf(Text,"%d", iResY);
 gtk_entry_set_text(GTK_ENTRY(EditH), Text);
 gtk_widget_set_usize(EditH, 40, 22);
 gtk_box_pack_start(GTK_BOX(Box1), EditH, FALSE, FALSE, 5);

 Box1 = gtk_vbox_new(0, 0);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 StretchBtn = gtk_check_button_new_with_label("No Stretching");
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(StretchBtn), iUseNoStrechBlt);
 gtk_box_pack_start(GTK_BOX(Box1), StretchBtn, FALSE, FALSE, 0);

 Frame = gtk_frame_new("Framerate");
 gtk_packer_add(GTK_PACKER(DlgPacker), Frame, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                GTK_FILL_X | GTK_FILL_Y | GTK_EXPAND, 0, 8, 8, 0, 0);

 Box = gtk_vbox_new(0, 2);
 gtk_container_set_border_width(GTK_CONTAINER(Box), 5);
 gtk_container_add(GTK_CONTAINER(Frame), Box);

 Box1 = gtk_vbox_new(0, 0);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 ShowFpsBtn = gtk_check_button_new_with_label("Show FPS display on startup");
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ShowFpsBtn), iShowFPS);
 gtk_box_pack_start(GTK_BOX(Box1), ShowFpsBtn, FALSE, FALSE, 0);

 Box1 = gtk_hbox_new(0, 5);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 Box2 = gtk_vbox_new(0, 2);
 gtk_box_pack_start(GTK_BOX(Box1), Box2, FALSE, FALSE, 0);

 FpsLimitBtn = gtk_check_button_new_with_label("Use FPS limit");
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(FpsLimitBtn), UseFrameLimit);
 gtk_box_pack_start(GTK_BOX(Box2), FpsLimitBtn, FALSE, FALSE, 0);
 
 FrameSkipBtn = gtk_check_button_new_with_label("Use Frame skipping");
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(FrameSkipBtn), UseFrameSkip);
 gtk_box_pack_start(GTK_BOX(Box2), FrameSkipBtn, FALSE, FALSE, 0);

 Box2 = gtk_vbox_new(0, 2);
 gtk_box_pack_start(GTK_BOX(Box1), Box2, FALSE, FALSE, 0);

 AutoFpsBtn = gtk_check_button_new_with_label("Auto-detect FPS/Frame skipping limit");
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(AutoFpsBtn), iFrameLimit-1);
 gtk_box_pack_start(GTK_BOX(Box2), AutoFpsBtn, FALSE, FALSE, 0);

 BBox = gtk_hbox_new(0, 0);
 gtk_box_pack_start(GTK_BOX(Box2), BBox, FALSE, FALSE, 0);

 Label = gtk_label_new("Framerate:");
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 5);

 EditF = gtk_entry_new();
 sprintf(Text,"%d", iFrameRate);
 gtk_entry_set_text(GTK_ENTRY(EditF), Text);
 gtk_box_pack_start(GTK_BOX(BBox), EditF, FALSE, FALSE, 5);
 gtk_widget_set_usize(EditF, 40, 22);

 Label = gtk_label_new("FPS");
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 5);

 Frame = gtk_frame_new("Options");
 gtk_packer_add(GTK_PACKER(DlgPacker), Frame, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                GTK_FILL_X, 0, 8, 8, 0, 0);

 Box = gtk_vbox_new(0, 2);
 gtk_container_set_border_width(GTK_CONTAINER(Box), 5);
 gtk_container_add(GTK_CONTAINER(Frame), Box);

 ScanLinesBtn = gtk_check_button_new_with_label("Scanlines enabled");
 gtk_box_pack_start(GTK_BOX(Box), ScanLinesBtn, FALSE, FALSE, 0);
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ScanLinesBtn), iUseScanLines);
 
 Box1 = gtk_hbox_new(0, 5);
 gtk_box_pack_start(GTK_BOX(Box), Box1, FALSE, FALSE, 0);

 FixesBtn = gtk_check_button_new_with_label("Activate special games fixes");
 gtk_box_pack_start(GTK_BOX(Box1), FixesBtn, FALSE, FALSE, 0);
 gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(FixesBtn), iUseFixes);

 Btn = gtk_button_new_with_label("...");
 gtk_signal_connect(GTK_OBJECT(Btn), "clicked", GTK_SIGNAL_FUNC(OnCfgFixes), NULL);
 gtk_box_pack_start(GTK_BOX(Box1), Btn, FALSE, FALSE, 0);

 Frame = gtk_frame_new("Default settings");
 gtk_packer_add(GTK_PACKER(DlgPacker), Frame, GTK_SIDE_LEFT, GTK_ANCHOR_W,
                GTK_FILL_Y | GTK_EXPAND, 0, 8, 8, 0, 0);

 BBox = gtk_hbutton_box_new();
 gtk_container_add(GTK_CONTAINER(Frame), BBox);

 Btn = gtk_button_new_with_label("Fast");
 gtk_signal_connect(GTK_OBJECT(Btn), "clicked", GTK_SIGNAL_FUNC(OnDefFast), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Btn, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Btn, GTK_CAN_DEFAULT);

 Btn = gtk_button_new_with_label("Nice");
 gtk_signal_connect(GTK_OBJECT(Btn), "clicked", GTK_SIGNAL_FUNC(OnDefNice), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Btn, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Btn, GTK_CAN_DEFAULT);

 BBox = gtk_hbutton_box_new();
 gtk_packer_add(GTK_PACKER(DlgPacker), BBox, GTK_SIDE_RIGHT, GTK_ANCHOR_S,
                GTK_FILL_Y | GTK_EXPAND, 0, 8, 8, 0, 0);

 Ok = gtk_button_new_with_label("Ok");
 gtk_signal_connect(GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnOk), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Ok, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Ok, GTK_CAN_DEFAULT);

 Cancel = gtk_button_new_with_label("Cancel");
 gtk_signal_connect(GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnCancel), NULL);
 gtk_box_pack_start(GTK_BOX(BBox), Cancel, FALSE, FALSE, 0);
 GTK_WIDGET_SET_FLAGS(Cancel, GTK_CAN_DEFAULT);

 gtk_widget_show_all(ConfDlg);

//added by lu 
 gtk_signal_connect (GTK_OBJECT (ConfDlg), "destroy",  
		      GTK_SIGNAL_FUNC(gtk_main_quit),
		      NULL);
 // added by pete:
 gtk_main ();
}
#ifndef _FPSE

GtkWidget *AboutDlg;

void OnAboutOk(GtkMenuItem *menuitem, gpointer userdata)
{
 gtk_widget_destroy(AboutDlg);
}

extern unsigned char revision;
extern unsigned char build;
#define RELEASE_DATE "06.09.2002"

void AboutDlgProc(void)
{
 GtkWidget *Label;
 GtkWidget *Ok;
 GtkWidget *Box,*BBox;
 char AboutText[256];

 AboutDlg = gtk_window_new(GTK_WINDOW_DIALOG);
 gtk_widget_set_usize(AboutDlg, 420, 270);
 gtk_window_set_title(GTK_WINDOW(AboutDlg), "About the P.E.Op.S. PSX X11 Soft Renderer...");
 gtk_window_set_position(GTK_WINDOW(AboutDlg), GTK_WIN_POS_CENTER);
 gtk_container_set_border_width(GTK_CONTAINER(AboutDlg), 10);

 Box = gtk_vbox_new(0, 0);
 gtk_container_add(GTK_CONTAINER(AboutDlg), Box);

 BBox = gtk_hbox_new(0, 10);
 gtk_box_pack_start(GTK_BOX(Box), BBox, FALSE, FALSE, 0);

 sprintf(AboutText, "Version:\n"
                    "Release date:\n"
                    "Coded by:\n"
					"Pete's EMail:\n");

 Label = gtk_label_new(AboutText);
 gtk_label_set_justify(GTK_LABEL(Label), GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0);

 sprintf(AboutText, "%d.%d\n"
                     RELEASE_DATE "\n"
                    "Pete Bernert and the P.E.Op.S. team\n"
					"BlackDove@addcom.de\n"
                  , revision, build);

 Label = gtk_label_new(AboutText);
 gtk_label_set_justify(GTK_LABEL(Label), GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0);

 Label = gtk_label_new("The P.E.Op.S. SoftGPU team:\n");
 gtk_misc_set_alignment(GTK_MISC(Label), 0, 0);
 gtk_box_pack_start(GTK_BOX(Box), Label, FALSE, FALSE, 0);

 BBox = gtk_hbox_new(0, 10);
 gtk_box_pack_start(GTK_BOX(Box), BBox, FALSE, FALSE, 0);

 sprintf(AboutText, "P.E.Op.S. page:\n"
					"- Pete Bernert\n"
                    "- Lewpy\n"
                    "- lu_zero\n"
                    "- linuzappz\n"
                    "- syo\n"
                    "- Darko Matesic");

 Label = gtk_label_new(AboutText);
 gtk_label_set_justify(GTK_LABEL(Label), GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0);

 sprintf(AboutText, "https://sourceforge.net/projects/peops\n"
					"http://home.t-online.de/home/PeteBernert\n"
                    "http://lewpy.psxemu.com/\n"
                    "http://brsk.virtualave.net/lu_zero/\n"
                    "http://www.pcsx.net\n"
                    "http://www.geocities.co.jp/SiliconValley-Bay/2072/\n"
                    "http://mrdario.tripod.com");

 Label = gtk_label_new(AboutText);
 gtk_label_set_justify(GTK_LABEL(Label), GTK_JUSTIFY_LEFT);
 gtk_box_pack_start(GTK_BOX(BBox), Label, FALSE, FALSE, 0);

 BBox = gtk_hbutton_box_new();
 gtk_box_pack_start(GTK_BOX(Box), BBox, FALSE, FALSE, 0);

 Ok = gtk_button_new_with_label("Ok");
 gtk_signal_connect(GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnAboutOk), NULL);
 gtk_container_add(GTK_CONTAINER(BBox), Ok);
 GTK_WIDGET_SET_FLAGS(Ok, GTK_CAN_DEFAULT);

 gtk_widget_show_all(AboutDlg);

 gtk_signal_connect (GTK_OBJECT (AboutDlg), "destroy",  
		      GTK_SIGNAL_FUNC(gtk_main_quit),
		      NULL);
 // added by pete:
 gtk_main ();
}


////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>

/* file helpers */

#define GetValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') var = atoi(p); \
 }

#define SetValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') { \
   len = sprintf(t1, "%d", var); \
   strncpy(p, t1, len); \
   if (p[len] != ' ' && p[len] != '\n' && p[len] != 0) p[len] = ' '; \
  } \
 } \
 else { \
  size+=sprintf(pB+size, "%s = %d\n", name, var); \
 }

void ReadConfigFile()
{

 struct stat buf;
 FILE *in;char t[256];int len, size;
 char * pB, * p;

 if(pConfigFile) 
      strcpy(t,pConfigFile);
 else 
  {
   strcpy(t,"cfg/gpuPeopsSoftX.cfg");
   in = fopen(t,"rb");
   if (!in)
    {
     strcpy(t,"gpuPeopsSoftX.cfg");
     in = fopen(t,"rb");
     if(!in) sprintf(t,"%s/gpuPeopsSoftX.cfg",getenv("HOME"));
     else    fclose(in);
    }
   else     fclose(in);
  }

 if (stat(t, &buf) == -1) return;
 size = buf.st_size;

 in = fopen(t,"rb");
 if (!in) return;

 pB=(char *)malloc(size);
 memset(pB,0,size);

 len = fread(pB, 1, size, in);
 fclose(in);

 GetValue("ResX", iResX);
 if(iResX<20) iResX=20;
 iResX=(iResX/4)*4;

 GetValue("ResY", iResY);
 if(iResY<20) iResY=20;
 iResY=(iResY/4)*4;

 GetValue("NoStretch", iUseNoStrechBlt);

 GetValue("FullScreen", iWindowMode);
 if(iWindowMode!=0) iWindowMode=0;
 else               iWindowMode=1;

 GetValue("ShowFPS", iShowFPS);
 if(iShowFPS<0) iShowFPS=0;
 if(iShowFPS>1) iShowFPS=1;

 GetValue("ScanLines", iUseScanLines);
 if(iUseScanLines<0) iUseScanLines=0;
 if(iUseScanLines>1) iUseScanLines=1;

 GetValue("UseFrameLimit", UseFrameLimit);
 if(UseFrameLimit<0) UseFrameLimit=0;
 if(UseFrameLimit>1) UseFrameLimit=1;

 GetValue("UseFrameSkip", UseFrameSkip);
 if(UseFrameSkip<0) UseFrameSkip=0;
 if(UseFrameSkip>1) UseFrameSkip=1;

 GetValue("FPSDetection", iFrameLimit);
 if(iFrameLimit<1) iFrameLimit=1;
 if(iFrameLimit>2) iFrameLimit=2;

 GetValue("FrameRate", iFrameRate);
 if(iFrameRate<10)   iFrameRate=10;
 if(iFrameRate>1000) iFrameRate=1000;

 GetValue("CfgFixes", dwCfgFixes);

 GetValue("UseFixes", iUseFixes);
 if(iUseFixes<0) iUseFixes=0;
 if(iUseFixes>1) iUseFixes=1;

 free(pB);

}

////////////////////////////////////////////////////////////////////////

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
 iShowFPS=0;

 // read sets
 ReadConfigFile();

 // additional checks
 if(!iColDepth)       iColDepth=32;
 if(iUseFixes)        dwActFixes=dwCfgFixes;
 SetFixes();
}

void WriteConfig(void) {

 struct stat buf;
 FILE *out;char t[256];int len, size;
 char * pB, * p; char t1[8];

 if(pConfigFile) 
      strcpy(t,pConfigFile);
 else 
  {
   strcpy(t,"cfg/gpuPeopsSoftX.cfg");
   out = fopen(t,"rb");
   if (!out) 
    {
     strcpy(t,"gpuPeopsSoftX.cfg");
     out = fopen(t,"rb");
     if(!out) sprintf(t,"%s/gpuPeopsSoftX.cfg",getenv("HOME"));
     else     fclose(out);
    }
   else     fclose(out);
  }

 if (stat(t, &buf) != -1) size = buf.st_size;
 else size = 0;

 out = fopen(t,"rb");
 if (!out) {
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
  iShowFPS=0;

  size = 0;
  pB=(char *)malloc(4096);
  memset(pB,0,4096);
 }
 else {
  pB=(char *)malloc(size+4096);
  memset(pB,0,size+4096);

  len = fread(pB, 1, size, out);
  fclose(out);
 }

 SetValue("ResX", iResX);
 SetValue("ResY", iResY);
 SetValue("NoStretch", iUseNoStrechBlt);
 SetValue("FullScreen", !iWindowMode);
 SetValue("ShowFPS", iShowFPS);
 SetValue("ScanLines", iUseScanLines);
 SetValue("UseFrameLimit", UseFrameLimit);
 SetValue("UseFrameSkip", UseFrameSkip);
 SetValue("FPSDetection", iFrameLimit);
 SetValue("FrameRate", iFrameRate);
 SetValue("CfgFixes", (unsigned int)dwCfgFixes);
 SetValue("UseFixes", iUseFixes);

 out = fopen(t,"wb");
 if (!out) return;

 len = fwrite(pB, 1, size, out);
 fclose(out);

 free(pB);

}
#endif
#endif // LINUX_VERSION






