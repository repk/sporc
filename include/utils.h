#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

/* TODO move this */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define PERR(...) do {						\
	fprintf(stderr, __VA_ARGS__);				\
	perror("");						\
} while(/*CONSTCOND*/0)

#define ERR(...) do {						\
	fprintf(stderr, __VA_ARGS__);				\
} while(/*CONSTCOND*/0)


#endif
