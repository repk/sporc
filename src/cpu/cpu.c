#include <string.h>

#include "cpu.h"

/**
 * Find a registered cpu plugin from its name.
 *
 * @param name: Name of requested cpu plugin
 * @return: The cpu plugin descriptor if found, NULL pointer otherwise
 */
static struct cpu_desc const *cpu_desc_get(char const *name)
{
	struct cpu_desc const **c, *ret = NULL;
	extern struct cpu_desc const *__cpu_start;
	extern struct cpu_desc const *__cpu_end;

	for(c = &__cpu_start; c < &__cpu_end; ++c) {
		if(strcmp((*c)->name, name) == 0) {
			ret = *c;
			break;
		}
	}

	return ret;
}

/**
 * Fetch next cpu instruction
 *
 * @param c: cpu instance
 * @return: 0 on success, -1 otherwise
 */
int cpu_fetch(struct cpu *c)
{
	return c->cpu->cops->fetch(c);
}

/**
 * Decode next cpu instruction
 *
 * @param c: cpu instance
 * @return: 0 on success, -1 otherwise
 */
int cpu_decode(struct cpu *c)
{
	return c->cpu->cops->decode(c);
}

/**
 * Exec next cpu instruction
 *
 * @param c: cpu instance
 * @return: 0 on success, -1 otherwise
 */
int cpu_exec(struct cpu *c)
{
	return c->cpu->cops->exec(c);
}

/**
 * Boot a specific cpu
 *
 * @param c: cpu instance to boot
 * @return: 0 on success, -1 otherwise
 */
int cpu_boot(struct cpu *c, uintptr_t addr)
{
	return c->cpu->cops->boot(c, addr);
}

/**
 * Instantiate a cpu plugin subsystem.
 *
 * @param name: Name of requested cpu plugin
 * @param args: cpu plugin specific arguments
 * @return: NULL pointer on error, a cpu plugin instance otherwise
 */
struct cpu *cpu_create(char const *name, struct memory *mem, char const *args)
{
	struct cpu_desc const *cdesc;
	struct cpu *c;

	cdesc = cpu_desc_get(name);
	if(cdesc == NULL)
		return NULL;

	c = cdesc->cops->create(args);
	if(c == NULL)
		return NULL;

	c->cpu = cdesc;
	c->mem = mem;
	return c;
}

/**
 * Destroy a cpu plugin instance.
 *
 * @param m: plugin instance.
 */
int cpu_destroy(struct cpu *c)
{
	c->cpu->cops->destroy(c);
	return 0;
}

