#ifndef _SRMMU_ACCESS_H_
#define _SRMMU_ACCESS_H_

struct srmmu_access {
	int (*phyacc)(struct dev *mem, phyaddr_t paddr, void *ptr);
	int (*ptecheck)(struct pdc_entry *pdce);
	void *ptr;
	addr_t addr;
	ctx_t ctx;
	pte_t flag;
};

#define SRMMU_ACCESS_INIT(c, va, p, type, sz, f)			\
{									\
	.phyacc = srmmu_phy ## type ## sz,				\
	.ptecheck = pdc_pte_ ## type,					\
	.ptr = p,							\
	.addr = va,							\
	.ctx = c,							\
	.flag = f,							\
}

/* Sparc MMU memory controller physical read */
int srmmu_phyread8(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phyread16(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phyread32(struct dev *mem, phyaddr_t paddr, void *ptr);

/* Sparc MMU memory controller physical fetch */
int srmmu_phyexec8(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phyexec16(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phyexec32(struct dev *mem, phyaddr_t paddr, void *ptr);

/* Sparc MMU memory controller physical write */
int srmmu_phywrite8(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phywrite16(struct dev *mem, phyaddr_t paddr, void *ptr);
int srmmu_phywrite32(struct dev *mem, phyaddr_t paddr, void *ptr);

#endif
