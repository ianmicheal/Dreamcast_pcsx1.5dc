# KallistiOS-D DISABLE_THREADS -D _SDL2 ##version##
#
# kernel Makefile
# (c)2001 Dan Potter
#   
# $Id: Makefile,v 1.1.1.1 2002/09/28 07:16:59 cvs Exp $

OBJS = Soft.o Prim.o Gpu.o fps.o Menu.o Key.o Config.o Draw.o SDL.o
#	SDL_surface.o SDL_pixels.o SDL_RLEaccel.o SDL_blit.o \
#	SDL_blit_0.o SDL_blit_1.o SDL_blit_N.o SDL_blit_A.o \
#	SDL_kos.o SDL_video.o SDL_cursor.o SDL_sysmutex.o SDL_syssem.o

SUBDIRS =

myall: $(OBJS)
	rm -f $(KOS_BASE)/lib/libgpu.a
	$(KOS_AR) rcs $(KOS_BASE)/lib/libgpu.a $(OBJS)

include $(KOS_BASE)/Makefile.prefab

KOS_CFLAGS += -D_FASTSOLID -D_SDL -D_SDL2 -DSDL_INLINE_OKAY -I SDL -D __LITTLE_ENDIAN__   

