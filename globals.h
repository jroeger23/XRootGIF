#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

extern Display                  *display;
extern Atom                     prop_root_pmap;
extern struct Background_screen *screens;
extern unsigned int             num_screens;

extern bool do_anim;

/**
 * Properties of a used screen
 */
struct Background_screen {
        int screen_number;
        Window root;
        Colormap cmap;
        Visual *visual;
        XWindowAttributes root_attr;
};

/**
 * A pixmap with the screen to draw it on
 */
struct Background_screen_pmap {
        Pixmap p;
        struct Background_screen *s;
};

/**
 * Holds all pixmap-screen relations for the current frame
 */
struct Background_frame{
        /**
          *Shall hold num_screens structs
          */
        struct Background_screen_pmap *sp;
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
