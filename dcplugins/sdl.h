#ifndef __SDL_H__
#define __SDL_H__

// #include "SDL_video.h"

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */


typedef signed short Sint16;
typedef unsigned long Uint32;
typedef unsigned short Uint16;
typedef unsigned char Uint8;


typedef struct {
        Sint16 x, y;
        Uint16 w, h;
} SDL_Rect;

typedef struct {
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint8 unused;
} SDL_Color;

typedef struct {
        int       ncolors;
        SDL_Color *colors;
} SDL_Palette;

typedef struct SDL_PixelFormat {
        SDL_Palette *palette;
        Uint8  BitsPerPixel;
        Uint8  BytesPerPixel;
        Uint8  Rloss;
        Uint8  Gloss;
        Uint8  Bloss;
        Uint8  Aloss;
        Uint8  Rshift;
        Uint8  Gshift;
        Uint8  Bshift;
        Uint8  Ashift;
        Uint32 Rmask;
        Uint32 Gmask;
        Uint32 Bmask;
        Uint32 Amask;

        Uint32 colorkey;
        Uint8  alpha;
} SDL_PixelFormat;


typedef struct SDL_Surface {
        Uint32 flags;                          
        SDL_PixelFormat *format;               
        int w, h;                             
		Uint16 bpp;
		Uint32 Rmask, Gmask, Bmask, Amask;
		
        Uint16 pitch;                           
        void *pixels;                           
        int offset;                          

        struct private_hwdata *hwdata;

        SDL_Rect clip_rect;                    
        Uint32 unused1;                       

        Uint32 locked;                       

        struct SDL_BlitMap *map;              


        unsigned int format_version;          

        int refcount;                         
} SDL_Surface;

#define SDL_SWSURFACE	0x00000000
#define SDL_DOUBLEBUF	0x40000000	/* Set up double-buffered video mode */
#define SDL_ANYFORMAT	0x10000000
#define SDL_INIT_VIDEO 0
#define SDL_FULLSCREEN	0x80000000
#define SDL_HWACCEL	0x00000100
#define SDL_HWSURFACE	0x00000001	/* Surface is in video memory */
#define SDL_HWPALETTE	0x20000000	/* Surface has exclusive palette */

#define SDLK_INSERT 0x4900
#define SDLK_HOME 0x4a00
#define SDLK_PAGEUP 0x4b00
#define SDLK_PAGEDOWN 0x4e00
#define SDLK_END 0x4d00
#define SDLK_DELETE 0x4c00
#define SDLK_F5 0x3e00

//#define SDL_SWSURFACE   0x00000000      /* Surface is in system memory */
//#define SDL_PREALLOC    0x01000000      /* Surface uses preallocated memory */

#endif
