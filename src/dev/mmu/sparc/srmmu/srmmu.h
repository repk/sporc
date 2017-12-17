#ifndef _SRMMU_H_
#define _SRMMU_H_

/* Context table pointer */
typedef uint32_t ctp_t;

/* Page table descriptor */
typedef uint32_t ptd_t;
#define PTD_TO_PTP(ptd) ((ptd) >> 2)

/* Page table entry */
typedef uint32_t pte_t;
#define PTE_TO_PTN(pte) (((pte) >> 8) & 0xffffff)
#define PTE_TO_C(pte) (((pte) >> 7) & 0x1)
#define PTE_TO_M(pte) (((pte) >> 6) & 0x1)
#define PTE_M (1 << 6)
#define PTE_TO_R(pte) (((pte) >> 5) & 0x1)
#define PTE_R (1 << 5)
#define PTE_TO_ACC(pte) (((pte) >> 2) & 0x7)
#define PTE_IS_URD(pte) ((PTE_TO_ACC(pte) < 4) || (PTE_TO_ACC(pte) == 5))
#define PTE_IS_UWR(pte) ((PTE_TO_ACC(pte) == 1) || (PTE_TO_ACC(pte) == 3))
#define PTE_IS_UX(pte) ((PTE_TO_ACC(pte) > 1) || (PTE_TO_ACC(pte) < 5))
#define PTE_IS_SRD(pte) (PTE_TO_ACC(pte) != 4)
#define PTE_IS_SWR(pte) (PTE_TO_ACC(pte) & 0x1)
#define PTE_IS_SX(pte) (PTE_TO_ACC(pte) & 0x1)

typedef uint32_t ctx_t;
#define CTX_MAXLOG2 8
#define CTX_MAX ((1 << CTX_MAXLOG2) - 1)
#define CTX_INVAL ((ctx_t)-1)
#define CTX_SUPER ((ctx_t)-2)

enum access_type {
	AT_U_DATA_LD, /* User data load */
	AT_S_DATA_LD, /* Superuser data load */
	AT_U_ISN_LD, /* User instruction fetch */
	AT_S_ISN_LD, /* Superuser instruction fetch */
	AT_U_DATA_ST, /* User data store */
	AT_S_DATA_ST, /* Superuser data store */
	AT_U_ISN_ST, /* User instruction store */
	AT_S_ISN_ST, /* Superuser instruction store */
};

#define VA_REG_NR(a) (((a) >> 24) & 0xff)
#define VA_REG_OFF_MASK (0xffffff)
#define VA_REG_OFF(a) ((a) & VA_REG_OFF_MASK)
#define VA_REG_ADDR(a) ((a) & ~VA_REG_OFF_MASK)
#define VA_SEG_NR(a) (((a) >> 18) & 0x3f)
#define VA_SEG_OFF_MASK (0x3ffff)
#define VA_SEG_OFF(a) ((a) & VA_SEG_OFF_MASK)
#define VA_SEG_ADDR(a) (a & ~VA_SEG_OFF_MASK)
#define VA_PAGE_NR(a) (((a) >> 12) & 0x3f)
#define VA_PAGE_OFF_MASK (0xfff)
#define VA_PAGE_ADDR(a) ((a) & (~VA_PAGE_OFF_MASK))
#define VA_PAGE_OFF(a) ((a) & VA_PAGE_OFF_MASK)

enum  pdc_lvl {
	PL_PAGE,
	PL_SEGMENT,
	PL_REGION,
	PL_CTX,
	PL_NR,
};

struct pdc_entry {
	struct list_head next;
	phyaddr_t pta; /* Page Table entry address */
	ptd_t ptd;
	ctx_t ctx;
	addr_t va;
	enum pdc_lvl lvl;
};
#define PDC_IS_VALID(p) (VA_PAGE_OFF(p->va) == 0)
#define PDC_INVALIDATE(p) ((p)->va |= VA_PAGE_OFF_MASK)
#define PDC_VALIDATE(p) ((p)->va &= ~VA_PAGE_OFF_MASK)
#define PDC_INIT(p, a, c, l) do {					\
	(p)->va = (a) & ~VA_PAGE_OFF_MASK;				\
	PDC_INVALIDATE(p);						\
	(p)->ctx = c;							\
	(p)->pta = 0;							\
	(p)->lvl = l;							\
} while(0);

static inline int pdc_pte_read(struct pdc_entry *pdce)
{
	/* TODO ASSERT(ENTRY_TYPE(pdce) == ET_PTE */
	return (pdce->ctx == CTX_SUPER) ? PTE_IS_SRD(pdce->ptd) :
		PTE_IS_URD(pdce->ptd);
}

static inline int pdc_pte_write(struct pdc_entry *pdce)
{
	/* TODO ASSERT(ENTRY_TYPE(pdce) == ET_PTE */
	return (pdce->ctx == CTX_SUPER) ? PTE_IS_SWR(pdce->ptd) :
		PTE_IS_UWR(pdce->ptd);
}

static inline int pdc_pte_exec(struct pdc_entry *pdce)
{
	/* TODO ASSERT(ENTRY_TYPE(pdce) == ET_PTE */
	return (pdce->ctx == CTX_SUPER) ? PTE_IS_SX(pdce->ptd) :
		PTE_IS_UX(pdce->ptd);
}

static inline phyaddr_t pdc_to_phyaddr(struct pdc_entry *pdce, addr_t va)
{
	phyaddr_t pa;

	/* TODO ASSERT(ENTRY_TYPE(pdce) == ET_PTE */

	switch(pdce->lvl) {
	case PL_PAGE:
		pa = (PTE_TO_PTN(pdce->ptd) << 12) | VA_PAGE_OFF(va);
		break;
	case PL_SEGMENT:
		pa = (PTE_TO_PTN(pdce->ptd) << 12) | VA_SEG_OFF(va);
		break;
	case PL_REGION:
		pa = (PTE_TO_PTN(pdce->ptd) << 12) | VA_REG_OFF(va);
		break;
	case PL_CTX:
	default:
		pa = (PTE_TO_PTN(pdce->ptd) << 12) | va;
		break;
	}

	return pa;
}

#define FSR_EBE(sr) ((((sr)->fsr) >> 10) & 0xff)
#define FSR_L(sr) ((((sr)->fsr) >> 8) & 0x3)
#define FSR_AT(sr) ((((sr)->fsr) >> 5) & 0x7)
enum fault_type {
	FT_NONE, /* No fault */
	FT_ADDR, /* Invalid address error */
	FT_PROT, /* Protection error */
	FT_PRIV, /* Privilege violation error */
	FT_TRANS, /* Translation error */
	FT_ACCESS, /* Access bus error */
	FT_INTERNAL, /* Internal error */
};
#define FSR_FT(sr) ((((sr)->fsr) >> 2) & 0x3)
#define FSR_FAV(sr) ((((sr)->fsr) >> 1) & 0x1)
#define FSR_OW(sr) (((sr)->fsr) & 0x1)

enum entry_type {
	ET_INVAL, /* Not mapped entry */
	ET_PTD, /* Page table descriptor */
	ET_PTE, /* Page table entry */
};
#define ENTRY_TYPE(e) ((e) & 0x3)

#define VFP_ADDR(a) (VA_PAGE_ADDR(a))
enum vfp_type {
	VFP_PAGE = 0,
	VFP_SEG = 1,
	VFP_REG = 2,
	VFP_CTX = 3,
	VFP_ENTIRE = 4,
	VFP_INVAL,
};
#define VFP_TYPE(a) ((enum vfp_type)(((a) >> 8) & 0xf))

static inline enum pdc_lvl vfp_to_pdc_lvl(enum vfp_type type)
{
	enum pdc_lvl ret;

	switch(type) {
	case VFP_PAGE:
	case VFP_ENTIRE:
		ret = PL_PAGE;
		break;
	case VFP_SEG:
		ret = PL_SEGMENT;
		break;
	case VFP_REG:
		ret = PL_REGION;
		break;
	case VFP_CTX:
		ret = PL_CTX;
		break;
	default:
		ret = PL_NR;
		break;
	}

	return ret;
}

#endif
