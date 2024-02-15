#include <stdint.h>
#include <limine.h>

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_addr = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

uint64_t
bl_get_hhdm_offset(void)
{
    return hhdm_request.response->offset;
}

uint64_t
bl_get_kernel_virt_addr(void)
{
    return kernel_addr.response->virtual_base;
}

uint64_t
bl_get_kernel_phys_addr(void)
{
    return kernel_addr.response->physical_base;
}

