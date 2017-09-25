#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#include "cpu/sparc/types.h"

/**
 * Read permission flag
 */
#define MP_R (1 << 1)
/**
 * write permission flag
 */
#define MP_W (1 << 2)
/**
 * exec permission flag
 */
#define MP_X (1 << 3)
typedef uint8_t perm_t;

#endif
