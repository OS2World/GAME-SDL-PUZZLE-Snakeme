/*
 * os2compat.h -- SDL1 -> SDL2 compatibility shim for SnakeMe ArcaOS/OS2 port
 *
 * Force-included via -include src/os2compat.h in Makefile.os2.
 * Resolves SDL1 APIs and constants that were removed in SDL2.
 */
#ifndef OS2COMPAT_H
#define OS2COMPAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_endian.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Global SDL2 window and 640x480 logical back-buffer -- defined in src/SnakeMe.cpp */
extern SDL_Window  *snakeme_window;
extern SDL_Surface *snakeme_logical;
extern void snakeme_update_window(void);

#ifdef __cplusplus
}
#endif

/* SDL1 surface-creation flags -- SDL2 SDL_CreateRGBSurface ignores the flags arg */
#define SDL_SWSURFACE   0
#define SDL_HWSURFACE   0
#define SDL_ANYFORMAT   0
/* SDL_SRCCOLORKEY = 1 (SDL_TRUE): SDL2 SDL_SetColorKey uses flag as a SDL_bool */
#define SDL_SRCCOLORKEY 1
/* SDL_SRCALPHA kept non-zero so SDL_SetAlpha compat can distinguish enable/disable */
#define SDL_SRCALPHA    0x00010000

/* SDL_UpdateRect: scale logical 640x480 back-buffer to the window, then present */
#define SDL_UpdateRect(s, x, y, w, h)  snakeme_update_window()

/* SDL1 key-repeat API removed in SDL2 */
#define SDL_DEFAULT_REPEAT_DELAY    500
#define SDL_DEFAULT_REPEAT_INTERVAL 30
#define SDL_EnableUNICODE(x)        ((void)0)
#define SDL_EnableKeyRepeat(a, b)   ((void)0)

/* SDLKey typedef removed in SDL2 */
typedef SDL_Keycode SDLKey;

/* SDL_GetKeyName returns const char* in SDL2; SGU's SetCaption expects char*.
   Strip non-ASCII bytes so a stale keycode never sends out-of-range indices
   into the bitmap font (which only covers printable ASCII 32-126). */
static inline char *SDL_GetKeyName_compat(SDL_Keycode key)
{
    static char buf[64];
    const char *name = SDL_GetKeyName(key);
    int j = 0;
    for (int i = 0; name[i] && j < 62; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c >= 32 && c < 127) buf[j++] = (char)c;
    }
    if (j == 0) { buf[0] = '?'; buf[1] = '\0'; }
    else buf[j] = '\0';
    return buf;
}
#define SDL_GetKeyName SDL_GetKeyName_compat

/* SDL_GetKeyState removed in SDL2; callers must use SDL_GetKeyboardState + scancodes */
#define SDL_GetKeyState(n)  SDL_GetKeyboardState(n)

/* SDL_SetTimer removed in SDL2; only usage in this codebase is SDL_SetTimer(0,NULL)
   (cancel a never-registered timer), so a no-op is correct */
#define SDL_SetTimer(interval, cb)  ((void)0)

/* SDL_SetAlpha removed in SDL2; flag != 0 means enable per-surface alpha */
static inline int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha)
{
    if (flag) {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
        SDL_SetSurfaceAlphaMod(surface, alpha);
    } else {
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
    return 0;
}

/* Mouse coordinate scaling: window is 960x720, game logic expects 640x480.
   Scale factor is 1.5x (3/2); invert with *2/3 = *640/960 = *480/720.
   All three inline functions are defined BEFORE their #defines so the calls
   inside them resolve to the real SDL functions (no infinite recursion). */
#define SNAKEME_TO_LOGICAL_X(v) ((int)((long)(v) * 640 / 960))
#define SNAKEME_TO_LOGICAL_Y(v) ((int)((long)(v) * 480 / 720))

static inline Uint32 snakeme_GetMouseState(int *x, int *y)
{
    Uint32 state = SDL_GetMouseState(x, y);
    if (x) *x = SNAKEME_TO_LOGICAL_X(*x);
    if (y) *y = SNAKEME_TO_LOGICAL_Y(*y);
    return state;
}

static inline void snakeme_scale_mouse_event(SDL_Event *ev)
{
    if (!ev) return;
    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        ev->button.x = SNAKEME_TO_LOGICAL_X(ev->button.x);
        ev->button.y = SNAKEME_TO_LOGICAL_Y(ev->button.y);
        break;
    case SDL_MOUSEMOTION:
        ev->motion.x    = SNAKEME_TO_LOGICAL_X(ev->motion.x);
        ev->motion.y    = SNAKEME_TO_LOGICAL_Y(ev->motion.y);
        ev->motion.xrel = SNAKEME_TO_LOGICAL_X(ev->motion.xrel);
        ev->motion.yrel = SNAKEME_TO_LOGICAL_Y(ev->motion.yrel);
        break;
    default:
        break;
    }
}

/* WaitEvent wrapper: game uses SDL_WaitEvent in most of its event loops */
static inline int snakeme_WaitEvent(SDL_Event *ev)
{
    int ret = SDL_WaitEvent(ev);
    if (ret) snakeme_scale_mouse_event(ev);
    return ret;
}

static inline int snakeme_PollEvent(SDL_Event *ev)
{
    int ret = SDL_PollEvent(ev);
    if (ret) snakeme_scale_mouse_event(ev);
    return ret;
}

/* #defines come AFTER the inline bodies so the bodies call the real SDL functions */
#define SDL_GetMouseState  snakeme_GetMouseState
#define SDL_WaitEvent      snakeme_WaitEvent
#define SDL_PollEvent      snakeme_PollEvent

#endif /* OS2COMPAT_H */
