#include <stdlib.h>

#include "types.h"
#include "dev/device.h"

#include "srmmu.h"
#include "access.h"

/**
 * Sparc MMU read physical 8 bits from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Read result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyread8(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->read8)
		return -ENOSYS;
	return mem->drv->phyops->read8(mem, pa, (uint8_t *)ptr);
}

/**
 * Sparc MMU read physical 16 bits from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Read result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyread16(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->read16)
		return -ENOSYS;
	return mem->drv->phyops->read16(mem, pa, (uint16_t *)ptr);
}

/**
 * Sparc MMU read physical 32 bits from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Read result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyread32(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->read32)
		return -ENOSYS;
	return mem->drv->phyops->read32(mem, pa, (uint32_t *)ptr);
}

/**
 * Sparc MMU fetch physical 8bit instruction from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Fetch result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyexec8(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->fetch_isn8)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn8(mem, pa, (uint8_t *)ptr);
}

/**
 * Sparc MMU fetch physical 16bit instruction from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Fetch result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyexec16(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->fetch_isn16)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn16(mem, pa, (uint16_t *)ptr);
}

/**
 * Sparc MMU fetch physical 32bit instruction from memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Fetch result
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phyexec32(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->fetch_isn32)
		return -ENOSYS;
	return mem->drv->phyops->fetch_isn32(mem, pa, (uint32_t *)ptr);
}

/**
 * Sparc MMU write physical 8bit value to memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Write data
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phywrite8(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->write8)
		return -ENOSYS;
	return mem->drv->phyops->write8(mem, pa, *(uint8_t *)ptr);
}

/**
 * Sparc MMU write physical 16bit value to memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Write data
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phywrite16(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->write16)
		return -ENOSYS;
	return mem->drv->phyops->write16(mem, pa, *(uint16_t *)ptr);
}

/**
 * Sparc MMU write physical 32bit value to memory
 *
 * @mem: Memory physical device
 * @pa: Physical address
 * @ptr: Write data
 *
 * @return: 0 on success negative number otherwise
 */
int srmmu_phywrite32(struct dev *mem, phyaddr_t pa, void *ptr)
{
	if(!mem->drv->phyops->write32)
		return -ENOSYS;
	return mem->drv->phyops->write32(mem, pa, *(uint32_t *)ptr);
}
