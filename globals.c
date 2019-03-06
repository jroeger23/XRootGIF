#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#elif defined(HAVE_XINERAMA)
#include <X11/extensions/Xinerama.h>
#endif

int               screen_number;
Display           *display;
Window            root;
Colormap          cmap;
Visual            *visual;
XWindowAttributes root_attr;
Atom              prop_root_pmap;
#ifdef HAVE_XRANDR
XRRMonitorInfo *monitors;
#elif defined(HAVE_XINERAMA)
XineramaScreenInfo *screens;
#endif
int            num_monitors = 1;

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

enum image_fitting_t{
        scale_across_monitor,
        scale_per_monitor
};

struct {
        char *image;
        char *display;
        char *screen;
        double speed;
        char anti_alias;
        bool performance;
        double target_fps;
        bool do_test;
        bool daemon;
        enum image_fitting_t fitting;
} opts;
