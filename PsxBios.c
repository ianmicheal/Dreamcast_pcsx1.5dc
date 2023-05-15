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

#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "PsxCommon.h"
//We try to emulate bios :) HELP US :P

char *biosA0n[256] = {
// 0x00
	"open",		"lseek",	"read",		"write",
	"close",	"ioctl",	"exit",		"sys_a0_07",
	"getc",		"putc",		"todigit",	"atof",
	"strtoul",	"strtol",	"abs",		"labs",
// 0x10
	"atoi",		"atol",		"atob",		"setjmp",
	"longjmp",	"strcat",	"strncat",	"strcmp",
	"strncmp",	"strcpy",	"strncpy",	"strlen",
	"index",	"rindex",	"strchr",	"strrchr",
// 0x20
	"strpbrk",	"strspn",	"strcspn",	"strtok",
	"strstr",	"toupper",	"tolower",	"bcopy",
	"bzero",	"bcmp",		"memcpy",	"memset",
	"memmove",	"memcmp",	"memchr",	"rand",
// 0x30
	"srand",	"qsort",	"strtod",	"malloc",
	"free",		"lsearch",	"bsearch",	"calloc",
	"realloc",	"InitHeap",	"_exit",	"getchar",
	"putchar",	"gets",		"puts",		"printf",
// 0x40
	"sys_a0_40",		"LoadTest",					"Load",		"Exec",
	"FlushCache",		"InstallInterruptHandler",	"GPU_dw",	"mem2vram",
	"SendGPUStatus",	"GPU_cw",					"GPU_cwb",	"SendPackets",
	"sys_a0_4c",		"GetGPUStatus",				"GPU_sync",	"sys_a0_4f",
// 0x50
	"sys_a0_50",		"LoadExec",				"GetSysSp",		"sys_a0_53",
	"_96_init()",		"_bu_init()",			"_96_remove()",	"sys_a0_57",
	"sys_a0_58",		"sys_a0_59",			"sys_a0_5a",	"dev_tty_init",
	"dev_tty_open",		"sys_a0_5d",			"dev_tty_ioctl","dev_cd_open",
// 0x60
	"dev_cd_read",		"dev_cd_close",			"dev_cd_firstfile",	"dev_cd_nextfile",
	"dev_cd_chdir",		"dev_card_open",		"dev_card_read",	"dev_card_write",
	"dev_card_close",	"dev_card_firstfile",	"dev_card_nextfile","dev_card_erase",
	"dev_card_undelete","dev_card_format",		"dev_card_rename",	"dev_card_6f",
// 0x70
	"_bu_init",			"_96_init",		"_96_remove",		"sys_a0_73",
	"sys_a0_74",		"sys_a0_75",	"sys_a0_76",		"sys_a0_77",
	"_96_CdSeekL",		"sys_a0_79",	"sys_a0_7a",		"sys_a0_7b",
	"_96_CdGetStatus",	"sys_a0_7d",	"_96_CdRead",		"sys_a0_7f",
// 0x80
	"sys_a0_80",		"sys_a0_81",	"sys_a0_82",		"sys_a0_83",
	"sys_a0_84",		"_96_CdStop",	"sys_a0_86",		"sys_a0_87",
	"sys_a0_88",		"sys_a0_89",	"sys_a0_8a",		"sys_a0_8b",
	"sys_a0_8c",		"sys_a0_8d",	"sys_a0_8e",		"sys_a0_8f",
// 0x90
	"sys_a0_90",		"sys_a0_91",	"sys_a0_92",		"sys_a0_93",
	"sys_a0_94",		"sys_a0_95",	"AddCDROMDevice",	"AddMemCardDevide",
	"DisableKernelIORedirection",		"EnableKernelIORedirection", "sys_a0_9a", "sys_a0_9b",
	"SetConf",			"GetConf",		"sys_a0_9e",		"SetMem",
// 0xa0
	"_boot",			"SystemError",	"EnqueueCdIntr",	"DequeueCdIntr",
	"sys_a0_a4",		"ReadSector",	"get_cd_status",	"bufs_cb_0",
	"bufs_cb_1",		"bufs_cb_2",	"bufs_cb_3",		"_card_info",
	"_card_load",		"_card_auto",	"bufs_cd_4",		"sys_a0_af",
// 0xb0
	"sys_a0_b0",		"sys_a0_b1",	"do_a_long_jmp",	"sys_a0_b3",
	"?? sub_function",
};

char *biosB0n[256] = {
// 0x00
	"SysMalloc",		"sys_b0_01",	"sys_b0_02",	"sys_b0_03",
	"sys_b0_04",		"sys_b0_05",	"sys_b0_06",	"DeliverEvent",
	"OpenEvent",		"CloseEvent",	"WaitEvent",	"TestEvent",
	"EnableEvent",		"DisableEvent",	"OpenTh",		"CloseTh",
// 0x10
	"ChangeTh",			"sys_b0_11",	"InitPAD",		"StartPAD",
	"StopPAD",			"PAD_init",		"PAD_dr",		"ReturnFromExecption",
	"ResetEntryInt",	"HookEntryInt",	"sys_b0_1a",	"sys_b0_1b",
	"sys_b0_1c",		"sys_b0_1d",	"sys_b0_1e",	"sys_b0_1f",
// 0x20
	"UnDeliverEvent",	"sys_b0_21",	"sys_b0_22",	"sys_b0_23",
	"sys_b0_24",		"sys_b0_25",	"sys_b0_26",	"sys_b0_27",
	"sys_b0_28",		"sys_b0_29",	"sys_b0_2a",	"sys_b0_2b",
	"sys_b0_2c",		"sys_b0_2d",	"sys_b0_2e",	"sys_b0_2f",
// 0x30
	"sys_b0_30",		"sys_b0_31",	"open",			"lseek",
	"read",				"write",		"close",		"ioctl",
	"exit",				"sys_b0_39",	"getc",			"putc",
	"getchar",			"putchar",		"gets",			"puts",
// 0x40
	"cd",				"format",		"firstfile",	"nextfile",
	"rename",			"delete",		"undelete",		"AddDevice",
	"RemoteDevice",		"PrintInstalledDevices", "InitCARD", "StartCARD",
	"StopCARD",			"sys_b0_4d",	"_card_write",	"_card_read",
// 0x50
	"_new_card",		"Krom2RawAdd",	"sys_b0_52",	"sys_b0_53",
	"_get_errno",		"_get_error",	"GetC0Table",	"GetB0Table",
	"_card_chan",		"sys_b0_59",	"sys_b0_5a",	"ChangeClearPAD",
	"_card_status",		"_card_wait",
};

char *biosC0n[256] = {
// 0x00
	"InitRCnt",			  "InitException",		"SysEnqIntRP",		"SysDeqIntRP",
	"get_free_EvCB_slot", "get_free_TCB_slot",	"ExceptionHandler",	"InstallExeptionHandler",
	"SysInitMemory",	  "SysInitKMem",		"ChangeClearRCnt",	"SystemError",
	"InitDefInt",		  "sys_c0_0d",			"sys_c0_0e",		"sys_c0_0f",
// 0x10
	"sys_c0_10",		  "sys_c0_11",			"InstallDevices",	"FlushStfInOutPut",
	"sys_c0_14",		  "_cdevinput",			"_cdevscan",		"_circgetc",
	"_circputc",		  "ioabort",			"sys_c0_1a",		"KernelRedirect",
	"PatchAOTable",
};

//#define r0 (psxRegs.GPR.n.r0)
#define at (psxRegs.GPR.n.at)
#define v0 (psxRegs.GPR.n.v0)
#define v1 (psxRegs.GPR.n.v1)
#define a0 (psxRegs.GPR.n.a0)
#define a1 (psxRegs.GPR.n.a1)
#define a2 (psxRegs.GPR.n.a2)
#define a3 (psxRegs.GPR.n.a3)
#define t0 (psxRegs.GPR.n.t0)
#define t1 (psxRegs.GPR.n.t1)
#define t2 (psxRegs.GPR.n.t2)
#define t3 (psxRegs.GPR.n.t3)
#define t4 (psxRegs.GPR.n.t4)
#define t5 (psxRegs.GPR.n.t5)
#define t6 (psxRegs.GPR.n.t6)
#define t7 (psxRegs.GPR.n.t7)
#define s0 (psxRegs.GPR.n.s0)
#define s1 (psxRegs.GPR.n.s1)
#define s2 (psxRegs.GPR.n.s2)
#define s3 (psxRegs.GPR.n.s3)
#define s4 (psxRegs.GPR.n.s4)
#define s5 (psxRegs.GPR.n.s5)
#define s6 (psxRegs.GPR.n.s6)
#define s7 (psxRegs.GPR.n.s7)
#define t8 (psxRegs.GPR.n.t6)
#define t9 (psxRegs.GPR.n.t7)
#define k0 (psxRegs.GPR.n.k0)
#define k1 (psxRegs.GPR.n.k1)
#define gp (psxRegs.GPR.n.gp)
#define sp (psxRegs.GPR.n.sp)
#define fp (psxRegs.GPR.n.s8)
#define ra (psxRegs.GPR.n.ra)
#define pc0 (psxRegs.pc)

#define Ra0 ((char*)PSXM(a0))
#define Ra1 ((char*)PSXM(a1))
#define Ra2 ((char*)PSXM(a2))
#define Ra3 ((char*)PSXM(a3))
#define Rv0 ((char*)PSXM(v0))
#define Rsp ((char*)PSXM(sp))



typedef struct _malloc_chunk {
	unsigned long stat;
	unsigned long size;
	struct _malloc_chunk *fd;
	struct _malloc_chunk *bk;
} malloc_chunk;

#define INUSE 0x1

typedef struct {
	u32 desc;
	s32 status;
	s32 mode;
	u32 fhandler;
} EvCB[32];

#define EvStUNUSED	0x0000
#define EvStWAIT	0x1000
#define EvStACTIVE	0x2000
#define EvStALREADY 0x4000

#define EvMdINTR	0x1000
#define EvMdNOINTR	0x2000

typedef struct {
	long next;
	long func1;
	long func2;
	long pad;
} SysRPst;

typedef struct {
	s32 status;
	s32 mode;
	u32 reg[32];
	u32 func;
} TCB;

typedef struct {                   
	unsigned long _pc0;      
	unsigned long gp0;      
	unsigned long t_addr;   
	unsigned long t_size;   
	unsigned long d_addr;   
	unsigned long d_size;   
	unsigned long b_addr;   
	unsigned long b_size;   
	unsigned long S_addr;
	unsigned long s_size;
	unsigned long _sp,_fp,_gp,ret,base;
} EXEC;

struct DIRENTRY {
	char name[20];
	long attr;
	long size;
	struct DIRENTRY *next;
	long head;
	char system[4];
};

typedef struct {
	char name[32];
	u32  mode;
	u32  offset;
	u32  size;
	u32  mcfile;
} FileDesc;

static unsigned long *jmp_int = NULL;
static int *pad_buf = NULL;
static char *pad_buf1,*pad_buf2;//shadow add
static int pad_buf1len,pad_buf2len;//shadow add


static u32 regs[35];
static EvCB *Event;
static EvCB *HwEV; // 0xf0
static EvCB *EvEV; // 0xf1
static EvCB *RcEV; // 0xf2
static EvCB *UeEV; // 0xf3
static EvCB *SwEV; // 0xf4
static EvCB *ThEV; // 0xff
static u32 *heap_addr = NULL;
static u32 SysIntRP[8];
static int CardState = -1;
static TCB Thread[8];
static int CurThread = 0;
static FileDesc FDesc[32];

static __inline void softCall(u32 pc) {
	pc0 = pc;
	ra = 0x80001000;
	while (pc0 != 0x80001000) psxCpu->ExecuteBlock();
}

static __inline void softCall2(u32 pc) {
	u32 sra = ra;
	pc0 = pc;
	ra = 0x80001000;
	while (pc0 != 0x80001000) psxCpu->ExecuteBlock();
	ra = sra;
}

static __inline void DeliverEvent(u32 ev, u32 spec) {
	if (Event[ev][spec].status != EvStACTIVE) return;

//	Event[ev][spec].status = EvStALREADY;
	if (Event[ev][spec].mode == EvMdINTR) {
		softCall2(Event[ev][spec].fhandler);
	} else Event[ev][spec].status = EvStALREADY;
}

/*                                           *
//                                           *
//                                           *
//               System calls A0             */


void bios_abs() { // 0x0e
	v0 = abs(a0);
	pc0 = ra;
}

void bios_labs() { // 0x0f
	v0 = labs(a0);
	pc0 = ra;
}

void bios_atoi() { // 0x10
	v0 = atoi((char *)Ra0);
	pc0 = ra;
}

void bios_atol() { // 0x11
	v0 = atoi((char *)Ra0);
	pc0 = ra;
}

void bios_setjmp() { // 13
	u32 *jmp_buf= (u32*)Ra0;
	int i;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x13]);
#endif

	jmp_buf[0] = ra;
	jmp_buf[1] = sp;
	jmp_buf[2] = fp;
	for (i=0; i<8; i++) // s0-s7
		jmp_buf[3+i] = psxRegs.GPR.r[16+i];
	jmp_buf[11] = gp;

	v0 = 0; pc0 = ra;
}

void bios_longjmp() { //14
	u32 *jmp_buf= (u32*)Ra0;
	int i;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x14]);
#endif

	ra = jmp_buf[0]; /* ra */
	sp = jmp_buf[1]; /* sp */
	fp = jmp_buf[2]; /* fp */
	for (i=0; i<8; i++) // s0-s7
	   psxRegs.GPR.r[16+i] = jmp_buf[3+i];		
	gp = jmp_buf[11]; /* gp */

	v0 = a1; pc0 = ra;
}

void bios_strcat() { // 0x15
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s, %s\n", biosA0n[0x15], Ra0, Ra1);
#endif

	strcat(Ra0, Ra1);
	v0 = a0; pc0 = ra;
}

/*0x16*/void bios_strncat() { strncat(Ra0, Ra1, a2); v0 = a0; pc0 = ra;}

void bios_strcmp() { // 0x17
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s (%lx), %s (%lx)\n", biosA0n[0x17], Ra0, a0, Ra1, a1);
#endif

	v0 = strcmp(Ra0, Ra1);
	pc0 = ra;
}

void bios_strncmp() { // 0x18
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s (%lx), %s (%lx), %d\n", biosA0n[0x18], Ra0, a0, Ra1, a1, a2);
#endif

	v0 = strncmp(Ra0, Ra1, a2);
	pc0 = ra;
}

/*0x19*/void bios_strcpy()  { strcpy(Ra0, Ra1); v0 = a0; pc0 = ra;}
/*0x1a*/void bios_strncpy() { strncpy(Ra0, Ra1, a2);  v0 = a0; pc0 = ra;}
/*0x1b*/void bios_strlen()  { v0 = strlen(Ra0); pc0 = ra;}

void bios_index() { // 0x1c
	char *pcA0 = (char *)Ra0; 
	char *pcRet = strchr(pcA0, a1); 
	if(pcRet) 
		v0 = a0 + pcRet - pcA0; 
	else 
		v0 = 0;
    pc0 = ra;
}

void bios_rindex() { // 0x1d
	char *pcA0 = (char *)Ra0; 
	char *pcRet = strrchr(pcA0, a1); 
	if(pcRet) 
		v0 = a0 + pcRet - pcA0; 
	else 
		v0 = 0;
    pc0 = ra;  
}

void bios_strchr() { // 0x1e
	char *pcA0 = (char *)Ra0; 
	char *pcRet = strchr(pcA0, a1); 
	if(pcRet) 
		v0 = a0 + pcRet - pcA0; 
	else 
		v0 = 0;
    pc0 = ra;
}

void bios_strrchr() { // 0x1f
	char *pcA0 = (char *)Ra0; 
	char *pcRet = strrchr(pcA0, a1); 
	if(pcRet) 
		v0 = a0 + pcRet - pcA0; 
	else 
		v0 = 0;
    pc0 = ra;
}

void bios_strpbrk() { // 0x20
	char *pcA0 = (char *)Ra0; 
	char *pcRet = strpbrk(pcA0, (char *)Ra1); 
	if(pcRet) 
		v0 = a0 + pcRet - pcA0; 
	else 
		v0 = 0;
    pc0 = ra;
}

void bios_strspn()  { v0 = strspn ((char *)Ra0, (char *)Ra1); pc0 = ra;}/*21*/ 
void bios_strcspn() { v0 = strcspn((char *)Ra0, (char *)Ra1); pc0 = ra;}/*22*/ 

void bios_strtok() { // 0x23
	char *pcA0 = (char *)Ra0;
	char *pcRet = strtok(pcA0, (char *)Ra1);
	if(pcRet)
		v0 = a0 + pcRet - pcA0;
	else
		v0 = 0;
    pc0 = ra;
}

void bios_strstr() { // 0x24
	char *pcA0 = (char *)Ra0;
	char *pcRet = strstr(pcA0, (char *)Ra1);
	if(pcRet)
		v0 = a0 + pcRet - pcA0;
	else
		v0 = 0;
    pc0 = ra;
}

/*0x25*/void bios_toupper() {v0 = toupper(a0); pc0 = ra;}
/*0x26*/void bios_tolower() {v0 = tolower(a0); pc0 = ra;}
/*0x27*/void bios_bcopy()   {memcpy(Ra1,Ra0,a2); pc0=ra;}
/*0x28*/void bios_bzero()   {memset(Ra0,0,a1); pc0=ra;}
/*0x29*/void bios_bcmp()    {v0 = memcmp(Ra0,Ra1,a2); pc0=ra; }
/*0x2a*/void bios_memcpy()  {memcpy(Ra0, Ra1, a2); v0 = a0; pc0 = ra;}
/*0x2b*/void bios_memset()  {memset(Ra0, a1, a2); v0 = a0; pc0 = ra;}
/*0x2c*/void bios_memmove() {memmove(Ra0, Ra1, a2); v0 = a0; pc0 = ra;}
/*0x2d*/void bios_memcmp()  {v0 = memcmp(Ra0, Ra1, a2); pc0 = ra;}  

void bios_memchr() { // 2e
	void *ret = memchr(Ra0, a1, a2);
	if (ret != NULL) v0 = (unsigned long)((char*)ret - Ra0) + a0;
	else v0 = 0;
	pc0 = ra;
}

void bios_rand() { // 2f
	v0 = 1+(int) (32767.0*rand()/(RAND_MAX+1.0));
	pc0 = ra;
}

void bios_srand() { // 30
	srand(a0); pc0 = ra;
}

void bios_malloc() { // 33
	malloc_chunk *chunk;
	malloc_chunk *fd;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x33]);
#endif

	chunk = (malloc_chunk *)heap_addr;
	if (chunk == NULL) { v0 = 0; return; }

	for (; ((a0 > chunk->size) || (chunk->stat == INUSE)) || (chunk->fd != NULL); chunk = chunk->fd);
//printf ("chunk %lx\n",chunk->size);
	/* split free chunk */
	fd = chunk + sizeof(malloc_chunk) + a0;
	fd->stat = chunk->stat;
	fd->size = chunk->size - a0;
	fd->fd = chunk->fd;
	fd->bk = chunk;

	/* set new chunk */
	chunk->stat = INUSE;
	chunk->size = a0;
	chunk->fd = fd;

	v0 = ((unsigned long)chunk - (unsigned long)psxM) + sizeof(malloc_chunk);
	v0|= 0x80000000;
//	printf ("malloc %lx,%lx\n", v0, a0);
	pc0 = ra;
}

void bios_InitHeap() { // 39
	malloc_chunk *chunk;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x39]);
#endif

	heap_addr = (u32*)Ra0;

	chunk = (malloc_chunk *)heap_addr;
	chunk->stat = 0;
	if (((a0 & 0x1fffff) + a1)>= 0x200000) chunk->size = 0x1ffffc - (a0 & 0x1fffff);
	else chunk->size = a1;
	chunk->fd = NULL;
	chunk->bk = NULL;

	SysPrintf("InitHeap %lx,%lx : %lx\n",a0,a1,chunk->size);

	pc0 = ra;
}

void bios_getchar(){ v0 = getchar(); pc0=ra;} //0x3b

void bios_printf() { // 3f
	char tmp[1024];
	char tmp2[1024];
	unsigned long save[4];
	char *ptmp = tmp;
	int n=1, i=0, j;

	memcpy(save, (char*)PSXM(sp), 4*4);
	psxMu32(sp) = a0;
	psxMu32(sp + 4) = a1;
	psxMu32(sp + 8) = a2;
	psxMu32(sp + 12) = a3;

	while (Ra0[i]) {
		switch (Ra0[i]) {
			case '%':
				j = 0;
				tmp2[j++] = '%';
_start:
				switch (Ra0[++i]) {
					case '.':
					case 'l':
						tmp2[j++] = Ra0[i]; goto _start;
					default:
						if (Ra0[i] >= '0' && Ra0[i] <= '9') {
							tmp2[j++] = Ra0[i];
							goto _start;
						}
						break;
				}
				tmp2[j++] = Ra0[i];
				tmp2[j] = 0;

				switch (Ra0[i]) {
					case 'f': case 'F':
						ptmp+= sprintf(ptmp, tmp2, (float)psxMu32(sp + n * 4)); n++; break;
					case 'a': case 'A':
					case 'e': case 'E':
					case 'g': case 'G':
						ptmp+= sprintf(ptmp, tmp2, (double)psxMu32(sp + n * 4)); n++; break;
					case 'p':
					case 'i':
					case 'd': case 'D':
					case 'o': case 'O':
					case 'x': case 'X':
						ptmp+= sprintf(ptmp, tmp2, (unsigned int)psxMu32(sp + n * 4)); n++; break;
					case 'c':
						ptmp+= sprintf(ptmp, tmp2, (unsigned char)psxMu32(sp + n * 4)); n++; break;
					case 's':
						ptmp+= sprintf(ptmp, tmp2, (char*)PSXM(psxMu32(sp + n * 4))); n++; break;
					case '%':
						*ptmp++ = Ra0[i]; break;
				}
				i++;
				break;
			default:
				*ptmp++ = Ra0[i++];
		}
	}
	*ptmp = 0;

	memcpy((char*)PSXM(sp), save, 4*4);

	SysPrintf(tmp);

	pc0 = ra;
}

/*
 *	long Load(char *name, struct EXEC *header);
 */

void bios_Load() { // 42
/*
	EXE_HEADER eheader;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s, %x\n", biosA0n[0x42], Ra0, a1);
#endif

	if (LoadCdromFile(Ra0, &eheader) == 0) {
		memcpy(Ra1, &eheader+16, sizeof(EXEC));
		v0 = 1;
	} else v0 = 0;
*/
	pc0 = ra;
}

/*
 *	int Exec(struct EXEC *header , int argc , char **argv);
 */

void bios_Exec() { // 43
	EXEC *header = (EXEC*)Ra0;
	u32 tmp;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x, %x, %x\n", biosA0n[0x43], a0, a1, a2);
#endif

	header->_sp = sp;
	header->_fp = fp;
	header->_sp = sp;
	header->_gp = gp;
	header->ret = ra;
	header->base = s0;

	if (header->S_addr != 0) {
		tmp = header->S_addr + header->s_size;
		sp = tmp;
		fp = sp;
	}

	gp = header->gp0;

	s0 = a0;

	a0 = a1;
	a1 = a2;

	ra = 0x8000;
	pc0 = header->_pc0;
}

void bios_FlushCache() { // 44
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x44]);
#endif

	pc0 = ra;
}

void bios_GPU_dw() { // 0x46
	int size;
	long *ptr;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x46]);
#endif

	GPU_writeData(0xa0000000);
	GPU_writeData((a1<<16)|(a0&0xffff));
	GPU_writeData((a3<<16)|(a2&0xffff));
	size = (a2*a3+1)/2;
	ptr = (long*)PSXM(Rsp[4]);  //that is correct?
	do {
		GPU_writeData(*ptr++);
	} while(--size);

	pc0 = ra;
}  

void bios_mem2vram() { // 0x47
	int size;

	GPU_writeData(0xa0000000);
	GPU_writeData((a1<<16)|(a0&0xffff));
	GPU_writeData((a3<<16)|(a2&0xffff));
	size = (a2*a3+1)/2;
	GPU_writeStatus(0x04000002);
	psxHwWrite32(0x1f8010f4,0);
	psxHwWrite32(0x1f8010f0,psxHwRead32(0x1f8010f0)|0x800);
	psxHwWrite32(0x1f8010a0,Rsp[4]);//might have a buggy...
	psxHwWrite32(0x1f8010a4,((size/16)<<16)|16);
	psxHwWrite32(0x1f8010a8,0x01000201);

	pc0 = ra;
}

void bios_SendGPU() { // 0x48
	GPU_writeStatus(a0);
	pc0 = ra;
}

void bios_GPU_cw() { // 0x49
	GPU_writeData(a0);
	pc0 = ra;
}

void bios_GPU_cwb() { // 0x4a
	long *ptr = (long*)Ra0;
	int size = a1;
	while(size--) {
		GPU_writeData(*ptr++);
	}

	pc0 = ra;
}
   
void bios_GPU_SendPackets() { //4b:	
	GPU_writeStatus(0x04000002);
	psxHwWrite32(0x1f8010f4,0);
	psxHwWrite32(0x1f8010f0,psxHwRead32(0x1f8010f0)|0x800);
	psxHwWrite32(0x1f8010a0,a0);
	psxHwWrite32(0x1f8010a4,0);
	psxHwWrite32(0x1f8010a8,0x010000401);
	pc0 = ra;
}

void bios_sys_a0_4c() { // 0x4c GPU relate
	psxHwWrite32(0x1f8010a8,0x00000401);
	GPU_writeData(0x0400000);
	GPU_writeData(0x0200000);
	GPU_writeData(0x0100000);

	pc0 = ra;
}

void bios_GPU_GetGPUStatus() { // 0x4d
	v0 = GPU_readStatus();
	pc0 = ra;
}

#undef s_addr

void bios_LoadExec() { // 51
	EXEC *header = (EXEC*)PSXM(0xf000);
	u32 s_addr, s_size;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s: %x,%x\n", biosA0n[0x51], Ra0, a1, a2);
#endif
	s_addr = a1; s_size = a2;

	a1 = 0xf000;	
	bios_Load();

	header->S_addr = s_addr;
	header->s_size = s_size;

	bios_Exec();
}

void bios__bu_init() { // 70
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x70]);
#endif

	DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
	DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004

	pc0 = ra;
}

void bios__96_init() { // 71
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x71]);
#endif

	pc0 = ra;
}

void bios__96_remove() { // 72
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosA0n[0x72]);
#endif

	pc0 = ra;
}

void bios__card_info() { // ab
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosA0n[0xab], a0);
#endif

//	DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
	DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004

	v0 = 1; pc0 = ra;
}

void bios__card_load() { // ac
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosA0n[0xac], a0);
#endif

//	DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
	DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004

	v0 = 1; pc0 = ra;
}

/* System calls B0 */

void bios_SetRCnt() { // 02
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x02]);
#endif

	a0&= 0x3;
	if (a0 != 3) {
		unsigned long mode=0;

		psxRcntWtarget(a0, a1);
		if (a2&0x1000) mode|= 0x050; // Interrupt Mode
		if (a2&0x0100) mode|= 0x008; // Count to 0xffff
		if (a2&0x0010) mode|= 0x001; // Timer stop mode
		if (a0 == 2) { if (a2&0x0001) mode|= 0x200; } // System Clock mode
		else         { if (a2&0x0001) mode|= 0x100; } // System Clock mode

		psxRcntWmode(a0, mode);
	}
	pc0 = ra;
}

void bios_GetRCnt() { // 03
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x03]);
#endif

	a0&= 0x3;
	if (a0 != 3) v0 = psxRcntRcount(a0);
	else v0 = 0;
	pc0 = ra;
}

void bios_StartRCnt() { // 04
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x04]);
#endif

	a0&= 0x3;
	if (a0 != 3) psxHu32(0x1074)|= (1<<(a0+4));
	else psxHu32(0x1074)|= 0x1;
	v0 = 1; pc0 = ra;
}

void bios_StopRCnt() { // 05
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x05]);
#endif

	a0&= 0x3;
	if (a0 != 3) psxHu32(0x1074)&= ~(1<<(a0+4));
	else psxHu32(0x1074)&= ~0x1;
	pc0 = ra;
}

void bios_ResetRCnt() { // 06
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x06]);
#endif

	a0&= 0x3;
	if (a0 != 3) {
		psxRcntWmode(a0, 0);
		psxRcntWtarget(a0, 0);
		psxRcntWcount(a0, 0);
	}
	pc0 = ra;
}


/* gets ev for use with Event */
#define GetEv() \
	ev = (a0 >> 24) & 0xf; \
	if (ev == 0xf) ev = 0x5; \
	ev*= 32; \
	ev+= a0&0x1f;

/* gets spec for use with Event */
#define GetSpec() \
	spec = 0; \
	switch (a1) { \
		case 0x0301: spec = 16; break; \
		case 0x0302: spec = 17; break; \
		default: \
			for (i=0; i<16; i++) if (a1 & (1 << i)) { spec = i; break; } \
			break; \
	}

void bios_DeliverEvent() { // 07
	int ev, spec;
	int i;

	GetEv();
	GetSpec();

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x07], ev, spec);
#endif

	DeliverEvent(ev, spec);

	pc0 = ra;
}

void bios_OpenEvent() { // 08
	int ev, spec;
	int i;

	GetEv();
	GetSpec();

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x (class:%lx, spec:%lx, mode:%lx, func:%lx)\n", biosB0n[0x08], ev, spec, a0, a1, a2, a3);
#endif

	Event[ev][spec].status = EvStWAIT;
	Event[ev][spec].mode = a2;
	Event[ev][spec].fhandler = a3;

	v0 = ev | (spec << 8);
	pc0 = ra;
}

void bios_CloseEvent() { // 09
	int ev, spec;

	ev   = a0 & 0xff;
	spec = (a0 >> 8) & 0xff;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x09], ev, spec);
#endif

	Event[ev][spec].status = EvStUNUSED;

	v0 = 1; pc0 = ra;
}

void bios_WaitEvent() { // 0a
	int ev, spec;

	ev   = a0 & 0xff;
	spec = (a0 >> 8) & 0xff;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x0a], ev, spec);
#endif

	Event[ev][spec].status = EvStACTIVE;

	v0 = 1; pc0 = ra;
}

void bios_TestEvent() { // 0b
	int ev, spec;

	ev   = a0 & 0xff;
	spec = (a0 >> 8) & 0xff;

	if (Event[ev][spec].status == EvStALREADY) {
		Event[ev][spec].status = EvStACTIVE; v0 = 1;
	} else v0 = 0;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x: %x\n", biosB0n[0x0b], ev, spec, v0);
#endif

	pc0 = ra;
}

void bios_EnableEvent() { // 0c
	int ev, spec;

	ev   = a0 & 0xff;
	spec = (a0 >> 8) & 0xff;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x0c], ev, spec);
#endif

	Event[ev][spec].status = EvStACTIVE;

	v0 = 1; pc0 = ra;
}

void bios_DisableEvent() { // 0d
	int ev, spec;

	ev   = a0 & 0xff;
	spec = (a0 >> 8) & 0xff;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x0d], ev, spec);
#endif

	Event[ev][spec].status = EvStWAIT;

	v0 = 1; pc0 = ra;
}

/*
 *	long OpenTh(long (*func)(), unsigned long sp, unsigned long gp);
 */

void bios_OpenTh() { // 0e
	int th;

	for (th=1; th<8; th++)
		if (Thread[th].status == 0) break;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x0e], th);
#endif

	Thread[th].status = 1;
	Thread[th].func    = a0;
	Thread[th].reg[29] = a1;
	Thread[th].reg[28] = a2;

	v0 = th; pc0 = ra;
}

/*
 *	int CloseTh(long thread);
 */

void bios_CloseTh() { // 0f
	int th = a0 & 0xff;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x0f], th);
#endif

	if (Thread[th].status == 0) {
		v0 = 0;
	} else {
		Thread[th].status = 0;
		v0 = 1;
	}

	pc0 = ra;
}

/*
 *	int ChangeTh(long thread);
 */

void bios_ChangeTh() { // 10
	int th = a0 & 0xff;

#ifdef PSXBIOS_LOG
//	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x10], th);
#endif

	if (Thread[th].status == 0 || CurThread == th) {
		v0 = 0;

		pc0 = ra;
	} else {
		v0 = 1;

		if (Thread[CurThread].status == 2) {
			Thread[CurThread].status = 1;
			Thread[CurThread].func = ra;
			memcpy(Thread[CurThread].reg, psxRegs.GPR.r, 32*4);
		}

		memcpy(psxRegs.GPR.r, Thread[th].reg, 32*4);
		pc0 = Thread[th].func;
		Thread[th].status = 2;
		CurThread = th;
	}
}

void bios_InitPAD() { // 0x12
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x12]);
#endif

	pad_buf1 = (char*)Ra0;
	pad_buf1len = a1;
	pad_buf2 = (char*)Ra2;
	pad_buf2len = a3;

	v0 = 1; pc0 = ra;
}

void bios_StartPAD() { // 13
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x13]);
#endif

	psxHwWrite16(0x1f801074, (unsigned short)(psxHwRead16(0x1f801074) | 0x1));
	psxRegs.CP0.n.Status |= 0x401;
	pc0 = ra;
}

void bios_StopPAD() { // 14
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x14]);
#endif

	pad_buf1 = NULL;
	pad_buf2 = NULL;
	pc0 = ra;
}

void bios_PAD_init() { // 15
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x15]);
#endif

	psxHwWrite16(0x1f801074, (u16)(psxHwRead16(0x1f801074) | 0x1));
	pad_buf = (int*)Ra1;
	*pad_buf = -1;
	psxRegs.CP0.n.Status |= 0x401;
	pc0 = ra;
}

void bios_PAD_dr() { // 16
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x16]);
#endif

	v0 = -1; pc0 = ra;
}

void bios_ReturnFromException() { // 17
	memcpy(psxRegs.GPR.r, regs, 32*4);
	psxRegs.GPR.n.lo = regs[32];
	psxRegs.GPR.n.hi = regs[33];

	pc0 = psxRegs.CP0.n.EPC;
	if (psxRegs.CP0.n.Cause & 0x80000000) pc0+=4;

	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & 0xfffffff0) |
						  ((psxRegs.CP0.n.Status & 0x3c) >> 2);
}

void bios_ResetEntryInt() { // 18
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x18]);
#endif

	jmp_int = NULL;
	pc0 = ra;
}

void bios_HookEntryInt() { // 19
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x19]);
#endif

	jmp_int = (u32*)Ra0;
	pc0 = ra;
}

void bios_UnDeliverEvent() { // 0x20
	int ev, spec;
	int i;

	GetEv();
	GetSpec();

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s %x,%x\n", biosB0n[0x07], ev, spec);
#endif

	if (Event[ev][spec].status == EvStALREADY &&
		Event[ev][spec].mode == EvMdNOINTR)
		Event[ev][spec].status = EvStACTIVE;

	pc0 = ra;
}

#define buopen(mcd) { \
	strcpy(FDesc[1 + mcd].name, Ra0+5); \
	FDesc[1 + mcd].offset = 0; \
	FDesc[1 + mcd].mode   = a1; \
 \
	for (i=1; i<16; i++) { \
		ptr = Mcd##mcd##Data + 128 * i; \
		if ((*ptr & 0xF0) != 0x50) continue; \
		if (strcmp(FDesc[1 + mcd].name, ptr+0xa)) continue; \
		FDesc[1 + mcd].mcfile = i; \
		SysPrintf("open %s\n", ptr+0xa); \
		v0 = 1 + mcd; \
		break; \
	} \
	if (a1 & 0x200 && v0 == -1) { /* FCREAT */ \
		for (i=1; i<16; i++) { \
			int j, xor = 0; \
 \
			ptr = Mcd##mcd##Data + 128 * i; \
			if ((*ptr & 0xF0) == 0x50) continue; \
			ptr[0] = 0x50 | (u8)(a1 >> 16); \
			ptr[4] = 0x00; \
			ptr[5] = 0x20; \
			ptr[6] = 0x00; \
			ptr[7] = 0x00; \
			ptr[8] = 'B'; \
			ptr[9] = 'I'; \
			strcpy(ptr+0xa, FDesc[1 + mcd].name); \
			for (j=0; j<127; j++) xor^= ptr[j]; \
			ptr[127] = xor; \
			FDesc[1 + mcd].mcfile = i; \
			SysPrintf("openC %s\n", ptr); \
			v0 = 1 + mcd; \
			SaveMcd(Config.Mcd##mcd, Mcd##mcd##Data, 128 * i, 128); \
			break; \
		} \
	} \
}

/*
 *	int open(char *name , int mode);
 */

void bios_open() { // 0x32
	int i;
	char *ptr;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s,%x\n", biosB0n[0x32], Ra0, a1);
#endif

	v0 = -1;

	if (!strncmp(Ra0, "bu00", 4)) {
		buopen(1);
	}

	if (!strncmp(Ra0, "bu10", 4)) {
		buopen(2);
	}

	pc0 = ra;
}

/*
 *	int lseek(int fd , int offset , int whence);
 */

void bios_lseek() { // 0x33
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x, %x, %x\n", biosB0n[0x33], a0, a1, a2);
#endif

	switch (a2) {
		case 0: // SEEK_SET
			FDesc[a0].offset = a1;
			v0 = a1;
//			DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
//			DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004
			break;

		case 1: // SEEK_CUR
			FDesc[a0].offset+= a1;
			v0 = FDesc[a0].offset;
			break;
	}

	pc0 = ra;
}

#define buread(mcd) { \
	SysPrintf("read %d: %x,%x (%s)\n", FDesc[1 + mcd].mcfile, FDesc[1 + mcd].offset, a2, Mcd##mcd##Data + 128 * FDesc[1 + mcd].mcfile + 0xa); \
	ptr = Mcd##mcd##Data + 8192 * FDesc[1 + mcd].mcfile + FDesc[1 + mcd].offset; \
	memcpy(Ra1, ptr, a2); \
	if (FDesc[1 + mcd].mode & 0x8000) v0 = 0; \
	else v0 = a2; \
	DeliverEvent(0x11, 0x2); /* 0xf0000011, 0x0004 */ \
	DeliverEvent(0x81, 0x2); /* 0xf4000001, 0x0004 */ \
}

/*
 *	int read(int fd , void *buf , int nbytes);
 */

void bios_read() { // 0x34
	char *ptr;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x, %x, %x\n", biosB0n[0x34], a0, a1, a2);
#endif

	v0 = -1;

	switch (a0) {
		case 2: buread(1); break;
		case 3: buread(2); break;
	}
  		
	pc0 = ra;
}

#define buwrite(mcd) { \
	u32 offset =  + 8192 * FDesc[1 + mcd].mcfile + FDesc[1 + mcd].offset; \
	SysPrintf("write %d: %x,%x\n", FDesc[1 + mcd].mcfile, FDesc[1 + mcd].offset, a2); \
	ptr = Mcd##mcd##Data + offset; \
	memcpy(ptr, Ra1, a2); \
	SaveMcd(Config.Mcd##mcd, Mcd##mcd##Data, offset, a2); \
	if (FDesc[1 + mcd].mode & 0x8000) v0 = 0; \
	else v0 = a2; \
	DeliverEvent(0x11, 0x2); /* 0xf0000011, 0x0004 */ \
	DeliverEvent(0x81, 0x2); /* 0xf4000001, 0x0004 */ \
}

/*
 *	int write(int fd , void *buf , int nbytes);
 */

void bios_write() { // 0x35/0x03
	char *ptr;

    if (a0 == 1) { // stdout
		char *ptr = Ra1;

		while (a2 > 0) {
			SysPrintf("%c", *ptr++); a2--;
		}
		pc0 = ra; return;
    }
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x,%x,%x\n", biosB0n[0x35], a0, a1, a2);
#endif

	v0 = -1;

	switch (a0) {
		case 2: buwrite(1); break;
		case 3: buwrite(2); break;
	}
  		
	pc0 = ra;
}

/*
 *	int close(int fd);
 */

void bios_close() { // 0x36
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x36], a0);
#endif
	
	v0 = a0; pc0 = ra;
}

void bios_putchar () { // 3d
    char tmp[12];

    sprintf (tmp,"%c",(char)a0);
    SysPrintf(tmp);

    pc0 = ra;
}

void bios_puts () { // 3e/3f
    SysPrintf(Ra0);

    pc0 = ra;
}

char ffile[64], *pfile;
int nfile;

#define bufile(mcd) { \
	while (nfile < 16) { \
		int match=1; \
 \
		ptr = Mcd##mcd##Data + 128 * nfile; \
		nfile++; \
		if ((*ptr & 0xF0) != 0x50) continue; \
		ptr+= 0xa; \
		for (i=0; i<20; i++) { \
			if (pfile[i] == ptr[i]) { \
				dir->name[i] = ptr[i]; \
				if (ptr[i] == 0) break; else continue; } \
			if (pfile[i] == '?') { \
				dir->name[i] = ptr[i]; continue; } \
			if (pfile[i] == '*') { \
				strcpy(dir->name+i, ptr+i); break; } \
			match = 0; break; \
		} \
		SysPrintf("%d : %s = %s + %s (match=%d)\n", nfile, dir->name, pfile, ptr, match); \
		if (match == 0) continue; \
		dir->size = 8192; \
		v0 = _dir; \
		break; \
	} \
}

/*
 *	struct DIRENTRY* firstfile(char *name,struct DIRENTRY *dir);
 */
 
void bios_firstfile() { // 42
	struct DIRENTRY *dir = (struct DIRENTRY *)Ra1;
	u32 _dir = a1;
	char *ptr;
	int i;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s\n", biosB0n[0x42], Ra0);
#endif

	v0 = 0;

	strcpy(ffile, Ra0);
	pfile = ffile+5;
	nfile = 1;
	if (!strncmp(Ra0, "bu00", 4)) {
		bufile(1);
		v0 = _dir;
	}

	if (!strncmp(Ra0, "bu10", 4)) {
		bufile(2);
		v0 = _dir;
	}

	pc0 = ra;
}

/*
 *	struct DIRENTRY* nextfile(struct DIRENTRY *dir);
 */

void bios_nextfile() { // 43
	struct DIRENTRY *dir = (struct DIRENTRY *)Ra0;
	u32 _dir = a0;
	char *ptr;
	int i;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s\n", biosB0n[0x43], dir->name);
#endif

	v0 = 0;

	if (!strncmp(ffile, "bu00", 4)) {
		bufile(1);
	}

	if (!strncmp(ffile, "bu10", 4)) {
		bufile(2);
	}

	pc0 = ra;
}

#define budelete(mcd) { \
	for (i=1; i<16; i++) { \
		ptr = Mcd##mcd##Data + 128 * i; \
		if ((*ptr & 0xF0) != 0x50) continue; \
		if (strcmp(Ra0+5, ptr+0xa)) continue; \
		*ptr = (*ptr & 0xf) | 0xA0; \
		SysPrintf("delete %s\n", ptr+0xa); \
		v0 = 1; \
		break; \
	} \
}

/*
 *	int delete(char *name);
 */

void bios_delete() { // 45
	char *ptr;
	int i;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %s\n", biosB0n[0x45], Ra0);
#endif

	v0 = 0;

	if (!strncmp(Ra0, "bu00", 4)) {
		budelete(1);
	}

	if (!strncmp(Ra0, "bu10", 4)) {
		budelete(2);
	}

	pc0 = ra;
}

void bios_InitCARD() { // 4a
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x4a], a0);
#endif

	CardState = 0;

	pc0 = ra;
}

void bios_StartCARD() { // 4b
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x4b]);
#endif

	if (CardState == 0) CardState = 1;

	pc0 = ra;
}

void bios_StopCARD() { // 4c
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x4c]);
#endif

	if (CardState == 1) CardState = 0;

	pc0 = ra;
}

void bios__card_write() { // 0x4e
	int port;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x,%x,%x\n", biosB0n[0x4e], a0, a1, a2);
#endif

	port = a0 >> 4;

	if (port == 0) {
		memcpy(Mcd1Data + a1 * 128, Ra2, 128);
		SaveMcd(Config.Mcd1, Mcd1Data, a1 * 128, 128);
	} else {
		memcpy(Mcd2Data + a1 * 128, Ra2, 128);
		SaveMcd(Config.Mcd2, Mcd2Data, a1 * 128, 128);
	}

	DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
//	DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004

	v0 = 1; pc0 = ra;
}

void bios__card_read() { // 0x4f
	int port;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x4f]);
#endif

	port = a0 >> 4;

	if (port == 0) {
		memcpy(Ra2, Mcd1Data + a1 * 128, 128);
	} else {
		memcpy(Ra2, Mcd2Data + a1 * 128, 128);
	}

	DeliverEvent(0x11, 0x2); // 0xf0000011, 0x0004
//	DeliverEvent(0x81, 0x2); // 0xf4000001, 0x0004

	v0 = 1; pc0 = ra;
}

void bios__new_card() { // 0x50
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x50]);
#endif

	pc0 = ra;
}

void bios_GetC0Table() { // 56
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x56]);
#endif

	v0 = 0x674; pc0 = ra;
}

void bios_GetB0Table() { // 57
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s\n", biosB0n[0x57]);
#endif

	v0 = 0x874; pc0 = ra;
}

void bios_ChangeClearPad() { // 5b
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosB0n[0x5b], a0);
#endif	

	pc0 = ra;
}

/* System calls C0 */

/*
 * int SysEnqIntRP(int index , long *queue);
 */

void bios_SysEnqIntRP() { // 02
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosC0n[0x02] ,a0);
#endif

	SysIntRP[a0] = a1;

	v0 = 0; pc0 = ra;
}

/*
 * int SysDeqIntRP(int index , long *queue);
 */

void bios_SysDeqIntRP() { // 03
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x\n", biosC0n[0x03], a0);
#endif

	SysIntRP[a0] = 0;

	v0 = 0; pc0 = ra;
}

void bios_ChangeClearRCnt() { // 0a
	u32 *ptr;

#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("bios_%s: %x, %x\n", biosC0n[0x0a], a0, a1);
#endif

	ptr = (u32*)PSXM((a0 << 2) + 0x8600);
	v0 = *ptr;
	*ptr = a1;

//	psxRegs.CP0.n.Status|= 0x404;
	pc0 = ra;
}

void bios_dummy() { 
#ifdef PSXBIOS_LOG
	PSXBIOS_LOG("unk %lx call: %lx\n", pc0 & 0x1fffff, t1);
#endif
	pc0 = ra; 
}

void (*biosA0[256])();
void (*biosB0[256])();
void (*biosC0[256])();

void psxBiosInit() {
	u32 base, size;
	u32 *ptr; 
	int i;

	for(i = 0; i < 256; i++) {
		biosA0[i] = NULL;
		biosB0[i] = NULL;
		biosC0[i] = NULL;
	}
	biosA0[0x3e] = bios_puts;
	biosA0[0x3f] = bios_printf;

	biosB0[0x3d] = bios_putchar;
	biosB0[0x3f] = bios_puts;

	if (!Config.HLE) return;

	for(i = 0; i < 256; i++) {
		if (biosA0[i] == NULL) biosA0[i] = bios_dummy;
		if (biosB0[i] == NULL) biosB0[i] = bios_dummy;
		if (biosC0[i] == NULL) biosC0[i] = bios_dummy;
	}

    biosA0[0x00] = bios_open;
	biosA0[0x01] = bios_lseek;
	biosA0[0x02] = bios_read;
	biosA0[0x03] = bios_write;
	biosA0[0x04] = bios_close;
	//biosA0[0x05] = bios_ioctl;
	//biosA0[0x06] = bios_exit;
	//biosA0[0x07] = bios_sys_a0_07;
	//biosA0[0x08] = bios_getc;
	//biosA0[0x09] = bios_putc;
	//biosA0[0x0a] = bios_todigit;
	//biosA0[0x0b] = bios_atof;
	//biosA0[0x0c] = bios_strtoul;
	//biosA0[0x0d] = bios_strtol;
	biosA0[0x0e] = bios_abs;
	biosA0[0x0f] = bios_labs;
    biosA0[0x10] = bios_atoi;  
    biosA0[0x11] = bios_atol;  
	//biosA0[0x12] = bios_atob;
	biosA0[0x13] = bios_setjmp;
	biosA0[0x14] = bios_longjmp;
	biosA0[0x15] = bios_strcat;
	biosA0[0x16] = bios_strncat;
	biosA0[0x17] = bios_strcmp;
	biosA0[0x18] = bios_strncmp;
	biosA0[0x19] = bios_strcpy;
	biosA0[0x1a] = bios_strncpy;
	biosA0[0x1b] = bios_strlen;
	biosA0[0x1c] = bios_index;
	biosA0[0x1d] = bios_rindex;
	biosA0[0x1e] = bios_strchr;
	biosA0[0x1f] = bios_strrchr;
	biosA0[0x20] = bios_strpbrk;
	biosA0[0x21] = bios_strspn;
	biosA0[0x22] = bios_strcspn;
	biosA0[0x23] = bios_strtok;
	biosA0[0x24] = bios_strstr;
    biosA0[0x25] = bios_toupper;
    biosA0[0x26] = bios_tolower;
    biosA0[0x27] = bios_bcopy;
    biosA0[0x28] = bios_bzero;
    biosA0[0x29] = bios_bcmp;
	biosA0[0x2a] = bios_memcpy;
	biosA0[0x2b] = bios_memset;
	biosA0[0x2c] = bios_memmove;
	biosA0[0x2d] = bios_memcmp;
	biosA0[0x2e] = bios_memchr;
	biosA0[0x2f] = bios_rand;
	biosA0[0x30] = bios_srand;
	//biosA0[0x31] = bios_qsort;
	//biosA0[0x32] = bios_strtod;
	biosA0[0x33] = bios_malloc;
	//biosA0[0x34] = bios_free;
    //biosA0[0x35] = bios_lsearch;
    //biosA0[0x36] = bios_bsearch;
    //biosA0[0x37] = bios_calloc;
    //biosA0[0x38] = bios_realloc;
	biosA0[0x39] = bios_InitHeap;
    //biosA0[0x3a] = bios__exit;
	biosA0[0x3b] = bios_getchar;
	biosA0[0x3c] = bios_putchar;	
	//biosA0[0x3d] = bios_gets;		
	//biosA0[0x40] = bios_sys_a0_40;		
	//biosA0[0x41] = bios_LoadTest;				
	biosA0[0x42] = bios_Load;		
	biosA0[0x43] = bios_Exec;
	biosA0[0x44] = bios_FlushCache;
    //biosA0[0x45] = bios_InstallInterruptHandler;
	biosA0[0x46] = bios_GPU_dw;
	biosA0[0x47] = bios_mem2vram;
	biosA0[0x48] = bios_SendGPU; 
	biosA0[0x49] = bios_GPU_cw;
    biosA0[0x4a] = bios_GPU_cwb;
	biosA0[0x4b] = bios_GPU_SendPackets;
    biosA0[0x4c] = bios_sys_a0_4c;
	biosA0[0x4d] = bios_GPU_GetGPUStatus;
	//biosA0[0x4e] = bios_GPU_sync;	
	//biosA0[0x4f] = bios_sys_a0_4f;
	//biosA0[0x50] = bios_sys_a0_50;		
	biosA0[0x51] = bios_LoadExec;				
	//biosA0[0x52] = bios_GetSysSp;		
	//biosA0[0x53] = bios_sys_a0_53;
	//biosA0[0x54] = bios__96_init_a54;
	//biosA0[0x55] = bios__bu_init_a55;			
	//biosA0[0x56] = bios__96_remove_a56;	
	//biosA0[0x57] = bios_sys_a0_57;
	//biosA0[0x58] = bios_sys_a0_58;		
	//biosA0[0x59] = bios_sys_a0_59;			
	//biosA0[0x5a] = bios_sys_a0_5a;	
	//biosA0[0x5b] = bios_dev_tty_init;
	//biosA0[0x5c] = bios_dev_tty_open;		
	//biosA0[0x5d] = bios_sys_a0_5d;			
	//biosA0[0x5e] = bios_dev_tty_ioctl;
	//biosA0[0x5f] = bios_dev_cd_open;
	//biosA0[0x60] = bios_dev_cd_read;
	//biosA0[0x61] = bios_dev_cd_close;
	//biosA0[0x62] = bios_dev_cd_firstfile;
	//biosA0[0x63] = bios_dev_cd_nextfile;
	//biosA0[0x64] = bios_dev_cd_chdir;
	//biosA0[0x65] = bios_dev_card_open;
	//biosA0[0x66] = bios_dev_card_read;
	//biosA0[0x67] = bios_dev_card_write;
	//biosA0[0x68] = bios_dev_card_close;
	//biosA0[0x69] = bios_dev_card_firstfile;
	//biosA0[0x6a] = bios_dev_card_nextfile;
	//biosA0[0x6b] = bios_dev_card_erase;
	//biosA0[0x6c] = bios_dev_card_undelete;
	//biosA0[0x6d] = bios_dev_card_format;
	//biosA0[0x6e] = bios_dev_card_rename;
	//biosA0[0x6f] = bios_dev_card_6f;
	biosA0[0x70] = bios__bu_init;
	biosA0[0x71] = bios__96_init;
	biosA0[0x72] = bios__96_remove;
	//biosA0[0x73] = bios_sys_a0_73;
	//biosA0[0x74] = bios_sys_a0_74;
	//biosA0[0x75] = bios_sys_a0_75;
	//biosA0[0x76] = bios_sys_a0_76;
	//biosA0[0x77] = bios_sys_a0_77;
	//biosA0[0x78] = bios__96_CdSeekL;
	//biosA0[0x79] = bios_sys_a0_79;
	//biosA0[0x7a] = bios_sys_a0_7a;
	//biosA0[0x7b] = bios_sys_a0_7b;
	//biosA0[0x7c] = bios__96_CdGetStatus;
	//biosA0[0x7d] = bios_sys_a0_7d;
	//biosA0[0x7e] = bios__96_CdRead;
	//biosA0[0x7f] = bios_sys_a0_7f;
	//biosA0[0x80] = bios_sys_a0_80;
	//biosA0[0x81] = bios_sys_a0_81;
	//biosA0[0x82] = bios_sys_a0_82;		
	//biosA0[0x83] = bios_sys_a0_83;
	//biosA0[0x84] = bios_sys_a0_84;
	//biosA0[0x85] = bios__96_CdStop;	
	//biosA0[0x86] = bios_sys_a0_86;
	//biosA0[0x87] = bios_sys_a0_87;
	//biosA0[0x88] = bios_sys_a0_88;
	//biosA0[0x89] = bios_sys_a0_89;
	//biosA0[0x8a] = bios_sys_a0_8a;
	//biosA0[0x8b] = bios_sys_a0_8b;
	//biosA0[0x8c] = bios_sys_a0_8c;
	//biosA0[0x8d] = bios_sys_a0_8d;
	//biosA0[0x8e] = bios_sys_a0_8e;		
	//biosA0[0x8f] = bios_sys_a0_8f;
	//biosA0[0x90] = bios_sys_a0_90;
	//biosA0[0x91] = bios_sys_a0_91;
	//biosA0[0x92] = bios_sys_a0_92;
	//biosA0[0x93] = bios_sys_a0_93;
	//biosA0[0x94] = bios_sys_a0_94;
	//biosA0[0x95] = bios_sys_a0_95;
	//biosA0[0x96] = bios_AddCDROMDevice;
	//biosA0[0x97] = bios_AddMemCardDevide;
	//biosA0[0x98] = bios_DisableKernelIORedirection;
	//biosA0[0x99] = bios_EnableKernelIORedirection;
	//biosA0[0x9a] = bios_sys_a0_9a;
	//biosA0[0x9b] = bios_sys_a0_9b;
	//biosA0[0x9c] = bios_SetConf;
	//biosA0[0x9d] = bios_GetConf;
	//biosA0[0x9e] = bios_sys_a0_9e;
	//biosA0[0x9f] = bios_SetMem;
	//biosA0[0xa0] = bios__boot;
	//biosA0[0xa1] = bios_SystemError;
	//biosA0[0xa2] = bios_EnqueueCdIntr;
	//biosA0[0xa3] = bios_DequeueCdIntr;
	//biosA0[0xa4] = bios_sys_a0_a4;
	//biosA0[0xa5] = bios_ReadSector;
	//biosA0[0xa6] = bios_get_cd_status;
	//biosA0[0xa7] = bios_bufs_cb_0;
	//biosA0[0xa8] = bios_bufs_cb_1;
	//biosA0[0xa9] = bios_bufs_cb_2;
	//biosA0[0xaa] = bios_bufs_cb_3;
	biosA0[0xab] = bios__card_info;
	biosA0[0xac] = bios__card_load;
	//biosA0[0axd] = bios__card_auto;
	//biosA0[0xae] = bios_bufs_cd_4;
	//biosA0[0xaf] = bios_sys_a0_af;
	//biosA0[0xb0] = bios_sys_a0_b0;
	//biosA0[0xb1] = bios_sys_a0_b1;
	//biosA0[0xb2] = bios_do_a_long_jmp
	//biosA0[0xb3] = bios_sys_a0_b3;
	//biosA0[0xb4] = bios_sub_function;
//*******************B0 CALLS****************************
	//biosB0[0x00] = bios_SysMalloc;
	//biosB0[0x01] = bios_sys_b0_01;
	biosB0[0x02] = bios_SetRCnt;
	biosB0[0x03] = bios_GetRCnt;
	biosB0[0x04] = bios_StartRCnt;
	biosB0[0x05] = bios_StopRCnt;
	biosB0[0x06] = bios_ResetRCnt;
	biosB0[0x07] = bios_DeliverEvent;
	biosB0[0x08] = bios_OpenEvent;
	biosB0[0x09] = bios_CloseEvent;
	biosB0[0x0a] = bios_WaitEvent;
	biosB0[0x0b] = bios_TestEvent;
	biosB0[0x0c] = bios_EnableEvent;
	biosB0[0x0d] = bios_DisableEvent;
    biosB0[0x0e] = bios_OpenTh;
    biosB0[0x0f] = bios_CloseTh;
    biosB0[0x10] = bios_ChangeTh;
    //biosB0[0x11] = bios_bios_b0_11;
	biosB0[0x12] = bios_InitPAD;
	biosB0[0x13] = bios_StartPAD;
	biosB0[0x14] = bios_StopPAD;
	biosB0[0x15] = bios_PAD_init;
	biosB0[0x16] = bios_PAD_dr;
	biosB0[0x17] = bios_ReturnFromException;
	biosB0[0x18] = bios_ResetEntryInt;
	biosB0[0x19] = bios_HookEntryInt;
    //biosB0[0x1a] = bios_sys_b0_1a;
	//biosB0[0x1b] = bios_sys_b0_1b;
	//biosB0[0x1c] = bios_sys_b0_1c;
	//biosB0[0x1d] = bios_sys_b0_1d;
	//biosB0[0x1e] = bios_sys_b0_1e;
	//biosB0[0x1f] = bios_sys_b0_1f;
	biosB0[0x20] = bios_UnDeliverEvent;
	//biosB0[0x21] = bios_sys_b0_21;
	//biosB0[0x22] = bios_sys_b0_22;
	//biosB0[0x23] = bios_sys_b0_23;
	//biosB0[0x24] = bios_sys_b0_24;
	//biosB0[0x25] = bios_sys_b0_25;
	//biosB0[0x26] = bios_sys_b0_26;
	//biosB0[0x27] = bios_sys_b0_27;
	//biosB0[0x28] = bios_sys_b0_28;
	//biosB0[0x29] = bios_sys_b0_29;
	//biosB0[0x2a] = bios_sys_b0_2a;
	//biosB0[0x2b] = bios_sys_b0_2b;
	//biosB0[0x2c] = bios_sys_b0_2c;
	//biosB0[0x2d] = bios_sys_b0_2d;
	//biosB0[0x2e] = bios_sys_b0_2e;
	//biosB0[0x2f] = bios_sys_b0_2f;
	//biosB0[0x30] = bios_sys_b0_30;
	//biosB0[0x31] = bios_sys_b0_31;
    biosB0[0x32] = bios_open;
	biosB0[0x33] = bios_lseek;
	biosB0[0x34] = bios_read;		
	biosB0[0x35] = bios_write;
    biosB0[0x36] = bios_close;
	//biosB0[0x37] = bios_ioctl;
	//biosB0[0x38] = bios_exit;
	//biosB0[0x39] = bios_sys_b0_39;
	//biosB0[0x3a] = bios_getc;
	//biosB0[0x3b] = bios_putc;
	biosB0[0x3c] = bios_getchar;
	//biosB0[0x3e] = bios_gets;
	//biosB0[0x40] = bios_cd;
	//biosB0[0x41] = bios_format;
	biosB0[0x42] = bios_firstfile;
	biosB0[0x43] = bios_nextfile;
	//biosB0[0x44] = bios_rename;
	biosB0[0x45] = bios_delete;
	//biosB0[0x46] = bios_undelete;
	//biosB0[0x47] = bios_AddDevice;
	//biosB0[0x48] = bios_RemoteDevice;
	//biosB0[0x49] = bios_PrintInstalledDevices;
	biosB0[0x4a] = bios_InitCARD;
	biosB0[0x4b] = bios_StartCARD;
	biosB0[0x4c] = bios_StopCARD;
	//biosB0[0x4d] = bios_sys_b0_4d;
	biosB0[0x4e] = bios__card_write;
	biosB0[0x4f] = bios__card_read;
	biosB0[0x50] = bios__new_card;
	//biosB0[0x51] = bios_Krom2RawAdd;
	//biosB0[0x52] = bios_sys_b0_52;
	//biosB0[0x53] = bios_sys_b0_53;
	//biosB0[0x54] = bios__get_errno;
	//biosB0[0x55] = bios__get_error;
	biosB0[0x56] = bios_GetC0Table;
	biosB0[0x57] = bios_GetB0Table;
    //biosB0[0x58] = bios__card_chan;
	//biosB0[0x59] = bios_sys_b0_59;
	//biosB0[0x5a] = bios_sys_b0_5a;
	biosB0[0x5b] = bios_ChangeClearPad;
	//biosB0[0x5c] = bios__card_status;
	//biosB0[0x5d] = bios__card_wait;
//*******************C0 CALLS****************************
	//biosC0[0x00] = bios_InitRCnt;
	//biosC0[0x01] = bios_InitException;
	biosC0[0x02] = bios_SysEnqIntRP;
	biosC0[0x03] = bios_SysDeqIntRP;
	//biosC0[0x04] = bios_get_free_EvCB_slot;
	//biosC0[0x05] = bios_get_free_TCB_slot;
	//biosC0[0x06] = bios_ExceptionHandler;
	//biosC0[0x07] = bios_InstallExeptionHandler;
	//biosC0[0x08] = bios_SysInitMemory;
	//biosC0[0x09] = bios_SysInitKMem;
	biosC0[0x0a] = bios_ChangeClearRCnt;	
	//biosC0[0x0b] = bios_SystemError;
	//biosC0[0x0c] = bios_InitDefInt;
    //biosC0[0x0d] = bios_sys_c0_0d;
	//biosC0[0x0e] = bios_sys_c0_0e;
	//biosC0[0x0f] = bios_sys_c0_0f;
	//biosC0[0x10] = bios_sys_c0_10;
	//biosC0[0x11] = bios_sys_c0_11;
	//biosC0[0x12] = bios_InstallDevices;
	//biosC0[0x13] = bios_FlushStfInOutPut;
	//biosC0[0x14] = bios_sys_c0_14;
	//biosC0[0x15] = bios__cdevinput;
	//biosC0[0x16] = bios__cdevscan;
	//biosC0[0x17] = bios__circgetc;
	//biosC0[0x18] = bios__circputc;		  
	//biosC0[0x19] = bios_ioabort;
	//biosC0[0x1a] = bios_sys_c0_1a
	//biosC0[0x1b] = bios_KernelRedirect;
	//biosC0[0x1c] = bios_PatchAOTable;
//************** THE END ***************************************

	base = 0x1000;
	size = sizeof(EvCB) * 32;
	Event = (void *)&psxR[base]; base+= size*6;
	memset(Event, 0, size * 6);
	HwEV = Event;
	EvEV = Event + 32;
	RcEV = Event + 32*2;
	UeEV = Event + 32*3;
	SwEV = Event + 32*4;
	ThEV = Event + 32*5;

	ptr = (u32*)&psxM[0x0874]; // b0 table
	ptr[0] = 0x4c54 - 0x884;

	ptr = (u32*)&psxM[0x0674]; // c0 table
	ptr[6] = 0xc80;

	memset(SysIntRP, 0, sizeof(SysIntRP));
	memset(Thread, 0, sizeof(Thread));
	Thread[0].status = 2; // main thread

	psxMu32(0x0150) = 0x160;
	psxMu32(0x0154) = 0x320;
	psxMu32(0x0160) = 0x248;
	strcpy(&psxM[0x248], "bu");
/*	psxMu32(0x0ca8) = 0x1f410004;
	psxMu32(0x0cf0) = 0x3c020000;
	psxMu32(0x0cf4) = 0x2442641c;
	psxMu32(0x09e0) = 0x43d0;
	psxMu32(0x4d98) = 0x946f000a;
*/
	// opcode HLE
	psxRu32(0x0000) = (0x3b << 26) | 4;
	psxMu32(0x0000) = (0x3b << 26) | 0;
	psxMu32(0x00a0) = (0x3b << 26) | 1;
	psxMu32(0x00b0) = (0x3b << 26) | 2;
	psxMu32(0x00c0) = (0x3b << 26) | 3;
	psxMu32(0x4c54) = (0x3b << 26) | 0;
	psxMu32(0x8000) = (0x3b << 26) | 5;
	psxMu32(0x07a0) = (0x3b << 26) | 0;
	psxMu32(0x0884) = (0x3b << 26) | 0;
	psxMu32(0x0894) = (0x3b << 26) | 0;
}

void psxBiosShutdown() {
}

__inline void SaveRegs() {
	memcpy(regs, psxRegs.GPR.r, 32*4);
	regs[32] = psxRegs.GPR.n.lo;
	regs[33] = psxRegs.GPR.n.hi;
	regs[34] = psxRegs.pc;
}

__inline void LoadRegs() {
	memcpy(psxRegs.GPR.r, regs, 32*4);
	psxRegs.GPR.n.lo = regs[32];
	psxRegs.GPR.n.hi = regs[33];
}

#define bios_PADpoll(pad) { \
	PAD##pad##_startPoll(pad); \
	pad_buf##pad[0] = 0; \
	pad_buf##pad[1] = PAD##pad##_poll(0x42); \
	if (!(pad_buf##pad[1] & 0x0f)) { \
		bufcount = 32; \
	} else { \
		bufcount = (pad_buf##pad[1] & 0x0f) * 2; \
	} \
	PAD##pad##_poll(0); \
	i = 2; \
	while (bufcount--) { \
		pad_buf##pad[i++] = PAD##pad##_poll(0); \
	} \
}

void biosInterrupt() {
	int i, bufcount;

//	if (psxHu32(0x1070) & 0x1) { // Vsync
		if (pad_buf) {
			PAD1_startPoll(1);
			PAD1_poll(0x42);
			PAD1_poll(0);
			*pad_buf = PAD1_poll(0) << 8;
			*pad_buf|= PAD1_poll(0);
			PAD2_startPoll(2);
			PAD2_poll(0x42);
			PAD2_poll(0);
			*pad_buf|= PAD2_poll(0) << 24;
			*pad_buf|= PAD2_poll(0) << 16;
		}
		if (pad_buf1) {
			bios_PADpoll(1);
		}

		if (pad_buf2) {
			bios_PADpoll(2);
		}

	if (psxHu32(0x1070) & 0x1) { // Vsync
		if (RcEV[3][1].status == EvStACTIVE) {
			softCall(RcEV[3][1].fhandler);
//			hwWrite32(0x1f801070, ~(1));
		}
	}

	if (psxHu32(0x1070) & 0x70) { // Rcnt 0,1,2
		int i;

		for (i=0; i<3; i++) {
			if (psxHu32(0x1070) & (1 << (i+4))) {
				if (RcEV[i][1].status == EvStACTIVE) {
					softCall(RcEV[i][1].fhandler);
					psxHwWrite32(0x1f801070, ~(1 << (i+4)));
				}
			}
		}
	}
}

void psxBiosException() {
	int i;

	switch (psxRegs.CP0.n.Cause & 0x3c) {
		case 0x00: // Interrupt
#ifdef PSXCPU_LOG
//			PSXCPU_LOG("interrupt\n");
#endif
			SaveRegs();

			biosInterrupt();

			for (i=0; i<8; i++) {
				if (SysIntRP[i]) {
					u32 *queue = (u32*)PSXM(SysIntRP[i]);

					s0 = queue[2];
					softCall(queue[1]);
				}
			}

			if (jmp_int != NULL) {
				int i;

				psxHwWrite32(0x1f801070, 0xffffffff);

				ra = jmp_int[0];
				sp = jmp_int[1];
				fp = jmp_int[2];
				for (i=0; i<8; i++) // s0-s7
					 psxRegs.GPR.r[16+i] = jmp_int[3+i];
				gp = jmp_int[11];

				v0 = 1;
				pc0 = ra;
				return;
			}
			psxHwWrite16(0x1f801070, 0);
			break;
		case 0x20: // Syscall
#ifdef PSXCPU_LOG
//			PSXCPU_LOG("syscall exp %x\n", a0);
#endif
			switch (a0) {
				case 1: // EnterCritical - disable irq's
					psxRegs.CP0.n.Status&=~0x404; break;
				case 2: // ExitCritical - enable irq's
					psxRegs.CP0.n.Status|= 0x404; break;
			}
			pc0 = psxRegs.CP0.n.EPC + 4;

			psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & 0xfffffff0) |
								  ((psxRegs.CP0.n.Status & 0x3c) >> 2);
			return;
		default:
#ifdef PSXCPU_LOG
			PSXCPU_LOG("unk exp\n");
#endif
			break;
	}

	pc0 = psxRegs.CP0.n.EPC;
	if (psxRegs.CP0.n.Cause & 0x80000000) pc0+=4;

	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & 0xfffffff0) |
						  ((psxRegs.CP0.n.Status & 0x3c) >> 2);
}

#define bfreeze(ptr, size) \
	if (Mode == 1) memcpy(&psxR[base], ptr, size); \
	if (Mode == 0) memcpy(ptr, &psxR[base], size); \
	base+=size;

#define bfreezes(ptr) bfreeze(ptr, sizeof(ptr))
#define bfreezel(ptr) bfreeze(ptr, 4)

#define bfreezepsxMptr(ptr) \
	if (Mode == 1) { \
		if (ptr) psxRu32(base) = (u32)ptr - (u32)psxM; \
		else psxRu32(base) = 0; \
	} else { \
		if (psxRu32(base)) (u8*)ptr = (u8*)(psxM + psxRu32(base)); \
		else ptr = NULL; \
	} \
	base+=4;

void psxBiosFreeze(int Mode) {
	u32 base = 0x40000;

	bfreezepsxMptr(jmp_int);
	bfreezepsxMptr(pad_buf);
	bfreezepsxMptr(pad_buf1);
	bfreezepsxMptr(pad_buf2);
	bfreezepsxMptr(heap_addr);
	bfreezel(&pad_buf1len);
	bfreezel(&pad_buf2len);
	bfreezes(regs);
	bfreezes(SysIntRP);
	bfreezel(&CardState);
	bfreezes(Thread);
	bfreezel(&CurThread);
	bfreezes(FDesc);
}
