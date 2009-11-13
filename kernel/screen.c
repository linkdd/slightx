#include <screen.h>

char kattr = 0x07;
char kx = 0;
char ky = 0;

static void move_hw_cursor (void)
{
    uint16_t cursor_location = ky * 80 + kx;
    outb (0x3D4, 14);                   /* Tell the VGA board we are setting the high cursor byte. */
    outb (0x3D5, cursor_location << 8); /* Send the high cursor byte */
    outb (0x3D4, 15);                   /* Tell the VGA board we are setting the low cursor byte. */
    outb (0x3D5, cursor_location);      /* Send the low cursor byte */
}

void clear (void)
{
    char *video = (char *) RAMSCREEN;
    int i;

    for (i = 0; i < SIZESCREEN; ++i)
        video[i] = 0;

    kx = ky = 0;
    return;
}

void scrollup (int n)
{
    char *video, *tmp;

    for (video = (char *) RAMSCREEN;
         video < (char *) SCREENLIM;
         video += 2)
    {
        tmp = (char *) (video + n * 160);

        if (tmp < (char *) SCREENLIM)
        {
            *video = *tmp;
            *(video + 1) = *(tmp + 1);
        }
        else
        {
            *video = 0;
            *(video + 1) = kattr;
        }
    }

    ky -= n;
    if (ky < 0) ky = 0;
    return;
}

void putchar (char c)
{
    char *video = (char *) (RAMSCREEN + 2 * kx + 160 * ky);

    switch (c)
    {
        case 10: /* CR-NL */
            kx = 0;
            ky++;
            break;

        case 8:  /* BS */
            if (kx)
            {
                *(video + 1) = 0;
                kx--;
            }
            break;

        case 9:  /* TAB */
            kx = kx + 4 - (kx % 4);
            break;

        case 13:  /* CR */
            kx = 0;
            break;

        default:
            *video = c;
            *(video + 1) = kattr;

            kx++;
            if (kx > 79)
            {
                kx = 0;
                ky++;
            }
            break;
    }

    if (ky > 24)
        scrollup (ky - 24);

    move_hw_cursor ();

    return;
}
