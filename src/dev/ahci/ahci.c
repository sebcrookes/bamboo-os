#include "ahci.h"

#include "../../inc/stdio.h"
#include "../../inc/faults.h"
#include "../../mem/vmm.h"

void ahci_init(pci_device_t* device) {

    device->header->command = 0;
    pci_set_command_flag(device, PCI_CMD_MEM_SPACE);
    pci_set_command_flag(device, PCI_CMD_BUS_MASTER);
    pci_clear_command_flag(device, PCI_CMD_INT_DISABLE);

    uint64_t abar = (uint64_t) device->extended_header->bar5;
    vmm_map_page_flags((void*) abar, (void*) abar, PT_ENTRY_READ_WRITE | PT_ENTRY_CACHE_DISABLED);

    hba_mem_t* hba_mem = (hba_mem_t*) abar;

    // BIOS/OS handoff is currently unsupported
    faults_assert(!(hba_mem->cap2 & AHCI_CAP2_BOH), "AHCI - BOH Unsupported");

    // Performing a full internal reset of the HBA
    hba_mem->ghc |= AHCI_GHC_HR;
    while(hba_mem->ghc & AHCI_GHC_HR) {} // THe flag is cleared when reset is complete

    // printf("%X - %X\n", device->extended_header->interrupt_line, device->extended_header->interrupt_pin);

    // TODO: Check capabilities list

    // Setting the AHCI Enable flag (to only use AHCI mechanisms)
    hba_mem->ghc |= AHCI_GHC_AE;
    hba_mem->ghc |= AHCI_GHC_IE;

    // printf("%X\n", device->extended_header->interrupt_line);

    if(pci_is_status_flag_set(device, PCI_STATUS_CAP_LIST)) {

    }

    printf("%C[AHCI]%C - AHCI driver in development\n", COLOUR_DRIVER_INFO, COLOUR_PRINT);
}
