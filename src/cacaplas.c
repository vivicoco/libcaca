/*
 *  cacaplas      plasma effect for libcaca
 *  Copyright (c) 2004 Sam Hocevar <sam@zoy.org>
 *                1998 Michele Bini <mibin@tin.it>
 *                All Rights Reserved
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the Do What The Fuck You Want To
 *  Public License, Version 2, as published by Sam Hocevar. See
 *  http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include "config.h"

#if !defined(__KERNEL__)
#   include <stdio.h>
#   include <math.h>
#   ifndef M_PI
#       define M_PI 3.14159265358979323846
#   endif
#endif

#include "cucul.h"
#include "caca.h"

/* Virtual buffer size */
#define XSIZ 256
#define YSIZ 256

#define TABLEX (XSIZ * 2)
#define TABLEY (YSIZ * 2)

static unsigned char screen[XSIZ * YSIZ];
static unsigned char table[TABLEX * TABLEY];

static void do_plasma(unsigned char *,
                      double, double, double, double, double, double);

int main (int argc, char **argv)
{
    cucul_t *qq, *qq2, *mask; caca_t *kk;
    unsigned int red[256], green[256], blue[256], alpha[256];
    double r[3], R[6];
    struct cucul_dither *dither;
    int i, x, y, frame = 0, pause = 0;

    qq = cucul_create(0, 0);
    if(!qq)
        return 1;
    kk = caca_attach(qq);
    if(!kk)
        return 1;

    caca_set_delay(kk, 20000);

    qq2 = cucul_create(cucul_get_width(qq), cucul_get_height(qq));
    mask = cucul_create(cucul_get_width(qq), cucul_get_height(qq));

    /* Fill various tables */
    for(i = 0 ; i < 256; i++)
        red[i] = green[i] = blue[i] = alpha[i] = 0;

    for(i = 0; i < 3; i++)
        r[i] = (double)(cucul_rand(1, 1000)) / 60000 * M_PI;

    for(i = 0; i < 6; i++)
        R[i] = (double)(cucul_rand(1, 1000)) / 10000;

    for(y = 0 ; y < TABLEY ; y++)
        for(x = 0 ; x < TABLEX ; x++)
    {
        double tmp = (((double)((x - (TABLEX / 2)) * (x - (TABLEX / 2))
                              + (y - (TABLEX / 2)) * (y - (TABLEX / 2))))
                      * (M_PI / (TABLEX * TABLEX + TABLEY * TABLEY)));

        table[x + y * TABLEX] = (1.0 + sin(12.0 * sqrt(tmp))) * 256 / 6;
    }

    /* Create a libcucul dither */
    dither = cucul_create_dither(8, XSIZ, YSIZ, XSIZ, 0, 0, 0, 0);

    /* Main loop */
    for(;;) 
    {
        struct caca_event ev;
        if(caca_get_event(kk, CACA_EVENT_KEY_PRESS, &ev, 0))
        {
            switch(ev.data.key.c)
            {
                case CACA_KEY_ESCAPE: goto end;
                case ' ': pause = !pause;
            }
        }

        if(pause)
            goto paused;

        for(i = 0 ; i < 256; i++)
        {
            double z = ((double)i) / 256 * 6 * M_PI;

            red[i] = (1.0 + sin(z + r[1] * frame)) / 2 * 0xfff;
            blue[i] = (1.0 + cos(z + r[0] * frame)) / 2 * 0xfff;
            green[i] = (1.0 + cos(z + r[2] * frame)) / 2 * 0xfff;
        }

        /* Set the palette */
        cucul_set_dither_palette(dither, red, green, blue, alpha);

        do_plasma(screen,
                  (1.0 + sin(((double)frame) * R[0])) / 2,
                  (1.0 + sin(((double)frame) * R[1])) / 2,
                  (1.0 + sin(((double)frame) * R[2])) / 2,
                  (1.0 + sin(((double)frame) * R[3])) / 2,
                  (1.0 + sin(((double)frame) * R[4])) / 2,
                  (1.0 + sin(((double)frame) * R[5])) / 2);
        frame++;

paused:
        cucul_dither_bitmap(qq, 0, 0,
                            cucul_get_width(qq) - 1, cucul_get_height(qq) - 1,
                            dither, screen);

        cucul_blit(qq2, 0, 0, qq, NULL);
        cucul_invert(qq2);


        cucul_blit(qq, 0, 0, qq2, mask);

        cucul_set_color(qq, CUCUL_COLOR_WHITE, CUCUL_COLOR_BLUE);
        cucul_putstr(qq, cucul_get_width(qq) - 30, cucul_get_height(qq) - 2,
                     " -=[ Powered by libcaca ]=- ");
        caca_display(kk);
    }

end:
    cucul_free_dither(dither);
    caca_detach(kk);
    cucul_free(qq);

    return 0;
}

static void do_plasma(unsigned char *pixels, double x_1, double y_1,
                      double x_2, double y_2, double x_3, double y_3)
{
    unsigned int X1 = x_1 * (TABLEX / 2),
                 Y1 = y_1 * (TABLEY / 2),
                 X2 = x_2 * (TABLEX / 2),
                 Y2 = y_2 * (TABLEY / 2),
                 X3 = x_3 * (TABLEX / 2),
                 Y3 = y_3 * (TABLEY / 2);
    unsigned int y;
    unsigned char * t1 = table + X1 + Y1 * TABLEX,
                  * t2 = table + X2 + Y2 * TABLEX,
                  * t3 = table + X3 + Y3 * TABLEX;

    for(y = 0; y < YSIZ; y++)
    {
        unsigned int x;
        unsigned char * tmp = pixels + y * YSIZ;
        unsigned int ty = y * TABLEX, tmax = ty + XSIZ;
        for(x = 0; ty < tmax; ty++, tmp++)
            tmp[0] = t1[ty] + t2[ty] + t3[ty];
    }
}

