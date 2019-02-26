#include "pixmap_allocate.h"

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>

#include "globals.h"

void pmap_allocate_auto(Pixmap *pmap, DATA32 *data, int width, int height)
{
        pmap_allocate_crop(pmap, data, width, height);
}

void pmap_allocate_crop(Pixmap *pmap, DATA32 *data, int width, int height)
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
