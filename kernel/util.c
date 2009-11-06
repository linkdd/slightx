#include <stdarg.h>
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

int strlen (char *s)
{
	int i = 0;
	while (*s++) i++;
	return i;
}

int strncpy (char *dst, char *src, int n)
{
	int i = 0;
	while (n-- && (dst[i] = src[i++]));
	return i;
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

