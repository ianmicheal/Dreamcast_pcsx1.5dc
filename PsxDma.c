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

#include "PsxCommon.h"

// Dma0/1 in Mdec.c
// Dma3   in CdRom.c

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
	switch (chcr) {
		case 0x01000201: //cpu to spu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 4 - SPU mem2spu *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			SPU_writeDMAMem((u16 *)PSXM(madr), (bcr >> 16) * (bcr & 0xffff) * 2);
			break;
		case 0x01000200: //spu to cpu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 4 - SPU spu2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
    		SPU_readDMAMem ((u16 *)PSXM(madr), (bcr >> 16) * (bcr & 0xffff) * 2);
			break;
#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA 4 - SPU unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}
}

void psxDma2(u32 madr, u32 bcr, u32 chcr) { // GPU
	switch(chcr) {
		case 0x01000200: // vram2mem
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU vram2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_readDataMem((u32 *)PSXM(madr), (bcr >> 16) * (bcr & 0xffff));
			psxCpu->Clear(madr, ((bcr >> 16) * (bcr & 0xffff)) / 4);
			break;

		case 0x01000201: // mem2vram
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU mem2vram *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_writeDataMem((u32 *)PSXM(madr), (bcr >> 16) * (bcr & 0xffff));
			break;

		case 0x01000401: // dma chain
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU dma chain *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_dmaChain((u32 *)psxM, madr & 0x1fffff);
			break;
#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA 2 - GPU unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}
}

void psxDma6(u32 madr, u32 bcr, u32 chcr) {
	u32 *mem = (u32 *)PSXM(madr);

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA 6 - OT *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		while (bcr--) {
			*mem-- = (madr - 4) & 0xffffff;
			madr -= 4;
		}
		mem++; *mem = 0xffffff;
	} else {
		// Unknown option
#ifdef PSXDMA_LOG
		PSXDMA_LOG("*** DMA 6 - OT unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
	}
}
