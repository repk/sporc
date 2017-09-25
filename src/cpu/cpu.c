#include <string.h>

#include "types.h"

#include "cpu/cpu.h"

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
int cpu_boot(struct cpu *c, addr_t addr)
{
	return c->cpu->cops->boot(c, addr);
}

static LIST_HEAD(cpulst);

/**
 * Instantiate a cpu plugin subsystem.
 *
 * @param cfg: New cpu configuration
 * @return: NULL pointer on error, a cpu plugin instance otherwise
 */
struct cpu *cpu_create(struct cpucfg const *cpu)
{
	struct cpu_desc const *cdesc;
	struct cpu *c;

	cdesc = cpu_desc_get(cpu->cpu);
	if(cdesc == NULL)
		return NULL;

	c = cdesc->cops->create(cpu);
	if(c == NULL)
		return NULL;

	c->cpu = cdesc;
	strcpy(c->name, cpu->name);
	list_add_tail(&c->next, &cpulst);
	return c;
}

/**
 * Destroy a cpu plugin instance.
 *
 * @param m: plugin instance.
 */
int cpu_destroy(struct cpu *c)
{
	list_del(&c->next);
	c->cpu->cops->destroy(c);
	return 0;
}

/**
 * Get cpu form its name
 *
 * @param name: CPU name to find
 * @return: CPU if found, NULL pointer otherwise
 */
struct cpu *cpu_get(char const *name)
{
	struct cpu *cpu;

	list_for_each_entry(cpu, &cpulst, next)
		if(strcmp(cpu->name, name) == 0)
			goto out;

	/* No cpu found */
	cpu = NULL;
out:
	return cpu;
}
