/*
 * Sparc reference's MMU implementation
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "list.h"
#include "dev/device.h"
#include "dev/cfg/mmu/sparc/srmmu.h"
#include "cpu/cpu.h"
#include "cpu/sparc/sparc.h"

#include "srmmu.h"
#include "access.h"

/**
 * Sparc MMU virtual dev type for configuration
 */
enum srmmu_vdev_type {
	SDT_CACHE,
	SDT_UDATA,
	SDT_UISN,
	SDT_SDATA,
	SDT_SISN,
	SDT_NR,
};

/**
 * Sparc MMU virtual dev description with ASI number and name suffix
 */
struct srmmu_vdev_desc {
	asi_t asi;
	char const *suffix;
};

static struct srmmu_vdev_desc const _vdev_desc[] = {
	[SDT_CACHE] = {
		.asi = SPARC_AS_SRMMU_CACHE,
		.suffix = "-cache",
	},
	[SDT_UDATA] = {
		.asi = SPARC_AS_UDATA,
		.suffix = "-udata",
	},
	[SDT_UISN] = {
		.asi = SPARC_AS_UISN,
		.suffix = "-uisn",
	},
	[SDT_SDATA] = {
		.asi = SPARC_AS_SDATA,
		.suffix = "-sdata",
	},
	[SDT_SISN] = {
		.asi = SPARC_AS_SISN,
		.suffix = "-sisn",
	},
};

struct srmmu;
/* sparc MMU device (data/instruction) configuration */
struct srmmu_dev_cfg {
	enum srmmu_vdev_type type;
	struct srmmu *mmu;
	char const *mem;
};

/* sparc MMU virtual device (data/instruction) */
struct srmmu_dev {
	struct dev dev;
	struct dev *mem; /* Memory controller device */
	struct srmmu *mmu;
	asi_t asi;
};
#define to_srmmu_dev(d) (container_of(d, struct srmmu_dev, dev))

struct srmmu_reg {
	uint32_t ctrl; /* Control Register */
	ctp_t ctp; /* Context table pointer */
	ctx_t ctx; /* Context number */
	addr_t far; /* Fault address register */
	uint32_t fsr; /* Fault status register */
};
#define SRMMU_REG_INIT(r) do						\
{									\
	(r)->ctrl = 0;							\
	(r)->ctp = 0;							\
	(r)->ctx = 0;							\
	(r)->far = 0;							\
	(r)->fsr = 0;							\
} while(0)

#define SRMMU_REG_CTLR_ADDR 0x00000000
#define SRMMU_REG_CTP_ADDR 0x00000100
#define SRMMU_REG_CTX_ADDR 0x00000200
#define SRMMU_REG_FSR_ADDR 0x00000300
#define SRMMU_REG_FAR_ADDR 0x00000400

#define CTP_ADDR(sr) (((sr)->ctp) >> 2)
#define CTP_TO_PTD(sr) (((sr)->ctp) | ET_PTD) /* Fake a PTD from a Ctx ptr */

#define CTRL_IMPL(sr) ((((sr)->ctrl) >> 28) & 0xf)
#define CTRL_VER(sr) ((((sr)->ctrl) >> 24) & 0xf)
#define CTRL_SR(sr) ((((sr)->ctrl) >> 8) & 0xffff)
#define CTRL_PSO(sr) ((((sr)->ctrl) >> 7) & 0x1)
#define CTRL_NF(sr) ((((sr)->ctrl) >> 1) & 0x1)
#define CTRL_EN(sr) (((sr)->ctrl) & 0x1)

#define SRMMU_CACHESZ 64 /* TODO make it tunable */
struct srmmu {
	struct dev dev;
	struct srmmu_reg reg;
	struct srmmu_dev vdev[SDT_NR]; /* SRMMU memory virtual devices */
	struct list_head pdc; /* Page Descriptor cache */
	struct pdc_entry pdesc[SRMMU_CACHESZ]; /* pool of page descriptors */
	struct cpu *cpu;
};
#define to_srmmu(d) (container_of(d, struct srmmu, dev))

/*
 * Try to find address translation in page cache
 *
 * @param dev: Sparc MMU virutal device
 * @param vaddr: Virtual address to translate
 * @param ctx: Process context number
 * @param lvl: PDC entry level
 * @param pdce: If pdce points to non NULL pointer, it is copied with PDC entry
 * content if one is hit, otherwise pdce is set to point to a unlinked matching
 * PDC entry if one is hit. An unlinked PDC entry should be put backed into
 * cache with srmmu_pdc_put().
 *
 * @return: 0 if an entry has been found in cache, negative number otherwise.
 */
static int srmmu_pdc_find(struct srmmu_dev *dev, addr_t vaddr, ctx_t ctx,
		enum pdc_lvl lvl, struct pdc_entry **pdce)
{
	struct srmmu *mmu = dev->mmu;
	struct pdc_entry *e;
	int ret = -1;

	list_for_each_entry(e, &mmu->pdc, next) {
		if(!PDC_IS_VALID(e))
			break;

		if((e->ctx == ctx) && (e->va == vaddr) && (e->lvl == lvl)) {
			if(*pdce == NULL) {
				list_del(&e->next);
				*pdce = e;
			} else {
				**pdce = *e;
			}
			ret = 0;
			break;
		}
	}

	return ret;
}

/**
 * Get, invalidate and unlink the oldest PDC entry.
 *
 * @param dev: Sparc MMU virtual device
 *
 * @return: The oldest PDC entry. This entry has been unlinked from the cache,
 * it must be put back in cache with srmmu_pdc_put() after use.
 */
static inline struct pdc_entry *srmmu_pdc_get(struct srmmu_dev *dev)
{
	struct pdc_entry *res = list_last_entry(&dev->mmu->pdc,
			struct pdc_entry, next);
	list_del(&res->next);
	PDC_INVALIDATE(res);

	return res;
}

/**
 * Add virtual address translation in cache. Depending upon its validity, the
 * entry is either add at the head or the tail of the cache.
 *
 * @param dev: Sparc MMU device
 * @param pdce: Page Descriptor entry to store
 */
static inline void srmmu_pdc_put(struct srmmu_dev *dev, struct pdc_entry *pdce)
{
	struct srmmu *mmu = dev->mmu;

	if(PDC_IS_VALID(pdce))
		list_add(&pdce->next, &mmu->pdc);
	else
		list_add_tail(&pdce->next, &mmu->pdc);
}

/**
 * Flush a PTE from cache depending on the flush level
 *
 * @param dev: Sparc MMU virtual device
 * @param pte: pte to flush
 * @param type: flushed entry's type (an entire cache flush is done with
 * 		type equals to VFP_ENTIRE)
 */
static inline void _srmmu_pdc_flush_pte(struct srmmu_dev *dev,
		struct pdc_entry *pte, enum vfp_type type)
{
	struct srmmu *mmu = dev->mmu;

	switch(type) {
	case VFP_PAGE:
	case VFP_SEG:
	case VFP_REG:
		if((PTE_TO_ACC(pte->ptd) > 5) || (pte->ctx == mmu->reg.ctx)) {
			list_del(&pte->next);
			PDC_INVALIDATE(pte);
			list_add_tail(&pte->next, &mmu->pdc);
		}
		break;
	case VFP_CTX:
		if((PTE_TO_ACC(pte->ptd) < 6) || (pte->ctx == mmu->reg.ctx)) {
			list_del(&pte->next);
			PDC_INVALIDATE(pte);
			list_add_tail(&pte->next, &mmu->pdc);
		}
		break;
	case VFP_ENTIRE:
		list_del(&pte->next);
		PDC_INVALIDATE(pte);
		list_add_tail(&pte->next, &mmu->pdc);
		break;
	default:
		break;
	}
}

/**
 * Flush a PTD from cache depending on the flush level
 *
 * @param dev: Sparc MMU virtual device
 * @param ptd: ptd to flush
 * @param type: flushed entry's type (an entire cache flush is done with
 * 		type equals to VFP_ENTIRE)
 */
static inline void _srmmu_pdc_flush_ptd(struct srmmu_dev *dev,
		struct pdc_entry *ptd, enum vfp_type type)
{
	struct srmmu *mmu = dev->mmu;

	switch(type) {
	case VFP_PAGE:
	case VFP_SEG:
	case VFP_REG:
	case VFP_CTX:
		if(ptd->ctx == mmu->reg.ctx) {
			list_del(&ptd->next);
			PDC_INVALIDATE(ptd);
			list_add_tail(&ptd->next, &mmu->pdc);
		}
		break;
	case VFP_ENTIRE:
		list_del(&ptd->next);
		PDC_INVALIDATE(ptd);
		list_add_tail(&ptd->next, &mmu->pdc);
		break;
	default:
		break;
	}
}

/**
 * Flush some PDC cache entries
 *
 * @param dev: Sparc MMU virtual device
 * @param addr: Entry address to flush when applicable
 * @param type: flushed entries' type (an entire cache flush is done with
 * 		type equals to VFP_ENTIRE)
 */
static inline void srmmu_pdc_flushcache(struct srmmu_dev *dev,
		addr_t addr, enum vfp_type type)
{
	struct srmmu *mmu = dev->mmu;
	struct pdc_entry *pdce;
	enum pdc_lvl lvl = vfp_to_pdc_lvl(type);
	addr_t mask;

	if(type == VFP_INVAL)
		return;

	/* Find Addr mask comparison */
	switch(type) {
	case VFP_PAGE:
		mask = ~VA_PAGE_OFF_MASK;
		break;
	case VFP_SEG:
		mask = ~VA_SEG_OFF_MASK;
		break;
	case VFP_REG:
		mask = ~VA_REG_OFF_MASK;
		break;
	case VFP_CTX:
	case VFP_ENTIRE:
		mask = 0;
		break;
	default:
		return;
	}

	list_for_each_entry(pdce, &mmu->pdc, next) {
		if(!PDC_IS_VALID(pdce))
			break;

		if((pdce->va & mask) != (addr & mask))
			continue;

		if((type != VFP_ENTIRE) && (pdce->lvl > lvl))
			continue;

		switch(ENTRY_TYPE(pdce->ptd)) {
		case ET_PTE:
			_srmmu_pdc_flush_pte(dev, pdce, type);
			break;
		case ET_PTD:
			_srmmu_pdc_flush_ptd(dev, pdce, type);
			break;
		default:
			/* Just in case, should not happen */
			list_del(&pdce->next);
			PDC_INVALIDATE(pdce);
			list_add_tail(&pdce->next, &mmu->pdc);
			break;
		}
	}
}

/**
 * Translate a Virtual Address into a Physical one
 *
 * @param mmu: Sparc MMU virtual device
 * @param ctx: Current MMU context
 * @param vaddr: Virtual address being translated
 * @param lvl: Cache probing level (a full cache access has a PL_PAGE lvl)
 * @param pdce: Virtual address PDC entry found or created. If set to NULL, pdc
 * is updated, else the entry pointed by pdce is either filled out with
 * address translation result or pdce is set to point to a cache entry.
 *
 * @return: 0 on success (the pdce is filled with proper translation), negative
 * number otherwise.
 */
static int srmmu_translate(struct srmmu_dev *dev, ctx_t ctx, addr_t vaddr,
		enum pdc_lvl lvl, struct pdc_entry **pdce)
{
	struct pdc_entry *e = *pdce;
	struct dev *mem = dev->mem;
	size_t i;
	phyaddr_t pta;
	ptd_t ptd;
	ptd_t addr[] = {
		VA_PAGE_ADDR(vaddr),
		VA_SEG_ADDR(vaddr),
		VA_REG_ADDR(vaddr),
		0,
	};
	ptd_t off[] = {
		VA_PAGE_NR(vaddr) * sizeof(ptd_t),
		VA_SEG_NR(vaddr) * sizeof(ptd_t),
		VA_REG_NR(vaddr) * sizeof(ptd_t),
	};
	int ret;

	if(mem->drv->phyops->read32 == NULL)
		return -ENOSYS;

	/* Find closest macthing pdc entry */
	for(i = lvl; i < PL_NR; ++i) {
		ret = srmmu_pdc_find(dev, addr[i], ctx, i, &e);
		if(ret == 0)
			break;
	}

	if(ret != 0) {
		/* XXX ASSERT(i == PL_NR); */
		ret = mem->drv->phyops->read32(mem,
				(dev->mmu->reg.ctp << 4) + dev->mmu->reg.ctx,
				&ptd);
		if(ret != 0)
			goto out;

		if(*pdce == NULL)
			e = srmmu_pdc_get(dev);

		--i;
		PDC_INIT(e, 0, ctx, PL_CTX);
		e->ptd = be32toh(ptd);
	}

	/* XXX ASSERT(i < PL_NR); */
	pta = e->pta;

	/* Walk through, translate and cache all PTD levels */
	for(; (i > lvl) && (ENTRY_TYPE(e->ptd) == ET_PTD); --i) {
		if(*pdce == NULL) {
			PDC_VALIDATE(e);
			srmmu_pdc_put(dev, e);
		}

		pta = (PTD_TO_PTP(e->ptd) << 6) + off[i - 1];
		ret = mem->drv->phyops->read32(mem, pta, &ptd);
		if(ret != 0)
			goto out;

		if(*pdce == NULL)
			e = srmmu_pdc_get(dev);

		PDC_INIT(e, addr[i - 1], ctx, i - 1);
		e->ptd = be32toh(ptd);
	}

	ret = -EINVAL;
	if(ENTRY_TYPE(e->ptd) != ET_PTE) /* TODO Raise interrupt ? */
		goto out;

	/* Translation succeed */
	e->pta = pta;
	*pdce = e;
	ret = 0;

out:
	return ret;
}

/**
 * Update a Page descriptor PTE
 *
 * @param dev: Current Sparc MMU virtual device
 * @param pdce: Page descriptor to update
 * @param flag: New PTE flags
 *
 * @return: 0 on success, negative number otherwise.
 */
static inline int srmmu_pdc_update(struct srmmu_dev *dev,
		struct pdc_entry *pdce, pte_t flags)
{
	struct dev *mem = dev->mem;
	pte_t new = pdce->ptd | flags;
	int ret = -ENOSYS;

	if(!mem->drv->phyops->write32)
		goto out;

	ret = -EINVAL;
	if(ENTRY_TYPE(pdce->ptd) != ET_PTE)
		goto out;

	if(new != pdce->ptd) {
		pdce->ptd = new;
		ret = mem->drv->phyops->write32(mem, pdce->pta,
				htobe32(pdce->ptd));
		if(ret != 0)
			goto out;
	}

	PDC_VALIDATE(pdce);
	ret = 0;

out:
	return ret;
}

/**
 * Access MMU memory
 *
 * @param dev: MMU virtual device dev pointer
 * @param acc: MMU transaction description
 *
 * @return: 0 on success, negative number otherwise
 */
static int srmmu_access(struct dev *dev, struct srmmu_access const *acc)
{
	struct srmmu_dev *mdev = to_srmmu_dev(dev);
	struct dev *mem = mdev->mem;
	struct pdc_entry *pdce = NULL;
	phyaddr_t pa;
	int ret = -ENOSYS;

	/* MMU disabled, passthrough */
	if(!CTRL_EN(&mdev->mmu->reg))
		return acc->phyacc(mem, (phyaddr_t)acc->addr, acc->ptr);

	ret = srmmu_translate(mdev, acc->ctx, acc->addr, PL_PAGE, &pdce);
	if(ret != 0)
		goto out;

	/* Check access permissions */
	ret = -EPERM;
	if(!acc->ptecheck(pdce))
		goto out;

	/* Fetch requested value */
	pa = pdc_to_phyaddr(pdce, acc->addr);
	ret = acc->phyacc(mem, pa, acc->ptr);
	if(ret != 0)
		goto out;

	ret = srmmu_pdc_update(mdev, pdce, acc->flag);

out:
	if(pdce != NULL)
		srmmu_pdc_put(mdev, pdce);
	return ret;
}

/**
 * Fetch a 8 bit value from user data memory
 */
static int srmmu_uread8(struct dev *dev, addr_t vaddr, uint8_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, read, 8, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 16 bit value from user data memory
 */
static int srmmu_uread16(struct dev *dev, addr_t vaddr, uint16_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, read, 16, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 32 bit value from user data memory
 */
static int srmmu_uread32(struct dev *dev, addr_t vaddr, uint32_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, read, 32, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 8 bit value to user data memory
 */
static int srmmu_uwrite8(struct dev *dev, addr_t vaddr, uint8_t val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			&val, write, 8, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 16 bit value to user data memory
 */
static int srmmu_uwrite16(struct dev *dev, addr_t vaddr, uint16_t val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			&val, write, 16, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 32 bit value to user data memory
 */
static int srmmu_uwrite32(struct dev *dev, addr_t vaddr, uint32_t val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			&val, write, 32, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 8 bit value from user data memory
 */
static int srmmu_ufetch8(struct dev *dev, addr_t vaddr, uint8_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, exec, 8, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 16 bit value from user data memory
 */
static int srmmu_ufetch16(struct dev *dev, addr_t vaddr, uint16_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, exec, 16, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 32 bit value from user data memory
 */
static int srmmu_ufetch32(struct dev *dev, addr_t vaddr, uint32_t *val)
{
	struct srmmu *mmu = to_srmmu_dev(dev)->mmu;
	struct srmmu_access acc = SRMMU_ACCESS_INIT(mmu->reg.ctx, vaddr,
			val, exec, 32, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 8 bit value from supervisor data memory
 */
static int srmmu_sread8(struct dev *dev, addr_t vaddr, uint8_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, read, 8, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 16 bit value from supervisor data memory
 */
static int srmmu_sread16(struct dev *dev, addr_t vaddr, uint16_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, read, 16, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 32 bit value from supervisor data memory
 */
static int srmmu_sread32(struct dev *dev, addr_t vaddr, uint32_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, read, 32, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 8 bit value to supervisor data memory
 */
static int srmmu_swrite8(struct dev *dev, addr_t vaddr, uint8_t val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			&val, write, 8, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 16 bit value to supervisor data memory
 */
static int srmmu_swrite16(struct dev *dev, addr_t vaddr, uint16_t val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			&val, write, 16, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Write a 32 bit value to supervisor data memory
 */
static int srmmu_swrite32(struct dev *dev, addr_t vaddr, uint32_t val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			&val, write, 32, PTE_R | PTE_M);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 8 bit value from supervisor data memory
 */
static int srmmu_sfetch8(struct dev *dev, addr_t vaddr, uint8_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, exec, 8, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 16 bit value from supervisor data memory
 */
static int srmmu_sfetch16(struct dev *dev, addr_t vaddr, uint16_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, exec, 16, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Fetch a 32 bit value from supervisor data memory
 */
static int srmmu_sfetch32(struct dev *dev, addr_t vaddr, uint32_t *val)
{
	struct srmmu_access acc = SRMMU_ACCESS_INIT(CTX_SUPER, vaddr,
			val, exec, 32, PTE_R);

	return srmmu_access(dev, &acc);
}

/**
 * Create a new sparc reference mmu virtual device
 *
 * @param dev: Device to initialize
 * @param cfg: New device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int srmmu_vdev_create(struct dev **dev, struct devcfg const *cfg)
{
	struct srmmu_dev_cfg const *scfg =
		(struct srmmu_dev_cfg const*)cfg->cfg;
	struct srmmu_dev *mdev = &scfg->mmu->vdev[scfg->type];
	int ret = -EINVAL;

	if(scfg->type > SDT_NR)
		goto out;

	ret = -ENODEV;
	mdev->mem = dev_get(scfg->mem);
	if(mdev->mem == NULL)
		goto out;

	mdev->mmu = scfg->mmu;
	mdev->asi = _vdev_desc[scfg->type].asi;

	ret = scpu_register_mem(mdev->mmu->cpu, mdev->asi, &mdev->dev);
	if(ret != 0)
		goto out;

	*dev = &mdev->dev;
out:
	return ret;
}

/**
 * Destroy a sparc reference mmu virtual device
 *
 * @param dev: Device to remove
 *
 * @return: 0 on success, negative error otherwise
 */
static void srmmu_vdev_destroy(struct dev *dev)
{
	struct srmmu_dev *mdev = to_srmmu_dev(dev);

	scpu_remove_mem(mdev->mmu->cpu, mdev->asi);
}

/**
 * Sparc MMU PDC probe function
 */
static int srmmu_pdc_probe(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct pdc_entry _entry;
	struct srmmu_dev *mdev = to_srmmu_dev(dev);
	struct pdc_entry *pdce = &_entry;
	addr_t va = VFP_ADDR(addr);
	enum vfp_type type = VFP_TYPE(addr);
	enum pdc_lvl lvl = vfp_to_pdc_lvl(type);
	int ret;

	*val = 0; /* In case of error result is set to 0 */

	if(!CTRL_EN(&mdev->mmu->reg))
		goto out;

	if(type == VFP_INVAL)
		goto out;

	/* Update cache */
	if(type == VFP_ENTIRE)
		pdce = NULL;

	ret = srmmu_translate(mdev, mdev->mmu->reg.ctx, va, lvl, &pdce);
	if(ret == 0)
		goto out;

	/* This is not a proper level cache entry */
	if((type != VFP_ENTIRE) && (pdce->lvl != lvl))
		goto out;

	if(type == VFP_ENTIRE) {
		/* Only update when type is VFP_ENTIRE */
		ret = srmmu_pdc_update(mdev, pdce, PTE_R);
		if(ret == 0)
			goto out;
	}

	*val = pdce->ptd;
out:
	if(pdce != NULL)
		srmmu_pdc_put(mdev, pdce);
	return 0;
}

/**
 * Sparc MMU PDC flush function
 */
static int srmmu_pdc_flush(struct dev *dev, addr_t addr, uint32_t val)
{
	struct srmmu_dev *mdev = to_srmmu_dev(dev);
	enum vfp_type type = VFP_TYPE(addr);
	addr_t vfpa = VFP_ADDR(addr);
	(void)val;

	if(!CTRL_EN(&mdev->mmu->reg))
		goto out;

	if(type == VFP_INVAL)
		goto out;

	srmmu_pdc_flushcache(mdev, vfpa, type);

out:
	return 0;
}

/**
 * Sparc MMU register reading
 */
static int srmmu_rdreg(struct dev *dev, addr_t addr, uint32_t *val)
{
	struct srmmu *mmu = to_srmmu(dev);

	/* TODO Assert if addr & 0xff != 0 to detect bugged software ? */
	switch(addr) {
	case SRMMU_REG_CTLR_ADDR:
		*val = htobe32(mmu->reg.ctrl);
		break;
	case SRMMU_REG_CTP_ADDR:
		*val = htobe32(mmu->reg.ctp);
		break;
	case SRMMU_REG_CTX_ADDR:
		*val = htobe32(mmu->reg.ctx);
		break;
	case SRMMU_REG_FSR_ADDR:
		*val = htobe32(mmu->reg.fsr);
		break;
	case SRMMU_REG_FAR_ADDR:
		*val = htobe32(mmu->reg.far);
		break;
	default:
		/*
		 * TODO Sparc Manual is not clear about undefined sparc register
		 * accesses. For now they are just ignored.
		 */
		break;
	}

	return 0;
}

/**
 * Sparc MMU register writing
 */
static int srmmu_wrreg(struct dev *dev, addr_t addr, uint32_t val)
{
	struct srmmu *mmu = to_srmmu(dev);

	/* TODO Assert if addr & 0xff != 0 to detect bugged software ? */
	switch(addr) {
	case SRMMU_REG_CTLR_ADDR:
		mmu->reg.ctrl = be32toh(val) & 0x83; /* PSO NF E are writable */
		break;
	case SRMMU_REG_CTP_ADDR:
		if((val & ((1 << (CTX_MAXLOG2 + 2)) - 1)) == 0)
			mmu->reg.ctp = be32toh(val) & ~(0x3);
		break;
	case SRMMU_REG_CTX_ADDR:
		if(val <= CTX_MAX)
			mmu->reg.ctx = be32toh(val);
		break;
	case SRMMU_REG_FSR_ADDR:
	case SRMMU_REG_FAR_ADDR:
		break;
	default:
		/*
		 * TODO Sparc Manual is not clear about undefined sparc register
		 * accesses. For now they are just ignored.
		 */
		break;
	}

	return 0;
}

/**
 * Create a new sparc reference mmu device
 *
 * @param dev: New device to create
 * @param cfg: New device configuration
 *
 * @return: 0 on success, negative error otherwise
 */
static int srmmu_create(struct dev **dev, struct devcfg const *cfg)
{
	struct srmmu *mmu;
	struct sparc_srmmu_cfg const *scfg =
		(struct sparc_srmmu_cfg const*)cfg->cfg;
	struct srmmu_dev_cfg *vcfg;
	/* Sparc mmu virtual device config */
	struct devcfg vdev[] = {
		{
			.drvname = "srmmu-cache",
			.cfg = DEVCFG(srmmu_dev_cfg) {
				.mem = scfg->dmem,
				.type = SDT_CACHE,
			},
		},
		{
			.drvname = "srmmu-udata",
			.cfg = DEVCFG(srmmu_dev_cfg) {
				.mem = scfg->dmem,
				.type = SDT_UDATA,
			},
		},
		{
			.drvname = "srmmu-sdata",
			.cfg = DEVCFG(srmmu_dev_cfg) {
				.mem = scfg->dmem,
				.type = SDT_SDATA,
			},
		},
		{
			.drvname = "srmmu-uisn",
			.cfg = DEVCFG(srmmu_dev_cfg) {
				.mem = scfg->imem,
				.type = SDT_UISN,
			},
		},
		{
			.drvname = "srmmu-sisn",
			.cfg = DEVCFG(srmmu_dev_cfg) {
				.mem = scfg->imem,
				.type = SDT_SISN,
			},
		},
	};
	struct dev *d;
	char const *suffix;
	size_t i;
	int ret = -ENOMEM;

	*dev = NULL;

	mmu = malloc(sizeof(*mmu));
	if(mmu == NULL)
		goto err;

	SRMMU_REG_INIT(&mmu->reg);

	ret = -ENODEV;
	mmu->cpu = cpu_get(scfg->cpu);
	if(mmu->cpu == NULL)
		goto err;

	/* Initialize SRMMU page cache */
	INIT_LIST_HEAD(&mmu->pdc);
	for(i = 0; i < SRMMU_CACHESZ; ++i) {
		PDC_INVALIDATE(&mmu->pdesc[i]);
		list_add(&mmu->pdesc[i].next, &mmu->pdc);
	}

	/* Configure data and instruction memory virtual devices */
	ret = -EINVAL;
	for(i = 0; i < ARRAY_SIZE(vdev); ++i) {
		vcfg = (struct srmmu_dev_cfg *)vdev[i].cfg;
		suffix = _vdev_desc[vcfg->type].suffix;
		if(strlen(cfg->name) + strlen(suffix) > sizeof(vdev[i].name))
			goto derr;
		strcpy(vdev[i].name, cfg->name);
		strcat(vdev[i].name, suffix);
		vcfg->mmu = mmu;
		/* Create virtual devices */
		d = dev_create(&vdev[i]);
		if(d != &mmu->vdev[vcfg->type].dev)
			goto derr;
	}

	ret = scpu_register_mem(mmu->cpu, SPARC_AS_SRMMU_REG, &mmu->dev);
	if(ret != 0)
		goto derr;

	*dev = &mmu->dev;

	return 0;

derr:
	for(; i > 1; --i) {
		vcfg = (struct srmmu_dev_cfg *)vdev[i - 1].cfg;
		dev_destroy(&mmu->vdev[vcfg->type].dev);
	}
err:
	if(mmu)
		free(mmu);
	return ret;
}

/**
 * Destroy a sparc reference mmu device
 *
 * @param dev: Device to destroy
 */
static void srmmu_destroy(struct dev *dev)
{
	struct srmmu *mmu = to_srmmu(dev);
	size_t i;

	for(i = 0; i < ARRAY_SIZE(mmu->vdev); ++i)
		dev_destroy(&mmu->vdev[i].dev);

	free(mmu);
}

/* MMU PDC virtual driver */
static struct devops const srmmufpops = {
	.create = srmmu_vdev_create,
	.destroy = srmmu_vdev_destroy,
	.read32 = srmmu_pdc_probe,
	.write32 = srmmu_pdc_flush,
};

static struct drv const srmmu_cache = {
	.name = "srmmu-cache",
	.ops = &srmmufpops,
};
DRIVER_REGISTER(srmmu_cache);

/* User data mmu virtual driver */
static struct devops const srmmuudops = {
	.create = srmmu_vdev_create,
	.destroy = srmmu_vdev_destroy,
	.read8 = srmmu_uread8,
	.read16 = srmmu_uread16,
	.read32 = srmmu_uread32,
	.write8 = srmmu_uwrite8,
	.write16 = srmmu_uwrite16,
	.write32 = srmmu_uwrite32,
};

static struct drv const srmmu_udata = {
	.name = "srmmu-udata",
	.ops = &srmmuudops,
};
DRIVER_REGISTER(srmmu_udata);

/* Supervisor data mmu virtual driver */
static struct devops const srmmusdops = {
	.create = srmmu_vdev_create,
	.destroy = srmmu_vdev_destroy,
	.read8 = srmmu_sread8,
	.read16 = srmmu_sread16,
	.read32 = srmmu_sread32,
	.write8 = srmmu_swrite8,
	.write16 = srmmu_swrite16,
	.write32 = srmmu_swrite32,
};

static struct drv const srmmu_sdata = {
	.name = "srmmu-sdata",
	.ops = &srmmusdops,
};
DRIVER_REGISTER(srmmu_sdata);

/* User instruction mmu virtual driver */
static struct devops const srmmuuiops = {
	.create = srmmu_vdev_create,
	.destroy = srmmu_vdev_destroy,
	.read8 = srmmu_ufetch8,
	.read16 = srmmu_ufetch16,
	.read32 = srmmu_ufetch32,
};

static struct drv const srmmu_uisn = {
	.name = "srmmu-uisn",
	.ops = &srmmuuiops,
};
DRIVER_REGISTER(srmmu_uisn);

/* Supervisor instruction mmu virtual driver */
static struct devops const srmmusiops = {
	.create = srmmu_vdev_create,
	.destroy = srmmu_vdev_destroy,
	.read8 = srmmu_sfetch8,
	.read16 = srmmu_sfetch16,
	.read32 = srmmu_sfetch32,
};

static struct drv const srmmu_sisn = {
	.name = "srmmu-sisn",
	.ops = &srmmusiops,
};
DRIVER_REGISTER(srmmu_sisn);

/* Global sparc reference mmu driver */
static struct devops const srmmuops = {
	.create = srmmu_create,
	.destroy = srmmu_destroy,
	.read32 = srmmu_rdreg,
	.write32 = srmmu_wrreg,
};

static struct drv const srmmu = {
	.name = "sparc-srmmu",
	.ops = &srmmuops,
};
DRIVER_REGISTER(srmmu);
