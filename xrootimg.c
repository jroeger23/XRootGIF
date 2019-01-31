#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <Imlib2.h>
#include <gif_lib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#define EXIT_ON_ERROR 1
#define GRGBTOD32(grgb)(grgb.Red << 16 | grgb.Green << 8 | grgb.Blue)

static Display *display;
static Window root;
static int screen_number;
static Colormap cmap;
static XWindowAttributes root_attr;
static Atom prop_root_pmap;

static volatile bool do_anim = true;

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
        XSetErrorHandler(&error_handler);

        display = XOpenDisplay(opts.display);
        if(!display) {
                fputs("Could not open Display...\n", stderr);
                goto display;
        }

        if(opts.screen)
                screen_number = atoi(opts.screen); // Safe, because invalid string retults in 0 => default screen
        else
                screen_number = DefaultScreen(display);

        root = RootWindow(display, screen_number);

        prop_root_pmap = XInternAtom(display, "_XROOTPMAP_ID", false);

        XGetWindowAttributes(display, root, &root_attr);

        cmap = DefaultColormap(display, screen_number);

        return 0;
display:
        return 1;
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

        return 0;
}

static void dispose_image(GifFileType *gif, GraphicsControlBlock *gcb, SavedImage *img, DATA32 *canvas)
{
        if(gcb->DisposalMode == DISPOSE_DO_NOT) return;

        GifByteType *raster;
        GifImageDesc *desc;
        DATA32 c;
        GifColorType color;
        GifByteType color_code;

        raster = img->RasterBits;
        desc = &img->ImageDesc;

        if(gcb->DisposalMode == DISPOSE_BACKGROUND) {
                color = gif->SColorMap->Colors[gif->SBackGroundColor];
                c = GRGBTOD32(color);
                for(int y = 0; desc->Height; ++y) {
                        for(int x = 0; x < desc->Width; ++x) {
                                int off = desc->Top*y*gif->SWidth + desc->Left;
                                canvas[off + x] = c;
                        }
                }
        } else if(gcb->DisposalMode == DISPOSAL_UNSPECIFIED) {
                for(int y = 0; desc->Height; ++y) {
                        for(int x = 0; x < desc->Width; ++x) {
                                int off = desc->Top*y*gif->SWidth + desc->Left;
                                int yoff = y*desc->Width;
                                color_code = raster[yoff + x];
                                color = desc->ColorMap->Colors[color_code];
                                c = GRGBTOD32(color);
                                if(gcb->TransparentColor == -1 || gcb->TransparentColor != color_code)
					canvas[off + x] = c;
                        }
                }
        }
}

int load_pixmaps_from_image()
{
        int ret;
        const char *err;
        GifFileType *gif = NULL;
        DATA32 *canvas = NULL;
        Imlib_Image img, img_scaled;

        gif = DGifOpenFileName(opts.image, &ret);
        if(!gif) goto error;

        ret = DGifSlurp(gif);
        if(!gif) goto error;

        printf("GIF - Frames: %d; Width: %d; Height: %d\n",
               gif->ImageCount, gif->SWidth, gif->SHeight);

        Background_anim.num = gif->ImageCount;
        Background_anim.frames = malloc(sizeof(struct Background_frame)
                                        * gif->ImageCount);

        canvas = malloc(sizeof(DATA32) * gif->SWidth * gif->SHeight);

        /* Render each image */
        for(int i = 0; i < gif->ImageCount; ++i) {
                ExtensionBlock *ext;
                GraphicsControlBlock *gcb = NULL;
                GifImageDesc desc = gif->SavedImages[i].ImageDesc;

                printf("Image %d -- Top: %d; Left, %d; Width: %d; Height: %d; Interlace: %s\n", i,
                       desc.Top, desc.Left, desc.Width, desc.Height, desc.Interlace ? "True" : "False");

                /* Search for the Graphics Control Block */
                for(int j = 0; j < gif->SavedImages[i].ExtensionBlockCount; ++j) {
                        ext = &gif->SavedImages[i].ExtensionBlocks[j];
                        if(ext->Function == GRAPHICS_EXT_FUNC_CODE) {
                                gcb = (GraphicsControlBlock*)ext;
                                break;
                        }
                }

                if(!gcb) {
                        // TODO: Handle absence of GCB
                }

                dispose_image(gif, gcb, &gif->SavedImages[i], canvas);

                Background_anim.frames[i].dur = 10000*gcb->DelayTime;
                Background_anim.frames[i].p = XCreatePixmap(display, root, root_attr.width,
                                                            root_attr.height, root_attr.depth);

                /* Render canvas on pixmap with imlib2 */
                img = imlib_create_image_using_data(gif->SWidth, gif->SHeight, canvas);
                imlib_context_set_image(img);
                img_scaled = imlib_create_cropped_scaled_image(0, 0, gif->SWidth, gif->SHeight, root_attr.width, root_attr.height);
                imlib_context_set_image(img_scaled);
                imlib_context_set_drawable(Background_anim.frames[i].p);
                imlib_context_set_anti_alias(0);
                imlib_context_set_dither(1);
                imlib_context_set_blend(1);
                imlib_context_set_angle(0);
                imlib_render_image_on_drawable(0, 0); // TODO: Segfaults for a reason
                imlib_context_set_image(img);
                imlib_free_image();
                imlib_context_set_image(img_scaled);
                imlib_free_image();
        }

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
        for(int i = 0; i < Background_anim.num; ++i) {
                XFreePixmap(display, Background_anim.frames[i].p);
                puts("Cleaned");
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
        char c;
        int longind = 0;
        const char *optstring = "i:d:s:";
        struct option longopts[] = {
                {"image", required_argument, NULL, 'i'},
                {"display", required_argument, NULL, 'd'},
                {"screen", required_argument, NULL, 's'},
                {NULL, no_argument, NULL, 0}
        };

        while( (c = getopt_long(argc, argv, optstring, longopts, &longind)) != -1) {
                switch(c) {
                case 'i':
                        opts.image = optarg;
                        break;
                case 'd':
                        opts.display = optarg;
                        break;
                case 's':
                        opts.screen = optarg;
                        break;
                }
        }

        return 0;
}

int main(int argc, char **argv)
{
        signal(SIGINT, interrupt_handler);

        parse_args(argc, argv);

        if(prepare())
               return 1;

        load_pixmaps_from_image();

        // anim_loop();

        unload_pixmaps();

        XCloseDisplay(display);

        return 0;
}
