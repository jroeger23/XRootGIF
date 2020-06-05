#include "pixmap_allocate.h"

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#elif defined(HAVE_XINERAMA)
#include <X11/extensions/Xinerama.h>
#endif

#include "globals.h"

void pmap_allocate_auto(Pixmap *pmap, DATA32 *data, int width, int height)
{
        if(num_monitors < 2) {
                /* per monitor is not needed */
                pmap_allocate_scale(pmap, data, width, height);
                return;
        }

        switch(opts.fitting) {
        case scale_per_monitor:

#if defined HAVE_XRANDR || defined(HAVE_XINERAMA) /* One of them must be installed */
                pmap_allocate_scale_per_monitor(pmap, data, width, height);
                return;
#endif

                /* fallthrough */
        case scale_across_monitor:
                pmap_allocate_scale(pmap, data, width, height);
                return;
        }
}

void pmap_allocate_scale(Pixmap *pmap, DATA32 *data, int width, int height)
{
        Imlib_Image img, img_scaled;

        /* Create pixmap */
        *pmap = XCreatePixmap(display, root, root_attr.width,
                              root_attr.height, root_attr.depth);
        XSync(display, false);

        /* Render data on pixmap with imlib2 */
        img = imlib_create_image_using_data(width, height, data);
        imlib_context_set_image(img);
        imlib_context_set_anti_alias(opts.anti_alias);
        img_scaled = imlib_create_cropped_scaled_image(
                0, 0, width, height, root_attr.width,
                root_attr.height);
        imlib_context_set_image(img_scaled);
        imlib_context_set_display(display);
        imlib_context_set_visual(visual);
        imlib_context_set_colormap(cmap);
        imlib_context_set_drawable(*pmap);
        imlib_context_set_anti_alias(opts.anti_alias);
        imlib_context_set_dither(1);
        imlib_context_set_blend(1);
        imlib_context_set_angle(0);
        imlib_render_image_on_drawable(0, 0);
        imlib_free_image();
        imlib_context_set_image(img);
        imlib_free_image();
}

void pmap_allocate_scale_per_monitor(Pixmap *pmap, DATA32 *data, int width, int height)
{
#if defined HAVE_XRANDR || defined(HAVE_XINERAMA)
        Imlib_Image img, img_scaled;

#ifdef HAVE_XRANDR
        XRRMonitorInfo *monitor;
#elif defined(HAVE_XINERAMA)
        XineramaScreenInfo *screen;
#endif

        /* Create pixmap */
        *pmap = XCreatePixmap(display, root, root_attr.width,
                              root_attr.height, root_attr.depth);
        XSync(display, false);

        /* load data as imlib2 img */
        img = imlib_create_image_using_data(width, height, data);
        imlib_context_set_image(img);
        imlib_context_set_anti_alias(opts.anti_alias);

        /* Render data on pixmap at each monitor's position with imlib2 */
        for(int i = 0; i < num_monitors; ++i) {

#ifdef HAVE_XRANDR
                monitor = &monitors[i];
#elif defined(HAVE_XINERAMA)
                screen = &screens[i];
#endif

                imlib_context_set_image(img);

#ifdef HAVE_XRANDR
                img_scaled = imlib_create_cropped_scaled_image(
                        0, 0, width, height, monitor->width,
                        monitor->height);
#elif defined(HAVE_XINERAMA)
                img_scaled = imlib_create_cropped_scaled_image(
                        0, 0, width, height, screen->width,
                        screen->height);
#endif

                imlib_context_set_image(img_scaled);
                imlib_context_set_display(display);
                imlib_context_set_visual(visual);
                imlib_context_set_colormap(cmap);
                imlib_context_set_drawable(*pmap);
                imlib_context_set_anti_alias(opts.anti_alias);
                imlib_context_set_dither(1);
                imlib_context_set_blend(1);
                imlib_context_set_angle(0);

#ifdef HAVE_XRANDR
                imlib_render_image_on_drawable(monitor->x, monitor->y);
#elif defined(HAVE_XINERAMA)
                imlib_render_image_on_drawable(screen->x_org, screen->y_org);
#endif

                imlib_free_image();
        }

        imlib_context_set_image(img);
        imlib_free_image();
#endif /* HAVE_XINERAMA or HAVE_XRANDR */
}
