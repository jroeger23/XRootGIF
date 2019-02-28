#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

extern int               screen_number;
extern Display           *display;
extern Window            root;
extern Colormap          cmap;
extern Visual            *visual;
extern XWindowAttributes root_attr;
extern Atom              prop_root_pmap;

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

extern struct {
        char *image;
        char *display;
        char *screen;
        double speed;
        char anti_alias;
        bool performance;
        double target_fps;
        bool do_test;
} opts;

#endif // __GLOBALS_H_
