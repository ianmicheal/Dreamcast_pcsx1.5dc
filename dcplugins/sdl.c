#include <dc/video.h>
#include "SDL.h"
//#include "SDL_video.h"
#include "system.h"
#include <kos.h>
#include"begin_code.h"



#define TEX_WIDTH 1024
#define TEX_HEIGHT 512

void draw_poly_strf(float x1, float y1, float z1, float a, float r,float g, float b, char *fmt, ...);
static SDL_Surface* SDL_PublicSurface;
int iMenuDisp = 0;
char szMenuString[256];
Uint16 conv2RGB[65536];
void init_conversion()
{
int i;
for (i = 0; i < 65536; i++)
conv2RGB[i] = ((i & 0x1f)<<11)|((i & 0x7c00)>>10)|((i & 0x3e0)<<1);	
}
SDL_Surface * SDL_SetVideoMode (int width, int height, int bpp, Uint32 flags)
{
SDL_Surface *surface;
printf("SDL_SetVideoMode: width %d height %d bpp %d\n", width, height, bpp);
surface = (SDL_Surface *)malloc(sizeof(*surface));
if ( surface == NULL ) {
return(NULL);
}
surface->w = width;
surface->h = height;
surface->bpp = 16; // bpp;
//surface->pixels = 0xa5200000;
SDL_PublicSurface = surface;
return(SDL_PublicSurface);
}
SDL_Surface * SDL_DisplayFormat (SDL_Surface *surface)
{
printf("SDL_DisplayFormat\n");
return 0;
}
void SDL_FreeSurface (SDL_Surface *surface)
{
	printf("SDL_FreeSurface\n");	
}
void SDL_QuitSubSystem(Uint32 flags)
{
printf("SDL_QuitSubSystem\n");	
}
pvr_ptr_t screen_tex = NULL;
int SDL_InitSubSystem(Uint32 flags)
{
printf("SDL_InitSubSystem\n");
SDL_PublicSurface = NULL;	/* Until SDL_SetVideoMode() */
screen_tex = pvr_mem_malloc(TEX_WIDTH*TEX_HEIGHT*2);
init_conversion();
return 0;
}
void SDL_OutOfMemory()
{
printf("SDL: Out of memory!\n");
}
SDL_Surface * SDL_CreateRGBSurface (Uint32 flags,
int width, int height, int depth,
Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
SDL_Surface *surface;
printf("SDL_CreateRGBSurface\n");
surface = (SDL_Surface *)malloc(sizeof(*surface));
if ( surface == NULL ) {
SDL_OutOfMemory();
return(NULL);
}
surface->w = width;
surface->h = height;
surface->pixels = NULL;
surface->offset = 0;
surface->hwdata = NULL;
surface->locked = 0;
surface->map = NULL;
surface->format_version = 0;
surface->Rmask = Rmask;
surface->Gmask = Gmask;
surface->Bmask = Bmask;
surface->Amask = Amask;
surface->bpp = depth;
if ( surface->w && surface->h ) {
surface->pixels = malloc(surface->h*surface->pitch);
if ( surface->pixels == NULL ) {
SDL_FreeSurface(surface);
SDL_OutOfMemory();
return(NULL);
}
memset(surface->pixels, 0, surface->h*surface->pitch);
}
surface->refcount = 1;
return(surface);
}
SDL_Surface * SDL_CreateRGBSurfaceFrom (void *pixels,
int width, int height, int depth, int pitch,
Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
printf("SDL_CreateRGBSurfaceFrom\n");
SDL_Surface *surface;
surface = SDL_CreateRGBSurface(0, 0, 0, depth,
Rmask, Gmask, Bmask, Amask);
if ( surface != NULL ) {
surface->pixels = pixels;
surface->w = width;
surface->h = height;
surface->pitch = pitch;
}
printf("\twidth %d height %d\n", width, height);
printf("\t(%d, %08x, %08x, %08x, %08x)\n", depth, Rmask, Bmask, Gmask, Amask);
return(surface);
}
void SDL_WM_SetCaption (const char *title, const char *icon)
{
strcpy(szMenuString, title);
iMenuDisp = 1;
}
int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
int y, x;
unsigned int pitch;
Uint16* tptr;
if (dstrect == NULL) {
pitch = (dst->w * (dst->bpp/8))/sizeof(*tptr);
tptr = (Uint16*)dst->pixels;
for (y = 0; y < dst->h; y++) {
for (x = 0; x < dst->w; x++) {
*(tptr+x) = color;
}
tptr += pitch;
}
} else {
pitch = (dst->w * (dst->bpp/8))/sizeof(*tptr);
tptr = (Uint16*)dst->pixels + dstrect->y*pitch;
for (y = 0; y < (dstrect->h-dstrect->y); y++) {
for (x = 0; x < (dstrect->w-dstrect->x); x++) {
*(tptr+dstrect->x+x) = color;
}
tptr += pitch;
}
}
return 0;
}
static Uint16 scr_x = 0, scr_y = 0, scr_h = 0, scr_w = 0;
int SDL_Flip(SDL_Surface *screen)
{
pvr_poly_cxt_t cxt;
pvr_poly_hdr_t hdr;
pvr_vertex_t vert;
if (scr_h == 0) return 0;
if (scr_w == 0) return 0;
Uint32 y_top, y_bot, x_left, x_right;
float ratio = (float)scr_h / (float)scr_w;
if (ratio > 0.75) {
y_top = 0; y_bot = 480;
x_left = (Uint32) ((640.0 - (480.0 / ratio)) / 2.0);
x_right = 640 - x_left;
} else {
x_left = 0; x_right = 640;
y_top = (Uint32) ((480.0 - (640.0 * ratio)) / 2.0);
y_bot = 480 - y_top;    
}	


pvr_wait_ready();
pvr_scene_begin();
pvr_list_begin(PVR_LIST_TR_POLY);
pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, TEX_WIDTH, TEX_HEIGHT, screen_tex, PVR_FILTER_TRILINEAR1);
pvr_poly_compile(&hdr, &cxt);
pvr_prim(&hdr, sizeof(hdr));


vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
vert.oargb = 0;
vert.flags = PVR_CMD_VERTEX;
    
vert.x = x_left; vert.y = y_top; vert.z = 1;
vert.u = 0.0; vert.v = 0.0;
pvr_prim(&vert, sizeof(vert));
    
vert.x = x_right; vert.y = y_top; vert.z = 1;
vert.u = (scr_w)/1024.0; vert.v = 0.0;
pvr_prim(&vert, sizeof(vert));
    
vert.x = x_left; vert.y = y_bot; vert.z = 1;
vert.u = 0.0; vert.v = (scr_h)/512.0;
pvr_prim(&vert, sizeof(vert));
    
vert.x = x_right; vert.y = y_bot; vert.z = 1;
vert.u = (scr_w)/1024.0; vert.v = (scr_h)/512.0;
vert.flags = PVR_CMD_VERTEX_EOL;
pvr_prim(&vert, sizeof(vert));

if (iMenuDisp) {
draw_poly_strf(10.0, 10.0, 100.0, 1.0, 1.0, 1.0, 1.0, szMenuString);
iMenuDisp = 0;
}
pvr_list_finish();
pvr_scene_finish();
return 0;
}
#include <dc/sq.h>
/*rewritten this part of the code to have no warnings and act like it should
Reaper2k2 well its a start at lest to making this code better */

/* copies n bytes from src to dest, dest must be 32-byte aligned */
void * inline_sq_cpy(void *dest, void *src, int n) {
unsigned int *d = (unsigned int *)(void *)
(0xe0000000 | (((unsigned long)dest) & 0x03ffffe0));
Uint16 *e = dest;
Uint16 *s = src;
/* Align dest on 32-Byte boundary */
while ((Uint32)e & 0x1f) {
*e = conv2RGB[*s];
e++; s++; n--;
}
/* Set store queue memory area as desired */
QACR0 = ((((Uint32)e)>>26)<<2)&0x1c;
QACR1 = ((((Uint32)e)>>26)<<2)&0x1c;
/* fill/write queues as many times necessary */
int m = n>>4;
while(m--) {
d[0] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[1] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[2] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[3] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[4] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[5] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[6] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
d[7] = conv2RGB[s[0]] | conv2RGB[s[1]]<<16; s+=2;
asm("pref @%0" : : "r" (d));
d += 8; e = (Uint16*) ((Uint32)e + 32);
}
/* Wait for both store queues to complete */
d = (unsigned int *)0xe0000000;
d[0] = d[8] = 0;
}
int SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
int y, x;
register int lx = srcrect->w - srcrect->x, ly = srcrect->h - srcrect->y;
Uint32 srcpitch, dstpitch;
Uint16 *srcptr, *dstptr;
scr_x = dstrect->x;
scr_y = dstrect->y;
scr_w = lx;
scr_h = ly;
srcpitch = (src->w * (src->bpp/8))/sizeof(*srcptr);
dstpitch = 1024.0;
srcptr = (Uint16*)src->pixels + srcrect->y*srcpitch + srcrect->x;
dstptr = (Uint16*)screen_tex;
for (y = 0; y < ly; y++) {
Uint16 *s = srcptr, *d = dstptr;
for (x = 0; x < lx; x++) {
*d = conv2RGB[*s];
s++; d++;
}
dstptr += dstpitch; srcptr += srcpitch;
}
return 0;
}
int SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	printf("SDL_SoftStretch\n");
	return 0;
}


