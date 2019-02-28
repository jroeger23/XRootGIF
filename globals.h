#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#elif defined(HAVE_XINERAMA)
#include <X11/extensions/Xinerama.h>
#endif

extern int               screen_number;
extern Display           *display;
extern Window            root;
extern Colormap          cmap;
extern Visual            *visual;
extern XWindowAttributes root_attr;
extern Atom              prop_root_pmap;
#ifdef HAVE_XRANDR
extern XRRMonitorInfo *monitors;
#elif defined(HAVE_XINERAMA)
extern XineramaScreenInfo *screens;
#endif
/**
 * Number of detected Xinerama screens or XRandR monitors
 */
extern int            num_monitors;

extern bool do_anim;

struct Background_frame{
        Pixmap p;
        unsigned int dur;
};

extern struct {
        struct Background_frame *frames;
        unsigned int num;
        unsigned int cur;
} Background_anim;

enum image_fitting_t{
        scale_across_monitor,
        scale_per_monitor
};

extern struct {
        char *image;
        char *display;
        char *screen;
        double speed;
        char anti_alias;
        bool performance;
        double target_fps;
        bool do_test;
        enum image_fitting_t fitting;
} opts;

#endif // __GLOBALS_H_
