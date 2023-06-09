/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

// #include <stdarg.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/kernel.h>

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')


static int skip_atoi(const char **s)
{
	int i = 0;

	while(is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

/*
# define do_div(n,base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
 })
*/

static uint32_t do_div(long *n, long base)
{
	uint32_t d = ((uint32_t)(*n)) / ((uint32_t)base);
	uint32_t r = ((uint32_t)(*n)) % ((uint32_t)base);
	*n = (long)d;
	return r;
}

static char * number(char * str, long num, long base, int size, int precision
					 , int type)
{
	static const char *const ucase = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const char *const lcase = "0123456789abcdefghijklmnopqrstuvwxyz";
	char sign = '\0', tmp[36];
	const char *digits =  (type&SMALL)  ? lcase : ucase;

	if(type&LEFT)
	{
		type &= ~ZEROPAD;
	}

	if(base < 2 || base>36)
	{
		return 0;
	}

	char c = (type & ZEROPAD) ? '0' : ' ';
	if( (type&SIGN) && (num < 0) )
	{
		sign = '-';
		num = -num;
	}
	else if( type&PLUS )
	{
		sign = '+';
	}
	else if( type&SPACE )
	{
		sign = ' ';
	}

	if(sign)
	{
		size--;
	}

	if(type&SPECIAL)
	{
		if(base == 16)
		{
			size -= 2;
		}
		else if(base == 8)
		{
			size--;
		}
	}
	int i = 0;
	if(num == 0)
	{
		tmp[i++] = '0';
	}
	else
	{
		while(num != 0)
		{
			size_t index = do_div(&num, base);
			tmp[i++] = digits[index];
		}
	}

	if(i > precision)
	{
		precision = i;
	}

	size -= precision;
	if(!(type&(ZEROPAD + LEFT)))
	{
		while(size-- > 0)
		{
			*str++ = ' ';
		}
	}

	if(sign)
	{
		*str++ = sign;
	}

	if(type&SPECIAL)
	{
		if(base == 8)
		{
			*str++ = '0';
		}
		else if(base == 16)
		{
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if(!(type&LEFT))
	{
		while(size-- > 0)
		{
			*str++ = c;
		}
	}

	while(i < precision--)
	{
		*str++ = '0';
	}

	while(i-- > 0)
	{
		*str++ = tmp[i];
	}

	while(size-- > 0)
	{
		*str++ = ' ';
	}

	return str;
}

static
unsigned long
get_integer(int qualifier, va_list args)
{
	unsigned long n = 0;

	switch(qualifier)
	{
		case 'l':
			n = (unsigned long)(va_arg(args, unsigned long));
			break;
		case 'h':
			n = (unsigned long)(va_arg(args, unsigned int));
			break;
		case (('l' << 8) | 'l'):
			n = (unsigned long)(va_arg(args, unsigned long long));
			break;
		case (('h' << 8) | 'h'):
			n = (unsigned long)(va_arg(args, unsigned int));
			break;
		default:
			n = va_arg(args, unsigned long);
			break;
	}

	return n;
}

static
int vsprintf_imp(char *buf, const char *fmt, va_list args)
{
	int i=0;
	char * str=NULL;

	for(str = buf; *fmt; ++fmt)
	{
		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		int flags = 0;
	repeat:
		++fmt;		/* this also skips first '%' */
		switch(*fmt)
		{
		case '-': flags |= LEFT; goto repeat;
		case '+': flags |= PLUS; goto repeat;
		case ' ': flags |= SPACE; goto repeat;
		case '#': flags |= SPECIAL; goto repeat;
		case '0': flags |= ZEROPAD; goto repeat;
		}

		/* get field width */
		int field_width = -1;
		if(is_digit(*fmt))
		{
			field_width = skip_atoi(&fmt);
		}
		else if(*fmt == '*')
		{
			/* it's the next argument */
			field_width = va_arg(args, int);
			if(field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		int precision = -1;
		if(*fmt == '.')
		{
			++fmt;
			if(is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if(precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		int qualifier = -1;
		if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
		{
			qualifier = *fmt;
			++fmt;
			if(qualifier == 'h' && *fmt == 'h')
			{
				qualifier = (((qualifier & 0xFF) << 8) | 'h');
				++fmt;
			}
			else if(qualifier == 'l' && *fmt == 'l')
			{
				qualifier = (((qualifier & 0xFF) << 8) | 'l');
				++fmt;
			}
		}

		switch(*fmt)
		{
		case 'c':
			if(!(flags & LEFT))
			{
				while(--field_width > 0)
				{
					*str++ = ' ';
				}
			}

			*str++ = (unsigned char)va_arg(args, int);
			while(--field_width > 0)
			{
				*str++ = ' ';
			}
			break;

		case 's':
		{
			char *s = va_arg(args, char *);
			int len = (int)(strlen(s));
			if(precision < 0)
			{
				precision = len;
			}
			else if(len > precision)
			{
				len = precision;
			}

			if(!(flags & LEFT))
			{
				while(len < field_width--)
				{
					*str++ = ' ';
				}
			}
			for(i = 0; i < len; ++i)
			{
				*str++ = *s++;
			}
			while(len < field_width--)
			{
				*str++ = ' ';
			}
			break;
		}
		case 'o':
		{
			unsigned long n = get_integer(qualifier, args);
			str = number(str, n, 8, field_width, precision, flags);
			break;
		}
		case 'p':
			if(field_width == -1)
			{
				field_width = 16;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long)va_arg(args, void *), 16,
						 field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
		{
			unsigned long n = get_integer(qualifier, args);
			str = number(str, n, 16, field_width, precision, flags);
			break;
		}

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
		{
			unsigned long n = get_integer(qualifier, args);
			str = number(str, n, 10, field_width, precision, flags);
			break;
		}
		case 'n':
		{
			long *ip = va_arg(args, long *);
			*ip = (str - buf);
			break;
		}
		default:
			if(*fmt != '%')
			{
				*str++ = '%';
			}
			if(*fmt)
			{
				*str++ = *fmt;
			}
			else
			{
				--fmt;
			}
			break;
		}
	}
	*str = '\0';
	return (int)(str - buf);
}

#define DT_DRV_COMPAT tdx_memory_debug
#include <zephyr/device.h>

// #define OLD_WAY 1
#ifdef OLD_WAY
struct Buffer
{
	struct {
		long buffer;
		size_t buffer_size;
	}b;
	size_t index;
};

static struct Buffer debug_buffer = {
	.b = DT_PROP(DT_DRV_INST(0), reg),
	.index = 0
};

static void write_string(const char *s)
{
	static char *buffer = (char *)0x80000000; // (DT_PROP(DT_DRV_INST(0), reg)[0]); // 0x80000000;
	static size_t index = 0;
	const size_t buffer_size = 0x1000000;

//	char *buffer = (char *)(debug_buffer.b.buffer);
//	const size_t buffer_size = debug_buffer.b.buffer_size;

	while(*s)
	{
		buffer[index++] = *s++;
		if(index >= buffer_size)
		{
			index = 0;
		}
	}
}

void my_debug_out(const char *fmt,...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	(void)vsprintf_imp(buffer, fmt, args);
	va_end(args);
	char *s = buffer;
	write_string(s);
}

#else

struct Buffer2
{
	long start;
	size_t size;
};

const static struct Buffer2 debug_buffer2 = {
	.start = 0x80000000, /* DT_REG_ADDR(DT_DRV_INST(0)), */
	.size = 0x1000000 /* DT_REG_SIZE(DT_DRV_INST(0)) */ 
};

static size_t index2 = 0;

void my_debug_out(const char *fmt,...)
{
	char *buffer = (char *)(debug_buffer2.start + index2);
	va_list args;
	va_start(args, fmt);
	int count = vsprintf_imp(buffer, fmt, args);
	va_end(args);
	index2 += count;
	if(index2 >= (debug_buffer2.size - 0x80000))
	{
		index2 = 0;
	}
}

#endif
/*
static int vsprintf(char *buf, const char *fmt, va_list args)
{
	return vsprintf_imp(buf, fmt, args);
}

static int sprintf(char *buf, const char *fmt, ...)
{
	int retVal = 0;
	va_list args;
	va_start(args, fmt);
	retVal = vsprintf_imp(buf, fmt, args);
	va_end(args);
	return retVal;
}
*/

