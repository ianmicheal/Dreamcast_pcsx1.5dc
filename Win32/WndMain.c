/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "resource.h"
#include "AboutDlg.h"

#include "PsxCommon.h"
#include "plugin.h"
#include "Debug.h"
#include "Win32.h"

int AccBreak=0;
int ConfPlug=0;
int StatesC=0;
int NeedReset=1;
int cdOpenCase=0;
#define MAXFILENAME 256

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	gApp.hInstance = hInstance;

	Running=0;

	GetCurrentDirectory(256, PcsxDir);

	memset(&Config, 0, sizeof(PcsxConfig));
	strcpy(Config.Net, "Disabled");
	if (LoadConfig() == -1) {
		Config.PsxAuto = 1;
		strcpy(Config.PluginsDir, "Plugin\\");
		strcpy(Config.BiosDir,    "Bios\\");
		SysMessage("Pcsx needs to be configured");
		ConfPlug=1;
		ConfigurePlugins(gApp.hWnd);
		DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_MCDCONF), gApp.hWnd, (DLGPROC)ConfigureMcdsDlgProc);
		SysMessage("Pcsx now will quit, restart it");
		return 0;
	}

	if (SysInit() == -1) return 1;

	CreateMainWindow(nCmdShow);

	RunGui();

	return 0;
}

void RunGui() {
    MSG msg;

    for (;;) {
		if(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void RestoreWindow() {
	AccBreak = 1;
	DestroyWindow(gApp.hWnd);
	CreateMainWindow(SW_SHOWNORMAL);
	SetMenu(gApp.hWnd, gApp.hMenu);
	ShowCursor(TRUE);
	SetCursor(LoadCursor(gApp.hInstance, IDC_ARROW));
	ShowCursor(TRUE);
}

int Slots[5] = { -1, -1, -1, -1, -1 };

void ResetMenuSlots() {
	int i;

	for (i=0; i<5; i++) {
		if (Slots[i] == -1)
			EnableMenuItem(GetSubMenu(gApp.hMenu, 0), ID_FILE_STATES_LOAD_SLOT1+i, MF_GRAYED);
		else 
			EnableMenuItem(GetSubMenu(gApp.hMenu, 0), ID_FILE_STATES_LOAD_SLOT1+i, MF_ENABLED);
	}
}

void UpdateMenuSlots() {
	char str[256];
	int i;

	for (i=0; i<5; i++) {
		sprintf(str, "sstates/%s.%3.3d", CdromId, i);
		Slots[i] = CheckState(str);
	}
}

void OpenConsole() {
	if (hConsole) return;
	AllocConsole();
	SetConsoleTitle("Psx Output");
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

void CloseConsole() {
	FreeConsole(); hConsole = NULL;
}

void States_Load(int num) {
	char Text[256];
	int ret;

	SetMenu(gApp.hWnd, NULL);
	OpenPlugins(gApp.hWnd);
	SysReset();
	NeedReset = 0;

	sprintf (Text, "sstates/%s.%3.3d", CdromId, num);
	ret = LoadState(Text);
	sprintf (Text, "*PCSX*: %s State %d", !ret ? "Loaded" : "Error Loading", StatesC+1);
	GPU_displayText(Text);

	Running = 1;
	psxCpu->Execute();
}

void States_Save(int num) {
	char Text[256];
	int ret;

	SetMenu(gApp.hWnd, NULL);
	OpenPlugins(gApp.hWnd);
	if (NeedReset) {
		SysReset();
		NeedReset = 0;
	}
	GPU_updateLace();

	sprintf (Text, "sstates/%s.%3.3d", CdromId, num);
	GPU_freeze(2, (GPUFreeze_t *)&num);
	ret = SaveState(Text);
	sprintf (Text, "*PCSX*: %s State %d", !ret ? "Saved" : "Error Saving", StatesC+1);
	GPU_displayText(Text);

	Running = 1;
	psxCpu->Execute();
}

void OnStates_LoadOther() {
	OPENFILENAME ofn;
	char szFileName[MAXFILENAME];
	char szFileTitle[MAXFILENAME];

	memset(&szFileName,  0, sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= gApp.hWnd;
    ofn.lpstrFilter			= "PCSX State Format\0*.*;*.*\0";
	ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 1;
    ofn.lpstrFile			= szFileName;
    ofn.nMaxFile			= MAXFILENAME;
    ofn.lpstrInitialDir		= NULL;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAXFILENAME;
    ofn.lpstrTitle			= NULL;
    ofn.lpstrDefExt			= "EXE";
    ofn.Flags				= OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if (GetOpenFileName ((LPOPENFILENAME)&ofn)) {
		char Text[256];
		int ret;

		SetMenu(gApp.hWnd, NULL);
		OpenPlugins(gApp.hWnd);
		if (NeedReset) {
			SysReset();
			NeedReset = 0;
		}

		ret = LoadState(szFileName);
		sprintf (Text, "*PCSX*: %s State %s", !ret ? "Loaded" : "Error Loading", szFileName);
		GPU_displayText(Text);

		Running = 1;
		psxCpu->Execute();
	}
} 

void OnStates_Save1() { States_Save(0); } 
void OnStates_Save2() { States_Save(1); } 
void OnStates_Save3() { States_Save(2); } 
void OnStates_Save4() { States_Save(3); } 
void OnStates_Save5() { States_Save(4); } 

void OnStates_SaveOther() {
	OPENFILENAME ofn;
	char szFileName[MAXFILENAME];
	char szFileTitle[MAXFILENAME];

	memset(&szFileName,  0, sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= gApp.hWnd;
    ofn.lpstrFilter			= "PCSX State Format\0*.*;*.*\0";
	ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 1;
    ofn.lpstrFile			= szFileName;
    ofn.nMaxFile			= MAXFILENAME;
    ofn.lpstrInitialDir		= NULL;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAXFILENAME;
    ofn.lpstrTitle			= NULL;
    ofn.lpstrDefExt			= "EXE";
    ofn.Flags				= OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if (GetOpenFileName ((LPOPENFILENAME)&ofn)) {
		char Text[256];
		int ret;

		SetMenu(gApp.hWnd, NULL);
		OpenPlugins(gApp.hWnd);
		SysReset();
		NeedReset = 0;

		ret = SaveState(szFileName);
		sprintf (Text, "*PCSX*: %s State %s", !ret ? "Saved" : "Error Saving", szFileName);
		GPU_displayText(Text);

		Running = 1;
		psxCpu->Execute();
	}
} 

LRESULT WINAPI MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	char File[256];

	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_EXIT:
					SysClose();
					PostQuitMessage(0);
					exit(0);
					return TRUE;

				case ID_FILE_RUN_CD:
					LoadCdBios = 0;
					SetMenu(hWnd, NULL);
					OpenPlugins(hWnd);
					SysReset();
					NeedReset = 0;
					CheckCdrom();
					if (LoadCdrom() == -1) {
						ClosePlugins();
						RestoreWindow();
						SysMessage("Could not load Cdrom\n");
						return TRUE;
					}
					ShowCursor(FALSE);
					Running = 1;
					psxCpu->Execute();
					return TRUE;

				case ID_FILE_RUNCDBIOS:
					LoadCdBios = 1;
					SetMenu(hWnd, NULL);
					OpenPlugins(hWnd);
					ShowCursor(FALSE);
					CheckCdrom();
					SysReset();
					NeedReset = 0;
					Running = 1;
					psxCpu->Execute();
					return TRUE;

				case ID_FILE_RUN_EXE:
					if (!Open_File_Proc(File)) return TRUE;
					SetMenu(hWnd, NULL);
					OpenPlugins(hWnd);
					SysReset();
					NeedReset = 0;
					Load(File);
					Running = 1;
					psxCpu->Execute();
					return TRUE;

				case ID_FILE_STATES_LOAD_SLOT1: States_Load(0); return TRUE;
				case ID_FILE_STATES_LOAD_SLOT2: States_Load(1); return TRUE;
				case ID_FILE_STATES_LOAD_SLOT3: States_Load(2); return TRUE;
				case ID_FILE_STATES_LOAD_SLOT4: States_Load(3); return TRUE;
				case ID_FILE_STATES_LOAD_SLOT5: States_Load(4); return TRUE;
				case ID_FILE_STATES_LOAD_OTHER: OnStates_LoadOther(); return TRUE;

				case ID_FILE_STATES_SAVE_SLOT1: States_Save(0); return TRUE;
				case ID_FILE_STATES_SAVE_SLOT2: States_Save(1); return TRUE;
				case ID_FILE_STATES_SAVE_SLOT3: States_Save(2); return TRUE;
				case ID_FILE_STATES_SAVE_SLOT4: States_Save(3); return TRUE;
				case ID_FILE_STATES_SAVE_SLOT5: States_Save(4); return TRUE;
				case ID_FILE_STATES_SAVE_OTHER: OnStates_SaveOther(); return TRUE;

				case ID_EMULATOR_RUN:
					SetMenu(hWnd, NULL);
					OpenPlugins(hWnd);
					ShowCursor(FALSE);
					if (NeedReset) { SysReset(); NeedReset = 0; }
					Running = 1;
					psxCpu->Execute();
					return TRUE;

				case ID_EMULATOR_RESET:
					NeedReset = 1;
					return TRUE;

				case ID_CONFIGURATION_GRAPHICS:
					GPU_configure();
					return TRUE;

				case ID_CONFIGURATION_SOUND:
					SPU_configure();
					return TRUE;

				case ID_CONFIGURATION_CONTROLLERS:
					PAD1_configure();
					if (strcmp(Config.Pad1, Config.Pad2)) PAD2_configure();
					return TRUE;
			    
				case ID_CONFIGURATION_CDROM:
				    CDR_configure();
					return TRUE;

				case ID_CONFIGURATION_NETPLAY:
					DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_NETPLAY), hWnd, (DLGPROC)ConfigureNetPlayDlgProc);
					return TRUE;

				case ID_CONFIGURATION_MEMORYCARDMANAGER:
					DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_MCDCONF), hWnd, (DLGPROC)ConfigureMcdsDlgProc);
					return TRUE;

				case ID_CONFIGURATION_CPU:
					DialogBox(gApp.hInstance, MAKEINTRESOURCE(IDD_CPUCONF), hWnd, (DLGPROC)ConfigureCpuDlgProc);
					return TRUE;

				case ID_CONFIGURATION:
					ConfigurePlugins(hWnd);
					return TRUE;

				case ID_HELP_HELP:
					ShellExecute(NULL, "open", "Readme.txt", NULL, NULL, SW_SHOWNORMAL);
					return TRUE;

				case ID_HELP_ABOUT:
					DialogBox(gApp.hInstance, MAKEINTRESOURCE(ABOUT_DIALOG), hWnd, (DLGPROC)AboutDlgProc);
					return TRUE;
			}
			break;

		case WM_SYSKEYDOWN:
			if (wParam != VK_F10)
				return DefWindowProc(hWnd, msg, wParam, lParam);
		case WM_KEYDOWN:
			PADhandleKey(wParam);
			return TRUE;
	
		case WM_DESTROY:
			if (!AccBreak) {
				if (Running) ClosePlugins();
				SysClose();
				PostQuitMessage(0);
				exit(0);
			}
			else AccBreak = 0;
		
			return TRUE;

		case WM_CREATE:
			gApp.hMenu = LoadMenu(gApp.hInstance, MAKEINTRESOURCE(IDR_MENU1));
			SetMenu(hWnd, gApp.hMenu);
			break;

		case WM_QUIT:
			exit(0);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return FALSE;
}

HWND mcdDlg;
McdBlock Blocks[2][15];
int IconC[2][15];
HIMAGELIST Iiml[2];
HICON eICON;

void CreateListView(int idc) {
	HWND List;
	LV_COLUMN col;

	List = GetDlgItem(mcdDlg, idc);

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt  = LVCFMT_LEFT;
	
	col.pszText  = "Title";
	col.cx       = 170;
	col.iSubItem = 0;

	ListView_InsertColumn(List, 0, &col);

	col.pszText  = "Status";
	col.cx       = 50;
	col.iSubItem = 1;

	ListView_InsertColumn(List, 1, &col);

	col.pszText  = "Game ID";
	col.cx       = 90;
	col.iSubItem = 2;

	ListView_InsertColumn(List, 2, &col);

	col.pszText  = "Game";
	col.cx       = 80;
	col.iSubItem = 3;

	ListView_InsertColumn(List, 3, &col);
}

int GetRGB() {
    HDC scrDC, memDC;
    HBITMAP oldBmp = NULL; 
    HBITMAP curBmp = NULL;
    COLORREF oldColor;
    COLORREF curColor = RGB(255,255,255);
    int i, R, G, B;

    R = G = B = 1;
 
    scrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    memDC = CreateCompatibleDC(NULL); 
    curBmp = CreateCompatibleBitmap(scrDC, 1, 1);    
    oldBmp = (HBITMAP)SelectObject(memDC, curBmp);
        
    for (i = 255; i >= 0; --i) {
        oldColor = curColor;
        curColor = SetPixel(memDC, 0, 0, RGB(i, i, i));
        
        if (GetRValue(curColor) < GetRValue(oldColor)) ++R; 
        if (GetGValue(curColor) < GetGValue(oldColor)) ++G;
        if (GetBValue(curColor) < GetBValue(oldColor)) ++B;
    }
 
    DeleteObject(oldBmp);
    DeleteObject(curBmp);
    DeleteDC(scrDC);
    DeleteDC(memDC);
 
    return (R * G * B);
}
 
HICON GetIcon(short *icon) {
    ICONINFO iInfo;
    HDC hDC;
    char mask[16*16];
    int x, y, c, Depth;
  
    hDC = CreateIC("DISPLAY",NULL,NULL,NULL);
    Depth=GetDeviceCaps(hDC, BITSPIXEL);
    DeleteDC(hDC);
 
    if (Depth == 16) {
        if (GetRGB() == (32 * 32 * 32))        
            Depth = 15;
    }
 
    for (y=0; y<16; y++) {
        for (x=0; x<16; x++) {
            c = icon[y*16+x];
            if (Depth == 15)
				c = ((c&0x001f) << 10) | ((c&0x7c00) >> 10) | (c&0x03e0);
			else
                c = (c&0x001f) | ((c&0x7c00) << 1) | ((c&0x03e0) << 1);

            icon[y*16+x] = c;
        }
    }    

    iInfo.fIcon = TRUE;
    memset(mask, 0, 16*16);
    iInfo.hbmMask  = CreateBitmap(16, 16, 1, 1, mask);
    iInfo.hbmColor = CreateBitmap(16, 16, 1, 16, icon); 
 
    return CreateIconIndirect(&iInfo);
}

HICON hICON[2][3][15];
int aIover[2];                        
int ani[2];
 
void LoadMcdItems(int mcd, int idc) {
    HWND List = GetDlgItem(mcdDlg, idc);
    LV_ITEM item;
    HIMAGELIST iml = Iiml[mcd-1];
    int i, j;
    HICON hIcon;
    McdBlock *Info;
 
    aIover[mcd-1]=0;
    ani[mcd-1]=0;
 
    ListView_DeleteAllItems(List);

    for (i=0; i<15; i++) {
  
        item.mask      = LVIF_TEXT | LVIF_IMAGE;
        item.iItem       = i;
        item.iImage    = i;
        item.pszText  = LPSTR_TEXTCALLBACK;
        item.iSubItem = 0;
 
        IconC[mcd-1][i] = 0;
        Info = &Blocks[mcd-1][i];
 
        if ((Info->Flags & 0xF) == 1 && Info->IconCount != 0) {
            hIcon = GetIcon(Info->Icon);   
 
            if (Info->IconCount > 1) {
                for(j = 0; j < 3; j++)
                    hICON[mcd-1][j][i]=hIcon;
            }
        } else {
            hIcon = eICON; 
        }
 
        ImageList_ReplaceIcon(iml, -1, hIcon);
        ListView_InsertItem(List, &item);
    } 
}

void UpdateMcdItems(int mcd, int idc) {
    HWND List = GetDlgItem(mcdDlg, idc);
    LV_ITEM item;
    HIMAGELIST iml = Iiml[mcd-1];
    int i, j;
    McdBlock *Info;
    HICON hIcon;
 
    aIover[mcd-1]=0;
    ani[mcd-1]=0;
  
    for (i=0; i<15; i++) { 
 
        item.mask     = LVIF_TEXT | LVIF_IMAGE;
        item.iItem    = i;
        item.iImage   = i;
        item.pszText  = LPSTR_TEXTCALLBACK;
        item.iSubItem = 0;
 
        IconC[mcd-1][i] = 0; 
        Info = &Blocks[mcd-1][i];
 
        if ((Info->Flags & 0xF) == 1 && Info->IconCount != 0) {
            hIcon = GetIcon(Info->Icon);   
 
            if (Info->IconCount > 1) { 
                for(j = 0; j < 3; j++)
                    hICON[mcd-1][j][i]=hIcon;
            }
        } else { 
            hIcon = eICON; 
        }
              
        ImageList_ReplaceIcon(iml, i, hIcon);
        ListView_SetItem(List, &item);
    } 
    ListView_Update(List, -1);
}
 
void McdListGetDispInfo(int mcd, int idc, LPNMHDR pnmh) {
	LV_DISPINFO *lpdi = (LV_DISPINFO *)pnmh;
	McdBlock *Info;

	Info = &Blocks[mcd-1][lpdi->item.iItem];

	switch (lpdi->item.iSubItem) {
		case 0:
			switch (Info->Flags & 0xF) {
				case 1:
					lpdi->item.pszText = Info->Title;
					break;
				case 2:
					lpdi->item.pszText = "mid link block";
					break;
				case 3:
					lpdi->item.pszText = "terminiting link block";
					break;
			}
			break;
		case 1:
			if ((Info->Flags & 0xF0) == 0xA0) {
				if ((Info->Flags & 0xF) >= 1 &&
					(Info->Flags & 0xF) <= 3) {
					lpdi->item.pszText = "Deleted";
				} else lpdi->item.pszText = "Free";
			} else if ((Info->Flags & 0xF0) == 0x50)
				lpdi->item.pszText = "Used";
			else { lpdi->item.pszText = "Free"; }
			break;
		case 2:
			if((Info->Flags & 0xF)==1)
				lpdi->item.pszText = Info->ID;
			break;
		case 3:
			if((Info->Flags & 0xF)==1)
				lpdi->item.pszText = Info->Name;
			break;
	}
}

void McdListNotify(int mcd, int idc, LPNMHDR pnmh) {
	switch (pnmh->code) {
		case LVN_GETDISPINFO: McdListGetDispInfo(mcd, idc, pnmh); break;
	}
}

void UpdateMcdDlg() {
	int i;

	for (i=1; i<16; i++) GetMcdBlockInfo(1, i, &Blocks[0][i-1]);
	for (i=1; i<16; i++) GetMcdBlockInfo(2, i, &Blocks[1][i-1]);
	UpdateMcdItems(1, IDC_LIST1);
	UpdateMcdItems(2, IDC_LIST2);
}

void LoadMcdDlg() {
	int i;

	for (i=1; i<16; i++) GetMcdBlockInfo(1, i, &Blocks[0][i-1]);
	for (i=1; i<16; i++) GetMcdBlockInfo(2, i, &Blocks[1][i-1]);
	LoadMcdItems(1, IDC_LIST1);
	LoadMcdItems(2, IDC_LIST2);
}

void UpdateMcdIcon(int mcd, int idc) {
    HWND List = GetDlgItem(mcdDlg, idc);
    HIMAGELIST iml = Iiml[mcd-1];
    int i;
    McdBlock *Info;
    int *count; 
 
    if(!aIover[mcd-1]) {
        ani[mcd-1]++; 
 
        for (i=0; i<15; i++) {
            Info = &Blocks[mcd-1][i];
            count = &IconC[mcd-1][i];
 
            if ((Info->Flags & 0xF) != 1) continue;
            if (Info->IconCount <= 1) continue;
 
            if (*count < Info->IconCount) {
                (*count)++;
                aIover[mcd-1]=0;
 
                if(ani[mcd-1] <= (Info->IconCount-1))  // last frame and below...
                    hICON[mcd-1][ani[mcd-1]][i] = GetIcon(&Info->Icon[(*count)*16*16]);
            } else {
                aIover[mcd-1]=1;
            }
        }

    } else { 
 
        if (ani[mcd-1] > 1) ani[mcd-1] = 0;  // 1st frame
        else ani[mcd-1]++;                       // 2nd, 3rd frame
 
        for(i=0;i<15;i++) {
            Info = &Blocks[mcd-1][i];
 
            if (((Info->Flags & 0xF) == 1) && (Info->IconCount > 1))
                ImageList_ReplaceIcon(iml, i, hICON[mcd-1][ani[mcd-1]][i]);
        }
        InvalidateRect(List,  NULL, FALSE);
    }
}

static int copy = 0, copymcd = 0;
static int listsel = 0;

BOOL CALLBACK ConfigureMcdsDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char str[256];
	LPBYTE lpAND, lpXOR;
	LPBYTE lpA, lpX;
	int i, j;

	switch(uMsg) {
		case WM_INITDIALOG:
			mcdDlg = hW;

			lpA=lpAND=(LPBYTE)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(16*16));
			lpX=lpXOR=(LPBYTE)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(16*16));

			for(i=0;i<16;i++)
			{
				for(j=0;j<16;j++)
				{
					*lpA++=0xff;
					*lpX++=0;
				}
			}
			eICON=CreateIcon(gApp.hInstance,16,16,1,1,lpAND,lpXOR);

			HeapFree(GetProcessHeap(),0,lpAND);
			HeapFree(GetProcessHeap(),0,lpXOR);

			if (!strlen(Config.Mcd1)) strcpy(Config.Mcd1, "memcards\\Mcd001.mcr");
			if (!strlen(Config.Mcd2)) strcpy(Config.Mcd2, "memcards\\Mcd002.mcr");
			Edit_SetText(GetDlgItem(hW,IDC_MCD1), Config.Mcd1);
			Edit_SetText(GetDlgItem(hW,IDC_MCD2), Config.Mcd2);

			CreateListView(IDC_LIST1);
			CreateListView(IDC_LIST2);
 
            Iiml[0] = ImageList_Create(16, 16, ILC_COLOR16, 0, 0);
            Iiml[1] = ImageList_Create(16, 16, ILC_COLOR16, 0, 0);
 
            ListView_SetImageList(GetDlgItem(mcdDlg, IDC_LIST1), Iiml[0], LVSIL_SMALL);
            ListView_SetImageList(GetDlgItem(mcdDlg, IDC_LIST2), Iiml[1], LVSIL_SMALL);

			Button_Enable(GetDlgItem(hW, IDC_PASTE), FALSE);

			LoadMcdDlg();

			SetTimer(hW, 1, 250, NULL);

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_COPYTO1:
					copy = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST2));
					copymcd = 1;

					Button_Enable(GetDlgItem(hW, IDC_PASTE), TRUE);
					return TRUE;
				case IDC_COPYTO2:
					copy = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST1));
					copymcd = 2;

					Button_Enable(GetDlgItem(hW, IDC_PASTE), TRUE);
					return TRUE;
				case IDC_PASTE:
					if (MessageBox(hW, "Are you sure you want to paste this selection?", "Confirmation", MB_YESNO) == IDNO) return TRUE;

					if (copymcd == 1) {
						Edit_GetText(GetDlgItem(hW,IDC_MCD1), str, 256);
						i = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST1));

						// save dir data + save data
						memcpy(Mcd1Data + (i+1) * 128, Mcd2Data + (copy+1) * 128, 128);
						SaveMcd(str, Mcd1Data, (i+1) * 128, 128);
						memcpy(Mcd1Data + (i+1) * 1024 * 8, Mcd2Data + (copy+1) * 1024 * 8, 1024 * 8);
						SaveMcd(str, Mcd1Data, (i+1) * 1024 * 8, 1024 * 8);
					} else { // 2
						Edit_GetText(GetDlgItem(hW,IDC_MCD2), str, 256);
						i = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST2));

						// save dir data + save data
						memcpy(Mcd2Data + (i+1) * 128, Mcd1Data + (copy+1) * 128, 128);
						SaveMcd(str, Mcd2Data, (i+1) * 128, 128);
						memcpy(Mcd2Data + (i+1) * 1024 * 8, Mcd1Data + (copy+1) * 1024 * 8, 1024 * 8);
						SaveMcd(str, Mcd2Data, (i+1) * 1024 * 8, 1024 * 8);
					}

					UpdateMcdDlg();

					return TRUE;
				case IDC_DELETE1:
				{
					McdBlock *Info;
					int mcd = 1;
					int i, xor = 0, j;
					unsigned char *data, *ptr;

					Edit_GetText(GetDlgItem(hW,IDC_MCD1), str, 256);
					i = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST1));
					data = Mcd1Data;

					i++;

					ptr = data + i * 128;

					Info = &Blocks[mcd-1][i-1];

					if ((Info->Flags & 0xF0) == 0xA0) {
						if ((Info->Flags & 0xF) >= 1 &&
							(Info->Flags & 0xF) <= 3) { // deleted
							*ptr = 0x50 | (Info->Flags & 0xF);
						} else return TRUE;
					} else if ((Info->Flags & 0xF0) == 0x50) { // used
						*ptr = 0xA0 | (Info->Flags & 0xF);
					} else { return TRUE; }

					for (j=0; j<127; j++) xor^=*ptr++;
					*ptr = xor;

					SaveMcd(str, data, i * 128, 128);
					UpdateMcdDlg();
				}

					return TRUE;
				case IDC_DELETE2:
				{
					McdBlock *Info;
					int mcd = 2;
					int i, xor = 0, j;
					unsigned char *data, *ptr;

					Edit_GetText(GetDlgItem(hW,IDC_MCD2), str, 256);
					i = ListView_GetSelectionMark(GetDlgItem(mcdDlg, IDC_LIST2));
					data = Mcd2Data;

					i++;

					ptr = data + i * 128;

					Info = &Blocks[mcd-1][i-1];

					if ((Info->Flags & 0xF0) == 0xA0) {
						if ((Info->Flags & 0xF) >= 1 &&
							(Info->Flags & 0xF) <= 3) { // deleted
							*ptr = 0x50 | (Info->Flags & 0xF);
						} else return TRUE;
					} else if ((Info->Flags & 0xF0) == 0x50) { // used
						*ptr = 0xA0 | (Info->Flags & 0xF);
					} else { return TRUE; }

					for (j=0; j<127; j++) xor^=*ptr++;
					*ptr = xor;

					SaveMcd(str, data, i * 128, 128);
					UpdateMcdDlg();
				}

					return TRUE;

				case IDC_MCDSEL1: 
					Open_Mcd_Proc(hW, 1); 
					return TRUE;
				case IDC_MCDSEL2: 
					Open_Mcd_Proc(hW, 2); 
					return TRUE;
				case IDC_RELOAD1: 
					Edit_GetText(GetDlgItem(hW,IDC_MCD1), str, 256);
					LoadMcd(1, str);
					UpdateMcdDlg();
					return TRUE;
				case IDC_RELOAD2: 
					Edit_GetText(GetDlgItem(hW,IDC_MCD2), str, 256);
					LoadMcd(2, str);
					UpdateMcdDlg();
					return TRUE;
				case IDC_FORMAT1:
					if (MessageBox(hW, "Are you sure you want to format this Memory Card?", "Confirmation", MB_YESNO) == IDNO) return TRUE;
					Edit_GetText(GetDlgItem(hW,IDC_MCD1), str, 256);
					CreateMcd(str);
					LoadMcd(1, str);
					UpdateMcdDlg();
					return TRUE;
				case IDC_FORMAT2:
					if (MessageBox(hW, "Are you sure you want to format this Memory Card?", "Confirmation", MB_YESNO) == IDNO) return TRUE;
					Edit_GetText(GetDlgItem(hW,IDC_MCD2), str, 256);
					CreateMcd(str);
					LoadMcd(2, str);
					UpdateMcdDlg();
					return TRUE;
       			case IDCANCEL:
					LoadMcds(Config.Mcd1, Config.Mcd2);

					EndDialog(hW,FALSE);

					return TRUE;
       			case IDOK:
					Edit_GetText(GetDlgItem(hW,IDC_MCD1), Config.Mcd1, 256);
					Edit_GetText(GetDlgItem(hW,IDC_MCD2), Config.Mcd2, 256);

					LoadMcds(Config.Mcd1, Config.Mcd2);
					SaveConfig();

					EndDialog(hW,TRUE);

					return TRUE;
			}
		case WM_NOTIFY:
			switch (wParam) {
				case IDC_LIST1: McdListNotify(1, IDC_LIST1, (LPNMHDR)lParam); break;
				case IDC_LIST2: McdListNotify(2, IDC_LIST2, (LPNMHDR)lParam); break;
			}
			return TRUE;
		case WM_TIMER:
			UpdateMcdIcon(1, IDC_LIST1);
			UpdateMcdIcon(2, IDC_LIST2);
			return TRUE;
		case WM_DESTROY:
			DestroyIcon(eICON);
			//KillTimer(hW, 1);
			return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ConfigureCpuDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	long tmp;

	switch(uMsg) {
		case WM_INITDIALOG:
			Button_SetCheck(GetDlgItem(hW,IDC_XA),      Config.Xa);
			Button_SetCheck(GetDlgItem(hW,IDC_SIO),     Config.Sio);
			Button_SetCheck(GetDlgItem(hW,IDC_MDEC),    Config.Mdec);
			Button_SetCheck(GetDlgItem(hW,IDC_QKEYS),   Config.QKeys);
			Button_SetCheck(GetDlgItem(hW,IDC_CDDA),    Config.Cdda);
	   		Button_SetCheck(GetDlgItem(hW,IDC_PSXAUTO), Config.PsxAuto);
	   		Button_SetCheck(GetDlgItem(hW,IDC_CPU),     Config.Cpu);
	   		Button_SetCheck(GetDlgItem(hW,IDC_PSXOUT),  Config.PsxOut);
	   		Button_SetCheck(GetDlgItem(hW,IDC_SPUIRQ),  Config.SpuIrq);
	   		Button_SetCheck(GetDlgItem(hW,IDC_RCNTFIX), Config.RCntFix);
	   		ComboBox_AddString(GetDlgItem(hW,IDC_PSXTYPES),"NTSC");
	   		ComboBox_AddString(GetDlgItem(hW,IDC_PSXTYPES),"PAL");
	   		ComboBox_SetCurSel(GetDlgItem(hW,IDC_PSXTYPES),Config.PsxType);

		case WM_COMMAND: {
     		switch (LOWORD(wParam)) {
       			case IDCANCEL: EndDialog(hW,FALSE); return TRUE;
        		case IDOK:
					tmp = ComboBox_GetCurSel(GetDlgItem(hW,IDC_PSXTYPES));
					if (tmp == 0) Config.PsxType = 0;
					else Config.PsxType = 1;

					Config.Xa      = Button_GetCheck(GetDlgItem(hW,IDC_XA));
					Config.Sio     = Button_GetCheck(GetDlgItem(hW,IDC_SIO));
					Config.Mdec    = Button_GetCheck(GetDlgItem(hW,IDC_MDEC));
					Config.QKeys   = Button_GetCheck(GetDlgItem(hW,IDC_QKEYS));
					Config.Cdda    = Button_GetCheck(GetDlgItem(hW,IDC_CDDA));
					Config.PsxAuto = Button_GetCheck(GetDlgItem(hW,IDC_PSXAUTO));
					tmp = Config.Cpu;
					Config.Cpu     = Button_GetCheck(GetDlgItem(hW,IDC_CPU));
					if (tmp != Config.Cpu) {
						psxCpu->Shutdown();
						if (Config.Cpu)	
							 psxCpu = &psxInt;
						else psxCpu = &psxRec;
						if (psxCpu->Init() == -1) {
							SysClose();
							exit(1);
						}
						psxCpu->Reset();
					}
					Config.PsxOut  = Button_GetCheck(GetDlgItem(hW,IDC_PSXOUT));
					Config.SpuIrq  = Button_GetCheck(GetDlgItem(hW,IDC_SPUIRQ));
					Config.RCntFix = Button_GetCheck(GetDlgItem(hW,IDC_RCNTFIX));

					SaveConfig();

					EndDialog(hW,TRUE);

					if (Config.PsxOut) OpenConsole();
					else CloseConsole();

					return TRUE;
			}
		}
	}
	return FALSE;
}

void Open_Mcd_Proc(HWND hW, int mcd) {
	OPENFILENAME ofn;
	char szFileName[MAXFILENAME];
	char szFileTitle[MAXFILENAME];

	memset(&szFileName,  0, sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= hW;
    ofn.lpstrFilter			= "Psx Mcd Format (*.mcr;*.mc;*.mem;*.vgs;*.mcd;*.gme;*.ddf)\0*.mcr;*.mcd;*.mem;*.gme;*.mc;*.ddf\0Psx Memory Card (*.mcr;*.mc)\0*.mcr;0*.mc\0CVGS Memory Card (*.mem;*.vgs)\0*.mem;*.vgs\0Bleem Memory Card (*.mcd)\0*.mcd\0DexDrive Memory Card (*.gme)\0*.gme\0DataDeck Memory Card (*.ddf)\0*.ddf\0";;
	ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 1;
    ofn.lpstrFile			= szFileName;
    ofn.nMaxFile			= MAXFILENAME;
    ofn.lpstrInitialDir		= "memcards";
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAXFILENAME;
    ofn.lpstrTitle			= NULL;
    ofn.lpstrDefExt			= "MCR";
    ofn.Flags				= OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if (GetOpenFileName ((LPOPENFILENAME)&ofn)) {
		Edit_SetText(GetDlgItem(hW,mcd == 1 ? IDC_MCD1 : IDC_MCD2), szFileName);
		LoadMcd(mcd, szFileName);
		UpdateMcdDlg();
	}
}

int Open_File_Proc(char *file) {
	OPENFILENAME ofn;
	char szFileName[MAXFILENAME];
	char szFileTitle[MAXFILENAME];

	memset(&szFileName,  0, sizeof(szFileName));
	memset(&szFileTitle, 0, sizeof(szFileTitle));

    ofn.lStructSize			= sizeof(OPENFILENAME);
    ofn.hwndOwner			= gApp.hWnd;
    ofn.lpstrFilter			= "Psx Exe Format\0*.*;*.*\0";
	ofn.lpstrCustomFilter	= NULL;
    ofn.nMaxCustFilter		= 0;
    ofn.nFilterIndex		= 1;
    ofn.lpstrFile			= szFileName;
    ofn.nMaxFile			= MAXFILENAME;
    ofn.lpstrInitialDir		= NULL;
    ofn.lpstrFileTitle		= szFileTitle;
    ofn.nMaxFileTitle		= MAXFILENAME;
    ofn.lpstrTitle			= NULL;
    ofn.lpstrDefExt			= "EXE";
    ofn.Flags				= OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

	if (GetOpenFileName ((LPOPENFILENAME)&ofn)) {
		strcpy(file, szFileName);
		return 1;
	} else
		return 0;
}

void CreateMainWindow(int nCmdShow) {
	char strTitle[20];      
	WNDCLASS wc;
	HWND hWnd;

	LoadString(gApp.hInstance, IDS_TITLE, strTitle, 20);

	wc.lpszClassName = "P©SX Main";
	wc.lpfnWndProc = MainWndProc;
	wc.style = 0;
	wc.hInstance = gApp.hInstance;
	wc.hIcon = LoadIcon(gApp.hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_MENUTEXT);
	wc.lpszMenuName = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	RegisterClass(&wc);

	hWnd = CreateWindow("P©SX Main",
						strTitle,
						WS_OVERLAPPED | WS_SYSMENU,
						20,
						20,
						320,
						240,
						NULL,
						NULL,
						gApp.hInstance,
						NULL);

	gApp.hWnd = hWnd;
	ResetMenuSlots();

	ShowWindow(hWnd, nCmdShow);
}

int SysInit() {
	if (Config.PsxOut) OpenConsole();

	if (psxInit() == -1) return -1;

	#ifdef GTE_DUMP
	gteLog = fopen("gteLog.txt","w");
	#endif

#ifdef EMU_LOG
	emuLog = fopen("emuLog.txt","w");
	setvbuf(emuLog, NULL,  _IONBF, 0);
#endif

	LoadPlugins();
	LoadMcds(Config.Mcd1, Config.Mcd2);

	return 0;
}

void SysReset() {
	psxReset();
}


void SysClose() {
	psxShutdown();
	ReleasePlugins();

	if (Config.PsxOut) CloseConsole();

	if (emuLog != NULL) fclose(emuLog);
	#ifdef GTE_DUMP
	if (gteLog != NULL) fclose(gteLog);
	#endif
}

void SysPrintf(char *fmt, ...) {
	va_list list;
	char msg[512];
	DWORD tmp;

	if (!hConsole) return;

	va_start(list,fmt);
	vsprintf(msg,fmt,list);
	va_end(list);

	WriteConsole(hConsole, msg, (DWORD)strlen(msg), &tmp, 0);
#ifdef EMU_LOG
#ifndef LOG_STDOUT
	fprintf(emuLog, "%s", msg);
#endif
#endif
}

void SysMessage(char *fmt, ...) {
	va_list list;
	char tmp[512];

	va_start(list,fmt);
	vsprintf(tmp,fmt,list);
	va_end(list);
	MessageBox(0, tmp, "Pcsx Msg", 0);
}

static char *err = "Error Loading Symbol";
static int errval;

void *SysLoadLibrary(char *lib) {
	return LoadLibrary(lib);
}

void *SysLoadSym(void *lib, char *sym) {
	void *tmp = GetProcAddress((HINSTANCE)lib, sym);
	if (tmp == NULL) errval = 1;
	else errval = 0;
	return tmp;
}

char *SysLibError() {
	if (errval) { errval = 0; return err; }
	return NULL;
}

void SysCloseLibrary(void *lib) {
	FreeLibrary((HINSTANCE)lib);
}

void SysUpdate() {
    MSG msg;

	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void SysRunGui() {
	RestoreWindow();
	RunGui();
}