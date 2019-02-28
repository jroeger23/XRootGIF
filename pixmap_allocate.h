#ifndef __PIXMAP_ALLOCATE_H_
#define __PIXMAP_ALLOCATE_H_

#include <X11/Xlib.h>
#include <Imlib2.h>

void pmap_allocate_auto(Pixmap *pmap, DATA32 *data, int width, int height);

void pmap_allocate_scale(Pixmap *pmap, DATA32 *data, int width, int height);

#endif // __PIXMAP_ALLOCATE_H_
