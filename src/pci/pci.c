#include "pci.h"

#include "pcie.h"

#include "../inc/stdio.h"
#include "../inc/vector.h"
#include "../mem/heap.h"

#include "../dev/nvme/nvme.h"
#include "../dev/ahci/ahci.h"

vector_t* pci_devices = NULL;

void pci_init() {
    pci_devices = vector_init();

    printf("%C[PCI]", COLOUR_KERNEL_INFO);
    printf(" - Discovering all PCI devices...\n");
    pcie_probe();
}

void pci_init_builtin_drivers() {
    for(uint64_t i = 0; i < vector_size(pci_devices); i++) {
        pci_device_t* device = vector_get(pci_devices, i);

        switch(device->header->class_) {
            case 0x1: { // Mass Storage Controllers
                switch(device->header->subclass) {
                    case 0x8: { // Non-Volatile Memory Controller
                        if(device->header->prog_if == 0x2) { // NVMe
                            nvme_init(device);
                        }
                    } break;

                    case 0x6: { // Serial ATA Controller
                        if(device->header->prog_if == 0x1) { // AHCI
                            ahci_init(device);
                        }
                    } break;
                }
            } break;
        }
    }
}

void pci_register_device(pci_device_hdr_t* hdr) {
    pci_descriptors_t* descriptors = pci_get_descriptors(hdr->vendor, hdr->device_id, hdr->class_, hdr->subclass, hdr->prog_if);

    pci_device_t* device = (pci_device_t*) malloc(sizeof(pci_device_t));
    device->header = hdr;
    device->descriptors = descriptors;

    if((hdr->hdr_type & 0b01111111) == 0x0) {
        device->extended_header = (pci_extended_hdr_t*)(hdr + 1);
        device->bridge_0x1_header = NULL;
    } else if((hdr->hdr_type & 0b01111111) == 0x0) {
        device->extended_header = NULL;
        device->bridge_0x1_header = (pci_bridge_0x1_hdr_t*)(hdr + 1);
    }

    vector_add(pci_devices, (void*) device);

    printf("%C [%d]", COLOUR_SPECIAL_INFO, vector_size(pci_devices));
    printf(" - %s | %s - %s\n", descriptors->vendor, descriptors->class, descriptors->subclass);
}

bool pci_is_status_flag_set(pci_device_t* dev, uint16_t bit) {
    return (dev->header->status & bit) > 0;
}

void pci_set_command_flag(pci_device_t* dev, uint16_t flag) {
    dev->header->command |= flag;
}

void pci_clear_command_flag(pci_device_t* dev, uint16_t flag) {
    dev->header->command &= ~flag;
}
