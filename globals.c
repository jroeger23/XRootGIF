#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

Display                  *display;
Atom                     prop_root_pmap;
struct Background_screen *screens;
unsigned int             num_screens;

bool do_anim = false;

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

struct Background_frame{
        struct Background_screen_pmap *sp;
        unsigned int dur;
};

struct {
        struct Background_frame *frames;
        unsigned int num;
        unsigned int cur;
} Background_anim;

struct {
        char *image;
        char *display;
        char *screen;
        double speed;
        char anti_alias;
        bool performance;
        double target_fps;
        bool do_test;
} opts;
