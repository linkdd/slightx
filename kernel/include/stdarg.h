#ifndef __STDARG_H
#define __STDARG_H

typedef char *va_list;

#define va_start(ap,p)    (ap = (char *) (&(p) + 1))
#define va_arg(ap,type)   ((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)

#endif /* __STDARG_H */
