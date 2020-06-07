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
#include "globals.h"
#include "output.h"
#include "sample.h"
#include "gif.h"
#include "daemon.h"

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

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#define VERSION "1.4.2"
#define EXIT_ON_ERROR 1
#define HELP_TEXT "" \
"XRootGIF "VERSION"\n" \
"A simple program for setting animated wallpapers, targeting performance\n" \
"\n" \
"Usage: d:S:s:apt:TqQh [image]\n" \
"  -d | --display [display]\n" \
"       X-Display to use (:0), if none, use default display\n"\
"  -S | --screen [num]\n" \
"       X-Screen to use, if none, use default screen\n"\
"  -s | --speed [float]\n" \
"       Playback speed as float\n"\
"  -a | --anti-alias\n" \
"       Use anti-aliasing\n"\
"  -p | --performance\n" \
"       Performance mode - scale framerate to 5 (default)\n"\
"  -t | --target-fps [float]\n" \
"       In performance mode, set target framerate\n"\
"  -T | --test-pattern\n" \
"       A little test pattern used for developing\n"\
"  -q | --quiet\n" \
"       Only print basic information\n"\
"  -Q | --Quiet\n" \
"       No output\n"\
"  --scale-per-monitor\n" \
"       Scales the image on each monitor (default)\n"\
"  --scale-across-monitor\n" \
"       Scales the image across all monitors\n"\
"  --daemon\n" \
"       After loading the image, fork of a daemon and exit\n"\
"  --version\n" \
"       Print version\n" \
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

static int cleanup();

void interrupt_handler(int i)
{
        sprint("Interrupt signal catched!", verbose);
        do_anim = false;
}

int error_handler(Display *d, XErrorEvent *e)
{
        char error_str[1024];
        XGetErrorText(d, e->error_code, error_str, sizeof(error_str));
        error_str[sizeof(error_str)-1] = 0;
        eformat(normal, "Error: %s\n", error_str);
        if(EXIT_ON_ERROR) {
                cleanup();
                exit(e->error_code);
        }
        return 0;
}

static int prepare()
{
        int ret = 0;

        XSetErrorHandler(&error_handler);

        display = XOpenDisplay(opts.display);
        if(!display) {
                eprintln("Could not open Display...", normal);
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
                eprintln("Could not get Window attributes...", normal);
                ret = 2;
                goto exit;
        }

#ifdef HAVE_XRANDR
        monitors = XRRGetMonitors(display, root, true, &num_monitors);
        sformat(verbose,"Detected %d active XRandR monitors\n", num_monitors);
#endif

        cmap = DefaultColormap(display, screen_number);

        visual = DefaultVisual(display, screen_number);

exit:
        return ret;
}

static int cleanup()
{
        sprintln("Cleanung up...", verbose);
        for(int i = 0; i < Background_anim.num; ++i) {
                XFreePixmap(display, Background_anim.frames[i].p);
        }

        free(Background_anim.frames);

#ifdef HAVE_XRANDR
        if(monitors) XRRFreeMonitors(monitors);
#endif

        XCloseDisplay(display);

        return 0;
}

static void anim_loop()
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

static int parse_args(int argc, char **argv)
{
        double tmp;
        char c;
        int longind = 0;
        const char *optstring = "d:S:s:apt:TqQh";
        struct option longopts[] = {
                {"display", required_argument, NULL, 'd'},
                {"screen", required_argument, NULL, 'S'},
                {"speed", required_argument, NULL, 's'},
                {"anti-alias", no_argument, NULL, 'a'},
                {"performance", no_argument, NULL, 'p'},
                {"target-fps", required_argument, NULL, 't'},
                {"test-pattern", no_argument, NULL, 'T'},
                {"quiet", no_argument, NULL, 'q'},
                {"Quiet", no_argument, NULL, 'Q'},
                {"help", no_argument, NULL, 'h'},
                {"scale-per-monitor", no_argument, NULL, -127},
                {"scale-across-monitor", no_argument, NULL, -126},
                {"daemon", no_argument, NULL, -125},
                {"version", no_argument, NULL, -124},
                {NULL, no_argument, NULL, 0}
        };

        /* Defaults */
        opts.speed = 1.0;
        opts.anti_alias = 0;
        opts.target_fps = 5.0;
        opts.performance = false;
        opts.do_test = false;
        opts.daemon = false;
        opts.fitting = scale_per_monitor;

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
                        opts.anti_alias = 1;
                        break;
                case 'p':
                        opts.performance = true;
                        break;
                case 't':
                        tmp = atof(optarg);
                        if(tmp > 0.0)
                                opts.target_fps = tmp;
                        break;
                case 'T':
                        opts.do_test = true;
                        break;
                case 'q':
                        output.level = warn;
                        break;
                case 'Q':
                        output.level = none;
                        break;
                case 'h':
                        sprintln(HELP_TEXT, normal);
                        exit(0);
                case -127: /* scale-per-monitor */
                        opts.fitting = scale_per_monitor;

                        /* only notify when wanted explicitly */
#if !defined(HAVE_XRANDR)
                        sprintln("XRandR extension missing, cannot scale per monitor"
                                 " falling back to scale across monitors...", warn);
#endif

                        break;
                case -126: /* scale-across-monitor */
                        opts.fitting = scale_across_monitor;
                        break;
                case -125: /* daemon */
                        opts.daemon = true;
                        break;
                case -124: /* version */
                        sprintln("XRootGIF - "VERSION, normal);
                        exit(0);
                        break;
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

        if(opts.daemon) {
                daemon_task_t *tasks[] = {
                        anim_loop,
                        (daemon_task_t*)cleanup,
                        NULL
                };
                void *task_argv[] = {
                        NULL, NULL
                };
                return daemon_run(tasks, task_argv);
        } else {
                anim_loop();

                cleanup();
        }

        return 0;
}
