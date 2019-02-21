#include "sample.h"
#include "globals.h"

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int load_pixmap_sample()
{
        XColor c1, c2, c3, c4;
        GC gc;
        XGCValues gcv;

        Background_anim.num = 4;
        Background_anim.frames = malloc(sizeof(struct Background_frame)
                                        * Background_anim.num);

        XAllocNamedColor(display, cmap, "blue", &c1, &c1);
        XAllocNamedColor(display, cmap, "green", &c2, &c2);
        XAllocNamedColor(display, cmap, "red", &c3, &c3);
        XAllocNamedColor(display, cmap, "black", &c4, &c4);

        Background_anim.frames[0].dur = 300000;
        Background_anim.frames[1].dur = 300000;
        Background_anim.frames[2].dur = 300000;
        Background_anim.frames[3].dur = 300000;

        gcv.foreground = c1.pixel;
        gc = XCreateGC(display, root, GCForeground, &gcv);
        Background_anim.frames[0].p = XCreatePixmap(display, root,
                                                    root_attr.width,
                                                    root_attr.height,
                                                    root_attr.depth);
        XFillRectangle(display, Background_anim.frames[0].p,
                       gc, 0, 0, root_attr.width, root_attr.height);

        gcv.foreground = c2.pixel;
        XChangeGC(display , gc, GCForeground, &gcv);
        Background_anim.frames[1].p = XCreatePixmap(display, root,
                                                    root_attr.width,
                                                    root_attr.height,
                                                    root_attr.depth);
        XFillRectangle(display, Background_anim.frames[1].p,
                       gc, 0, 0, root_attr.width, root_attr.height);

        gcv.foreground = c3.pixel;
        XChangeGC(display , gc, GCForeground, &gcv);
        Background_anim.frames[2].p = XCreatePixmap(display, root,
                                                    root_attr.width,
                                                    root_attr.height,
                                                    root_attr.depth);
        XFillRectangle(display, Background_anim.frames[2].p,
                       gc, 0, 0, root_attr.width, root_attr.height);

        gcv.foreground = c4.pixel;
        gc = XCreateGC(display, root, GCForeground, &gcv);
        Background_anim.frames[3].p = XCreatePixmap(display, root,
                                                    root_attr.width,
                                                    root_attr.height,
                                                    root_attr.depth);
        XFillRectangle(display, Background_anim.frames[3].p,
                       gc, 0, 0, root_attr.width, root_attr.height);

        XFreeGC(display, gc);

        do_anim = true;

        return 0;
}
