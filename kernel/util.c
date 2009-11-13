#include <asm/asm.h>
#include <screen.h>
#include <util.h>

void printk (char *format, ...)
{
    int i, j, size, buflen, neg;
    va_list ap;
    char buf[16];

    unsigned char c;
    int ival;
    unsigned int uival;

    va_start (ap, format);

    while ((c = *format++))
    {
        size = neg = 0;

        if (c == 0)
            break;
        else if (c == '%')
        {
            c = *format++;
            if (c >= '0' && c <= '9')
            {
                size = c - '0';
                c = *format++;
            }

            if (c == 'd')
            {
                ival = va_arg (ap, int);
                if (ival < 0)
                {
                    uival = 0 - ival;
                    neg++;
                }
                else uival = ival;

                itoa (buf, uival, 10);
                buflen = strlen (buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0; --i, --j)
                        buf[i] = (j >= 0) ? buf[j] : '0';

                if (neg)
                    printk ("-%s", buf);
                else
                    printk (buf);
            }
            else if (c == 'u')
            {
                uival = va_arg (ap, int);
                itoa (buf, uival, 10);

                buflen = strlen (buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0; --i, --j)
                        buf[i] = (j >= 0) ? buf[j] : '0';

                printk (buf);
            }
            else if (c == 'x' || c == 'X')
            {
                uival = va_arg (ap, int);
                itoa (buf, uival, 16);

                buflen = strlen (buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0; --i, --j)
                        buf[i] = (j >= 0) ? buf[j] : '0';

                printk ("0x%s", buf);
            }
            else if (c == 'p')
            {
                uival = va_arg (ap, int);
                itoa (buf, uival, 16);
                size = 8;

                buflen = strlen (buf);
                if (buflen < size)
                    for (i = size, j = buflen; i >= 0; --i, --j)
                        buf[i] = (j >= 0) ? buf[j] : '0';

                printk ("0x%s", buf);
            }
            else if (c == 's')
            {
                printk ((char *) va_arg (ap, int));
            }
            else if (c == 'c')
            {
                putchar (va_arg (ap, int));
            }
        }
        else
            putchar (c);
    }
    return;
}

void itoa (char *buf, unsigned long int n, int base)
{
    unsigned long int tmp;
    int i = 0, j;

    do
    {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'A' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; ++j, --i)
    {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}

void panic (const char *file, int line, char *msg)
{
    extern char kattr;
    kattr = 0x0C;
    printk ("PANIC: %s:%d : %s\n", file, line, msg);
    halt ();
}

void panic_assert (const char *file, int line, char *msg)
{
    extern char kattr;
    kattr = 0x0C;
    printk ("ASSERTION-FAILED: %s:%d : %s\n", file, line, msg);
    halt ();
}

void memset (void *dest, char val, unsigned int len)
{
    char *p = (char *) dest;
    for (; len != 0; --len) *p++ = val;
}

void memcpy (void *dest, const void *src, unsigned int len)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

int strlen (char *s)
{
    int i = 0;
    while (*s++) i++;
    return i;
}

int strcmp (char *str1, char *str2)
{
    while (*str1 != 0 && *str2 != 0)
    {
        if (*str1 != *str2)
        {
            return str1 - str2;
        }

        str1++; str2++;
    }

    return 0;
}

char *strcpy (char *dest, const char *src)
{
    do {
        *dest++ = *src++;
    } while (*src != 0);
    return dest;
}

char *strncpy (char *dst, char *src, int n)
{
    int i = 0;
    while (n--)
    {
        dst[i] = src[i];
        i++;
    }

    return dst;
}
