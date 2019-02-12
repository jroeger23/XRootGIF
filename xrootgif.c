/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Jonas Röger
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <gif_lib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#define VERSION 1
#define EXIT_ON_ERROR 1

#define HELP_TEXT "" \
"XRootGIF 1.0\n" \
"A simple program to display GIFs as X root, targeting performance\n" \
"\n" \
"Usage: d:S:s:apt:Th [image]\n" \
"  -d | --display [display]\n" \
"       X-Display to use (:0), if none, use default display\n"\
"  -S | --screen [num]\n" \
"       X-Screen to use, if none, use default screen\n"\
"  -s | --speed [float]\n" \
"       Playback speed as float\n"\
"  -a | --anti-alias-off\n" \
"       Don't use anti aliasing (Only use with native resolution GIFs)\n"\
"  -p | --performance\n" \
"       Performance mode - scale framerate to 5 (default)\n"\
"  -t | --target-fps [float]\n" \
"       In performance mode, set target framerate\n"\
"  -T | --test-pattern\n" \
"       A little test pattern used for developing\n"\
"  -h | --help\n" \
"\n"\
"Performance:\n"\
"  This Program grew out of the pain, that most GIF-Viewer consume quiet\n"\
"  some CPU time, so having a GIF as wallpaper somewhat drained the battery.\n"\
"  XRootGIF tries to minimize CPU time used to display fancy GIFs,\n"\
"  by pre rendering all frames and allocating them in the X-Display instance.\n"\
"  Some GIFs may still make your PC heat your room, but this can be\n"\
"  avoided by using the performance mode, which will simply downscale\n"\
"  the framerate.\n"\

#define GRGBTOD32(grgb)(0 | grgb.Red << 16 | grgb.Green << 8 | grgb.Blue)

static int               screen_number;
static Display           *display;
static Window            root;
static Colormap          cmap;
static Visual            *visual;
static XWindowAttributes root_attr;
static Atom              prop_root_pmap;

static bool do_anim = false;

struct Background_frame{
        Pixmap p;
        unsigned int dur;
};

static struct {
        struct Background_frame *frames;
        unsigned int num;
        unsigned int cur;
} Background_anim;

static struct {
        char *image;
        char *display;
        char *screen;
        double speed;
        char anti_alias;
        bool performance;
        double target_fps;
        bool do_test;
} opts;

int unload_pixmaps();

void interrupt_handler(int i)
{
        puts("Interrupt signal catched!");
        do_anim = false;
}

int error_handler(Display *d, XErrorEvent *e)
{
        char error_str[1024];
        XGetErrorText(d, e->error_code, error_str, sizeof(error_str));
        error_str[sizeof(error_str)-1] = 0;
        printf("Error: %s\n", error_str);
        if(EXIT_ON_ERROR) {
                unload_pixmaps();
                exit(e->error_code);
        }
        return 0;
}

int prepare()
{
        int ret = 0;

        XSetErrorHandler(&error_handler);

        display = XOpenDisplay(opts.display);
        if(!display) {
                fputs("Could not open Display...\n", stderr);
                ret = 1;
                goto exit;
        }

        if(opts.screen)
                screen_number = atoi(opts.screen); // Safe, because invalid string retults in 0 => default screen
        else
                screen_number = DefaultScreen(display);

        root = RootWindow(display, screen_number);

        prop_root_pmap = XInternAtom(display, "_XROOTPMAP_ID", false);

        if(!XGetWindowAttributes(display, root, &root_attr)) {
                fputs("Could not get Window attributes...\n", stderr);
                ret = 2;
                goto exit;
        }

        cmap = DefaultColormap(display, screen_number);

        visual = DefaultVisual(display, screen_number);

exit:
        return ret;
}

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

        printf("\tDispose: ");

        switch(gcb->DisposalMode) {
        case DISPOSE_BACKGROUND:
                puts("BACKGROUND");
                c = GRGBTOD32(color);
                for(int y = 0; y < desc->Height; ++y) {
                        off = desc->Top*gif->SWidth + y*gif->SWidth + desc->Left;
                        for(int x = 0; x < desc->Width; ++x) {
                                canvas[off + x] = c;
                        }
                }
                break;
        case DISPOSE_PREVIOUS:
                puts("PREVIOUS");
                for(int y = 0; y < desc->Height; ++y) {
                        off = desc->Top*gif->SWidth + y*gif->SWidth + desc->Left;
                        for(int x = 0; x < desc->Width; ++x) {
                                canvas[off + x] = old_canvas[off + x];
                        }
                }
                break;
        case DISPOSAL_UNSPECIFIED: printf("UNSPECIFIED, falling back to ");
        case DISPOSE_DO_NOT: puts("NONE");
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

        printf("\tCurrent Colors: %d, Color_Total: %d\n", num, *color_total);

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
        Imlib_Image          img, img_scaled;
        Pixmap               pmap;
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

        /* Render each image */
        for(int i = 0; i < gif->ImageCount; ++i) {
                desc = gif->SavedImages[i].ImageDesc;
                DGifSavedExtensionToGCB(gif, i, &gcb);
                pmap = XCreatePixmap(display, root, root_attr.width,
                                     root_attr.height, root_attr.depth);
                XSync(display, false);

                /* Render image on canvas */
                render_image(gif, &gcb, &gif->SavedImages[i], canvas, &color_total);


                /* Render canvas on pixmap with imlib2 */
                img = imlib_create_image_using_data(gif->SWidth,
                                                    gif->SHeight,
                                                    canvas);
                imlib_context_set_image(img);
                img_scaled = imlib_create_cropped_scaled_image(
                        0, 0, gif->SWidth, gif->SHeight, root_attr.width,
                        root_attr.height);
                imlib_context_set_image(img_scaled);
                imlib_context_set_display(display);
                imlib_context_set_visual(visual);
                imlib_context_set_colormap(cmap);
                imlib_context_set_drawable(pmap);
                imlib_context_set_anti_alias(opts.anti_alias);
                imlib_context_set_dither(1);
                imlib_context_set_blend(1);
                imlib_context_set_angle(0);
                imlib_render_image_on_drawable(0, 0);
                imlib_free_image();
                imlib_context_set_image(img);
                imlib_free_image();

                /* Dispose image */
                dispose_image(gif, &gcb, &gif->SavedImages[i], canvas, old_canvas);
                memcpy(old_canvas, canvas, canvas_size);

                delay = (gcb.DelayTime) ? gcb.DelayTime : 1; // Min delay time
                Background_anim.frames[i].p = pmap;
                Background_anim.frames[((i==0) ? gif->ImageCount : i)-1].dur
                        = opts.speed*(10000*delay);
                avg_delay += delay;

                printf("Image %d -- Top: %d; Left, %d; Width: %d; Height: %d; Delay: %d; Interlace: %s\n", i,
                       desc.Top, desc.Left, desc.Width, desc.Height, gcb.DelayTime,
                       desc.Interlace ? "True" : "False");
        }

        avg_delay = 100.0/(avg_delay/gif->ImageCount);

        /* Scale to target performance */
        if(opts.performance && opts.target_fps < avg_delay) {
                opts.speed = avg_delay / opts.target_fps;
                for(int i = 0; i < Background_anim.num; ++i) {
                        Background_anim.frames[i].dur *= opts.speed;
                }
        }

        printf("Loaded GIF! - %d Frames; %dx%d; %d Colors; FPS: %f (Scaled: %f)\n",
               gif->ImageCount, gif->SWidth, gif->SHeight, color_total,
               avg_delay, avg_delay/opts.speed);

        do_anim = true;
        goto exit;
error:
        err = GifErrorString(ret);
        fprintf(stderr, "GIFLIB: %s\n", err);
exit:
        if(canvas) free(canvas);
        if(gif) DGifCloseFile(gif,&ret);
        return ret;
}

int unload_pixmaps()
{
        puts("Cleanung up...");
        for(int i = 0; i < Background_anim.num; ++i) {
                XFreePixmap(display, Background_anim.frames[i].p);
        }

        free(Background_anim.frames);

        return 0;
}

void anim_loop()
{
        struct Background_frame *f;

        Background_anim.cur = 0;

        while(do_anim) {
                f = &Background_anim.frames[Background_anim.cur];

                /* Used for pseudo-transparency */
                XChangeProperty(display, root, prop_root_pmap, XA_PIXMAP, 32,
                                PropModeReplace, (unsigned char *) &f->p, 1);

                XSetWindowBackgroundPixmap(display, root, f->p);
                XClearWindow(display, root);
		XFlush(display);
                usleep(f->dur);
                Background_anim.cur += 1;
                Background_anim.cur %= Background_anim.num;
        }
}

int parse_args(int argc, char **argv)
{
        double tmp;
        char c;
        int longind = 0;
        const char *optstring = "d:S:s:apt:Th";
        struct option longopts[] = {
                {"display", required_argument, NULL, 'd'},
                {"screen", required_argument, NULL, 'S'},
                {"speed", required_argument, NULL, 's'},
                {"anti-alias-off", no_argument, NULL, 'a'},
                {"performance", no_argument, NULL, 'p'},
                {"target-fps", required_argument, NULL, 't'},
                {"test-pattern", no_argument, NULL, 'T'},
                {"help", no_argument, NULL, 'h'},
                {NULL, no_argument, NULL, 0}
        };

        /* Defaults */
        opts.speed = 1.0;
        opts.anti_alias = 1;
        opts.target_fps = 5.0;
        opts.performance = false;
        opts.do_test = false;

        while( (c = getopt_long(argc, argv, optstring, longopts, &longind)) != -1) {
                switch(c) {
                case 'd':
                        opts.display = optarg;
                        break;
                case 'S':
                        opts.screen = optarg;
                        break;
                case 's':
                        tmp = atof(optarg);
                        if(tmp > 0.0)
                                opts.speed = 1.0/tmp;
                        break;
                case 'a':
                        opts.anti_alias = 0;
                        break;
                case 'p':
                        opts.performance = true;
                        break;
                case 't':
                        tmp = atof(optarg);
                        if(tmp> 0.0)
                                opts.target_fps = tmp;
                        break;
                case 'T':
                        opts.do_test = true;
                        break;
                case 'h':
                        puts(HELP_TEXT);
                        exit(0);
                }
        }

        if(optind < argc)
                opts.image = argv[optind];

        return 0;
}

int main(int argc, char **argv)
{
        signal(SIGINT, interrupt_handler);

        parse_args(argc, argv);

        if(prepare())
               return 1;

        if(opts.do_test)
                load_pixmap_sample();
        else
                load_pixmaps_from_image();

        anim_loop();

        unload_pixmaps();

        XCloseDisplay(display);

        return 0;
}
