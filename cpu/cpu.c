#include <string.h>

#include "cpu.h"

static struct cpu_desc const *cpu_desc_get(char const *name)
{
	struct cpu_desc const **c, *ret;
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

int cpu_fetch(struct cpu *c)
{
	return c->cpu->cops->fetch(c);
}

int cpu_decode(struct cpu *c)
{
	return c->cpu->cops->decode(c);
}

int cpu_exec(struct cpu *c)
{
	return c->cpu->cops->exec(c);
}

int cpu_boot(struct cpu *c, uintptr_t addr)
{
	return c->cpu->cops->boot(c, addr);
}

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

int cpu_destroy(struct cpu *c)
{
	c->cpu->cops->destroy(c);
	return 0;
}

