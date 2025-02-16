

















#if !defined(GRUB_PCIUTILS_H)
#define GRUB_PCIUTILS_H 1

#include <pciaccess.h>

typedef struct pci_device *grub_pci_device_t;

static inline int
grub_pci_get_bus (grub_pci_device_t dev)
{
return dev->bus;
}

static inline int
grub_pci_get_device (grub_pci_device_t dev)
{
return dev->dev;
}

static inline int
grub_pci_get_function (grub_pci_device_t dev)
{
return dev->func;
}

struct grub_pci_address
{
grub_pci_device_t dev;
int pos;
};

typedef struct grub_pci_address grub_pci_address_t;

static inline grub_uint32_t
grub_pci_read (grub_pci_address_t addr)
{
grub_uint32_t ret;
pci_device_cfg_read_u32 (addr.dev, &ret, addr.pos);
return ret;
}

static inline grub_uint16_t
grub_pci_read_word (grub_pci_address_t addr)
{
grub_uint16_t ret;
pci_device_cfg_read_u16 (addr.dev, &ret, addr.pos);
return ret;
}

static inline grub_uint8_t
grub_pci_read_byte (grub_pci_address_t addr)
{
grub_uint8_t ret;
pci_device_cfg_read_u8 (addr.dev, &ret, addr.pos);
return ret;
}

static inline void
grub_pci_write (grub_pci_address_t addr, grub_uint32_t data)
{
pci_device_cfg_write_u32 (addr.dev, data, addr.pos);
}

static inline void
grub_pci_write_word (grub_pci_address_t addr, grub_uint16_t data)
{
pci_device_cfg_write_u16 (addr.dev, data, addr.pos);
}

static inline void
grub_pci_write_byte (grub_pci_address_t addr, grub_uint8_t data)
{
pci_device_cfg_write_u8 (addr.dev, data, addr.pos);
}

void *
grub_pci_device_map_range (grub_pci_device_t dev, grub_addr_t base,
grub_size_t size);

void
grub_pci_device_unmap_range (grub_pci_device_t dev, void *mem,
grub_size_t size);


#endif 
