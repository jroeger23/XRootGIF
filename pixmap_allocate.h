#ifndef __PIXMAP_ALLOCATE_H_
#define __PIXMAP_ALLOCATE_H_

#include "globals.h"

#include <X11/Xlib.h>
#include <Imlib2.h>

/**
 * Allocate data as pixmaps across screens, according to the selected fitting mode.
 * Memory will be allocated in sp and must be freed when no longer needed
 */
void pmap_allocate_auto(struct Background_screen_pmap **sp, DATA32 *data, int width, int height);

/**
 * @see pmap_allocate_auto
 * Scale data to pixmap on each screen
 */
void pmap_allocate_crop(struct Background_screen_pmap **sp, DATA32 *data, int width, int height);

#endif // __PIXMAP_ALLOCATE_H_
