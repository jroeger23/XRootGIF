#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int               screen_number;
Display           *display;
Window            root;
Colormap          cmap;
Visual            *visual;
XWindowAttributes root_attr;
Atom              prop_root_pmap;

bool do_anim = false;

struct Background_frame{
        Pixmap p;
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
