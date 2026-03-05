#include "pcie.h"

#include "pci.h"

#include "../acpi/mcfg.h"
#include "../mem/vmm.h"
#include "../inc/vector.h"

void pcie_probe() {
    if(!acpi_has_parsed("MCFG")) return;

    mcfg_hdr_t* header = acpi_get_mcfg();
    if(header == NULL) return;

    vector_t* segment_groups = acpi_mcfg_get_segment_groups();

    for(uint64_t i = 0; i < vector_size(segment_groups); i++) {
        mcfg_segment_group_t* segment_group = vector_get(segment_groups, i);

        for(uint8_t bus = segment_group->start_bus; bus < segment_group->end_bus; bus++) {
            pcie_probe_bus(segment_group->base_address, segment_group->start_bus, bus);
        }
    }
}

void pcie_probe_bus(uint64_t base_addr, uint64_t start_bus, uint64_t bus) {
    for(int slot = 0; slot < 32; slot++) {
        pcie_probe_slot(base_addr, start_bus, bus, slot);
    }
}

void pcie_probe_slot(uint64_t base_addr, uint64_t start_bus, uint64_t bus, uint64_t slot) {
    for(int function = 0; function < 8; function++) {
        pcie_probe_function(base_addr, start_bus, bus, slot, function);
    }
}

void pcie_probe_function(uint64_t base_addr, uint64_t start_bus, uint64_t bus, uint64_t slot, uint64_t function) {
    void* address = (void*)(base_addr + (((bus - start_bus) << 20) + (slot << 15) + (function << 12)));

    // Identity mapping this page so it can be accessed without issues
    vmm_map_page_flags(address, address, PT_ENTRY_READ_WRITE | PT_ENTRY_CACHE_DISABLED);

    pci_device_hdr_t* hdr = (pci_device_hdr_t*)(address);

    if(hdr->device_id == 0 || hdr->device_id == 0xFFFF) {
        return;
    }

    /* If this device is a PCI-PCI bridge, then we get the bus it is connected to and enumerate over all of
     * its connected devices. */
    if((hdr->hdr_type & 0b01111111) == 0x1) { // 0x1 is a PCI-PCI bridge
        pci_bridge_0x1_hdr_t* bridge_hdr = (pci_bridge_0x1_hdr_t*)(hdr + 1);
        pcie_probe_bus(base_addr, start_bus, bridge_hdr->secondary_bus_num);
    }

    pci_register_device(hdr);
}
