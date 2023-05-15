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
#include <stdio.h>
#include "plugins.h"
#include "resource.h"
#include <time.h>
#include <stdio.h>
#include "R3000A.h"
#include "Win32.h"

void PADhandleKey(int key) {
	char Text[255];
	int ret;

	if (Running == 0) return;
	switch (key) {
		case 0: break;
		case VK_F1:
			sprintf (Text, "sstates/%s.%3.3d", CdromId, StatesC);
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			ret = SaveState(Text);
			sprintf (Text, "*PCSX*: %s State %d", !ret ? "Saved" : "Error Saving", StatesC+1);
			GPU_displayText(Text);
			return;

		case VK_F2:
			if (StatesC < 4) StatesC++;
			else StatesC = 0;
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			return;

		case VK_F3:
			sprintf (Text, "sstates/%s.%3.3d", CdromId, StatesC);
			ret = LoadState(Text);
			sprintf (Text, "*PCSX*: %s State %d", !ret ? "Loaded" : "Error Loading", StatesC+1);
			GPU_displayText(Text);
			return;

		case VK_F4:
			{
			gzFile f;
			static int ShowPic;
			static unsigned char *pMem;

			if (!ShowPic) {
				sprintf (Text, "sstates/%s.%3.3d", CdromId, StatesC);
				f = gzopen(Text, "rb");
				if (f == NULL) return;

				gzseek(f, 32, SEEK_SET); // skip header

				pMem = (unsigned char *) malloc(128*96*3);
				gzread(f, pMem, 128*96*3);
				gzclose(f);
				GPU_freeze(2, (GPUFreeze_t *)&StatesC);
				GPU_showScreenPic(pMem);
				free(pMem);
				ShowPic = 1;
			}
			else {
				GPU_showScreenPic(NULL);
				ShowPic = 0;
			}

			}
			return;

		case VK_F5:
			if (Config.QKeys) break;
			Config.Sio ^= 0x1;
			sprintf (Text, "*PCSX*: Sio Irq %sAlways Enabled", Config.Sio ? "" : "Not ");
			GPU_displayText(Text);
			return;

		case VK_F6:
			if (Config.QKeys) break;
			Config.Mdec ^= 0x1;
			sprintf (Text, "*PCSX*: Black&White Only Mdecs %sabled", Config.Mdec ? "En" : "Dis");
			GPU_displayText(Text);
			return;

		case VK_F7:
			if (Config.QKeys) break;
			Config.Xa ^= 0x1;
			sprintf (Text, "*PCSX*: Xa %sabled", !Config.Xa ? "En" : "Dis");
			GPU_displayText(Text);
			return;

		case VK_F8:
			if (Config.QKeys) break;
			GPU_makeSnapshot();
			return;

		case VK_F9:
			GPU_displayText("*PCSX*: CdRom Case Opened");
			cdOpenCase = 1;
			return;

		case VK_F10:
			GPU_displayText("*PCSX*: CdRom Case Closed");
			cdOpenCase = 0;
			return;

		case VK_ESCAPE:
			Running = 0;
			ClosePlugins();
			SysRunGui();
			return;
	}
}

void CALLBACK SPUirq(void);

void CALLBACK clearDynarec(void) {
	psxCpu->Reset();
}

char charsTable[8] = { "|/-\\" };

BOOL CALLBACK ConnectDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char str[256];
	static int waitState;

	switch(uMsg) {
		case WM_INITDIALOG:
			SetTimer(hW, 0, 100, NULL);
			return TRUE;

		case WM_TIMER:
			if (++waitState == 4) waitState = 0;
			sprintf(str, "Please wait while connecting... %c\n", charsTable[waitState]);
			Static_SetText(GetDlgItem(hW, IDC_CONNECTSTR), str);
			return TRUE;

/*		case WM_COMMAND:
			switch (LOWORD(wParam)) {
       			case IDCANCEL:
					WaitCancel = 1;
					return TRUE;
			}*/
	}

	return FALSE;
}

int NetOpened = 0;

void OpenPlugins(HWND hWnd) {
	int ret;

	GPU_clearDynarec(clearDynarec);

	ret = CDR_open();
	if (ret < 0) { SysMessage ("Error Opening CDR Plugin\n"); exit(1); }
	ret = GPU_open(hWnd);
	if (ret < 0) { SysMessage ("Error Opening GPU Plugin (%d)\n", ret); exit(1); }
	ret = SPU_open(hWnd);
	if (ret < 0) { SysMessage ("Error Opening SPU Plugin\n"); exit(1); }
	SPU_registerCallback(SPUirq);
	ret = PAD1_open(hWnd);
	if (ret < 0) { SysMessage ("Error Opening PAD1 Plugin\n"); exit(1); }
	ret = PAD2_open(hWnd);
	if (ret < 0) { SysMessage ("Error Opening PAD2 Plugin\n"); exit(1); }

	if (Config.UseNet && NetOpened == 0) {
		ret = NET_open(hWnd);
		if (ret < 0) Config.UseNet = 0;
		else {
			HWND hW = CreateDialog(gApp.hInstance, MAKEINTRESOURCE(IDD_CONNECT), gApp.hWnd, ConnectDlgProc);
			ShowWindow(hW, SW_SHOW);

			if (NET_queryServer() == 1) {
				if (SendPcsxInfo() == -1) Config.UseNet = 0;
			} else {
				if (RecvPcsxInfo() == -1) Config.UseNet = 0;
			}

			DestroyWindow(hW);
		}
		NetOpened = 1;
	} else if (Config.UseNet) {
		NET_resume();
	}

	SetCurrentDirectory(PcsxDir);
	ShowCursor(FALSE);
}

void ClosePlugins() {
	int ret;

	UpdateMenuSlots();
	ret = CDR_close();
	if (ret < 0) { SysMessage ("Error Closing CDR Plugin\n"); exit(1); }
	ret = GPU_close();
	if (ret < 0) { SysMessage ("Error Closing GPU Plugin\n"); exit(1); }
	ret = SPU_close();
	if (ret < 0) { SysMessage ("Error Closing SPU Plugin\n"); exit(1); }
	ret = PAD1_close();
	if (ret < 0) { SysMessage ("Error Closing PAD1 Plugin\n"); exit(1); }
	ret = PAD2_close();
	if (ret < 0) { SysMessage ("Error Closing PAD2 Plugin\n"); exit(1); }

	if (Config.UseNet) {
		NET_pause();
	}
}

void ResetPlugins() {
	int ret;

	CDR_shutdown();
	GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();
	if (Config.UseNet) NET_shutdown(); 

	ret = CDR_init();
	if (ret != 0) { SysMessage ("CDRinit error : %d\n",ret); exit(1); }
	ret = GPU_init();
	if (ret != 0) { SysMessage ("GPUinit error : %d\n",ret); exit(1); }
	ret = SPU_init();
	if (ret != 0) { SysMessage ("SPUinit error : %d\n",ret); exit(1); }
	ret = PAD1_init(1);
	if (ret != 0) { SysMessage ("PAD1init error : %d\n",ret); exit(1); }
	ret = PAD2_init(2);
	if (ret != 0) { SysMessage ("PAD2init error : %d\n",ret); exit(1); }
	if (Config.UseNet) {
		ret = NET_init();
		if (ret < 0) { SysMessage ("NETinit error : %d\n",ret); exit(1); }
	}

	NetOpened = 0;
}


