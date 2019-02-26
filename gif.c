#include "gif.h"
#include "output.h"
#include "globals.h"
#include "pixmap_allocate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gif_lib.h>

#define GRGBTOD32(grgb)(0 | grgb.Red << 16 | grgb.Green << 8 | grgb.Blue)

static void dispose_image(GifFileType *gif, GraphicsControlBlock *gcb,
                          SavedImage *img, DATA32 *canvas, DATA32 *old_canvas)
{
        int          off;
        GifImageDesc *desc;
        DATA32       c;
        GifColorType color;

        desc = &img->ImageDesc;

        /* Select background color */
        if(gif->SColorMap && gif->SColorMap->ColorCount > gif->SBackGroundColor) {
                color = gif->SColorMap->Colors[gif->SBackGroundColor];
        }else {
                color = (GifColorType){0, 0, 0};
        }

        sprint("\tDispose: ", verbose);

        switch(gcb->DisposalMode) {
        case DISPOSE_BACKGROUND:
                sprintln("BACKGROUND", verbose);
                c = GRGBTOD32(color);
                for(int y = 0; y < desc->Height; ++y) {
                        off = desc->Top*gif->SWidth + y*gif->SWidth + desc->Left;
                        for(int x = 0; x < desc->Width; ++x) {
                                canvas[off + x] = c;
                        }
                }
                break;
        case DISPOSE_PREVIOUS:
                sprintln("BACKGROUND", verbose);
                for(int y = 0; y < desc->Height; ++y) {
                        off = desc->Top*gif->SWidth + y*gif->SWidth + desc->Left;
                        for(int x = 0; x < desc->Width; ++x) {
                                canvas[off + x] = old_canvas[off + x];
                        }
                }
                break;
        case DISPOSAL_UNSPECIFIED: sprint("UNSPECIFIED, falling back to ", verbose);
        case DISPOSE_DO_NOT: sprintln("NONE", verbose);
        }
}

static void render_image(GifFileType *gif, GraphicsControlBlock *gcb, SavedImage *img,
                         DATA32 *canvas, int *color_total)
{
        int          num;
        int          off, roff;
        GifByteType  *raster;
        GifImageDesc *desc;
        DATA32       c;
        GifColorType color;
        GifByteType  color_code;


        raster = img->RasterBits;
        desc = &img->ImageDesc;

        if(desc->ColorMap)
                *color_total += (num = desc->ColorMap->ColorCount);
        else
                num = gif->SColorMap->ColorCount;
        if(!*color_total) *color_total = num;

        sformat(verbose, "\tCurrent Colors: %d, Color_Total: %d\n", num, *color_total);

        /* Select background color */
        if(gif->SColorMap && gif->SColorMap->ColorCount > gif->SBackGroundColor) {
                color = gif->SColorMap->Colors[gif->SBackGroundColor];
        }else {
                color = (GifColorType){0, 0, 0};
        }

        for(int y = 0; y < desc->Height; ++y) {
                off = desc->Top*gif->SWidth + y*gif->SWidth + desc->Left;
                for(int x = 0; x < desc->Width; ++x) {
                        roff = y*desc->Width;
                        color_code = raster[roff + x];
                        if(desc->ColorMap && desc->ColorMap->ColorCount > color_code)
                                color = desc->ColorMap->Colors[color_code];
                        else if(gif->SColorMap && gif->SColorMap->ColorCount > color_code)
                                color = gif->SColorMap->Colors[color_code];
                        else /* this should not happen */
                                color = (GifColorType){0, 0, 0};
                        c = GRGBTOD32(color);
                        if(gcb->TransparentColor == -1
                           || gcb->TransparentColor != color_code)
                                canvas[off + x] = c;
                }
        }
}

int load_pixmaps_from_image()
{
        int                  ret;
        int                  color_total = 0;
        double               avg_delay = 0;
        int                  delay;
        const char           *err;
        GifFileType          *gif = NULL;
        size_t               canvas_size;
        DATA32               *canvas = NULL;
        DATA32               *old_canvas = NULL;
        GraphicsControlBlock gcb;
        GifImageDesc         desc;

        gif = DGifOpenFileName(opts.image, &ret);
        if(!gif) goto error;

        ret = DGifSlurp(gif);
        if(!gif) goto error;

        Background_anim.num = gif->ImageCount;
        Background_anim.frames = malloc(sizeof(struct Background_frame)
                                        * gif->ImageCount);

        canvas_size = sizeof(DATA32) * gif->SWidth * gif->SHeight;
        canvas      = malloc(canvas_size);
        old_canvas  = malloc(canvas_size);

        sprintln("Loading GIF...", normal);

        /* Render each image */
        for(int i = 0; i < gif->ImageCount; ++i) {
                desc = gif->SavedImages[i].ImageDesc;
                DGifSavedExtensionToGCB(gif, i, &gcb);

                /* set delay time */
                delay = (gcb.DelayTime) ? gcb.DelayTime : 1; // Min delay time
                Background_anim.frames[((i==0) ? gif->ImageCount : i)-1].dur
                        = opts.speed*(10000*delay);
                avg_delay += delay;

                sformat(verbose, "Image %d -- Top: %d; Left, %d; Width: %d; Height: %d; Delay: %d; Interlace: %s\n", i,
                        desc.Top, desc.Left, desc.Width, desc.Height, gcb.DelayTime,
                        desc.Interlace ? "True" : "False");

                render_image(gif, &gcb, &gif->SavedImages[i], canvas, &color_total);

                pmap_allocate_auto(&Background_anim.frames[i].p,
                                   canvas, gif->SWidth, gif->SHeight);

                /* Dispose image */
                dispose_image(gif, &gcb, &gif->SavedImages[i], canvas, old_canvas);
                memcpy(old_canvas, canvas, canvas_size);
        }

        avg_delay = 100.0/(avg_delay/gif->ImageCount);

        /* Scale to target performance */
        if(opts.performance && opts.target_fps < avg_delay) {
                opts.speed = avg_delay / opts.target_fps;
                for(int i = 0; i < Background_anim.num; ++i) {
                        Background_anim.frames[i].dur *= opts.speed;
                }
        }

        sformat(normal, "Loaded GIF! - %d Frames; %dx%d; %d Colors; FPS: %f (Scaled: %f)\n",
                gif->ImageCount, gif->SWidth, gif->SHeight, color_total,
                avg_delay, avg_delay/opts.speed);

        do_anim = true;
        goto exit;
error:
        err = GifErrorString(ret);
        eformat(normal, "GIFLIB: %s\n", err);
exit:
        if(canvas) free(canvas);
        if(gif) DGifCloseFile(gif,&ret);
        return ret;
}
