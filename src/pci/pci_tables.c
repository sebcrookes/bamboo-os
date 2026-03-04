#include "pci_tables.h"

#include "pci.h"

#include "../mem/heap.h"

/* An array of key-value pairs which match vendor IDs to their human readable descriptor. */
pci_vendor_kv_t pci_vendor_descriptors[] = {
    {0x8086, "Intel Corporation"},

    {0xFFFF, "Illegal Vendor ID"} // 0xFFFF also signifies the end of this array, do NOT remove
};

/* An array of all of the PCI class descriptors, except a few exceptions. Accessed by index. */
const char* pci_class_descriptors[] = {
    "Unclassified",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Device",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device",
    "Docking Station",
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Satellite Communication Controller",
    "Encryption Controller",
    "Signal Processing Controller",
    "Processing Accelerator",
    "Non-Essential Instrumentation"
    
    // 0x14 -> 0x3E and 0x41 -> 0xFE are reserved
    // 0x40 is "Co-Processor"
    // 0xFF is Unassigned Class
};

/* The following are arrays of key-key-value triples where each pair of two keys (subclass + prog_if) matches to a descriptor
 * for that device. If the prog_if value is -1, then that is a "catch all" string - the search algorithm progresses through the
 * array, and if it finds that value, then that string will be allocated, as long as the subclass matches. */

pci_subclass_kv_t pci_mass_storage_descriptors[] = {
    {0x0, -1, "SCSI Bus Controller"},

    {0x1, 0x0, "IDE Controller, ISA compatibility mode only"},
    {0x1, 0x5, "IDE Controller, PCI native mode only"},
    {0x1, 0xA, "IDE Controller, ISA compatibility mode, can switch to PCI native mode"},
    {0x1, 0xF, "IDE Controller, PCI native mode, can switch to ISA compatibility mode"},
    {0x1, 0x80, "IDE Controller, ISA compatibility mode only + bus mastering"},
    {0x1, 0x85, "IDE Controller, PCI native mode only + bus mastering"},
    {0x1, 0x8A, "IDE Controller, ISA compatibility mode, can switch to PCI native mode, + bus mastering"},
    {0x1, 0x8F, "IDE Controller, PCI native mode, can switch to ISA compatibility mode, + bus mastering"},
    {0x1, -1, "IDE Controller"},

    {0x2, -1, "Floppy Disk Controller"},
    {0x3, -1, "IPI Bus Controller"},
    {0x4, -1, "RAID Controller"},

    {0x5, 0x20, "ATA Controller, Single DMA"},
    {0x5, 0x30, "ATA Controller, Chained DMA"},
    {0x5, -1, "ATA Controller"},

    {0x6, 0x0, "Serial ATA Controller, Vendor Specific Interface"},
    {0x6, 0x1, "Serial ATA Controller, AHCI 1.0"},
    {0x6, 0x2, "Serial ATA Controller, Serial Storage Bus"},
    {0x6, -1, "Serial ATA Controller"},

    {0x7, 0x0, "Serial Attached SCSI Controller, SAS"},
    {0x7, 0x1, "Serial Attached SCSI Controller, Serial Storage Bus"},
    {0x7, -1, "Serial Attached SCSI Controller"},

    {0x8, 0x1, "Non-Volatile Memory Controller, NVMHCI"},
    {0x8, 0x2, "Non-Volatile Memory Controller, NVMe"},
    {0x8, -1, "Non-Volatile Memory Controller"},

    {0x80, -1, "Other"},

    {0xFF, -1, "Unknown"} // Signifies the end of this table, do NOT remove
};

pci_subclass_kv_t pci_network_cont_descriptors[] = {
    {0x0, -1, "Ethernet Controller"},
    {0x1, -1, "Token Ring Controller"},
    {0x2, -1, "FDDI Controller"},
    {0x3, -1, "ATM Controller"},
    {0x4, -1, "ISDN Controller"},
    {0x5, -1, "WorldFip Controller"},
    {0x6, -1, "PICMG 2.14 Multi Computing Controller"},
    {0x7, -1, "Infiniband Controller"},
    {0x8, -1, "Fabric Controller"},
    {0x80, -1, "Other"},

    {0xFF, -1, "Unknown"} // Signifies the end of this table, do NOT remove
};

pci_subclass_kv_t pci_display_cont_descriptors[] = {
    {0x0, 0x0, "VGA Controller"},
    {0x0, 0x1, "VGA Controller, 8514-Compatible"},
    {0x0, -1, "VGA Compatible Controller"},

    {0x1, -1, "XGA Controller"},
    {0x2, -1, "3D Controller (Not VGA-Compatible)"},
    {0x80, -1, "Other"},

    {0xFF, -1, "Unknown"} // Signifies the end of this table, do NOT remove
};

pci_subclass_kv_t pci_bridge_descriptors[] = {
    {0x0, -1, "Host Bridge"},
    {0x1, -1, "ISA Bridge"},
    {0x2, -1, "EISA Bridge"},
    {0x3, -1, "MCA Bridge"},

    {0x4, 0x0, "PCI-to-PCI Bridge, Normal Decode"},
    {0x4, 0x1, "PCI-to-PCI Bridge, Subtractive Decode"},
    {0x4, -1, "PCI-to-PCI Bridge"},

    {0x5, -1, "PCMCIA Bridge"},
    {0x6, -1, "NuBus Bridge"},
    {0x7, -1, "CardBus Bridge"},

    {0x8, 0x0, "RACEway Bridge, Transparent Mode"},
    {0x8, 0x1, "RACEway Bridge, Endpoint Mode"},
    {0x8, -1, "RACEway Bridge"},

    {0x9, 0x40, "PCI-to-PCI Bridge, Semi-Transparent, primary bus towards CPU"},
    {0x9, 0x80, "PCI-to-PCI Bridge, Semi-Transparent, secondary bus towards CPU"},
    {0x9, -1, "PCI-to-PCI Bridge"},

    {0xA, -1, "InfiniBand-to-PCI Host Bridge"},
    
    {0x80, -1, "Other"},

    {0xFF, -1, "Unknown"} // Signifies the end of this table, do NOT remove
};

pci_subclass_kv_t pci_serial_bus_cont_descriptors[] = {
    {0x0, 0x0, "FireWire (IEEE 1394) Controller (Generic)"},
    {0x0, 0x10, "FireWire (IEEE 1394) Controller (OCHI)"},
    {0x0, -1, "FireWire (IEEE 1394) Controller"},

    {0x1, -1, "ACCESS Bus Controller"},
    {0x2, -1, "SSA"},

    {0x3, 0x0, "UHCI Controller"},
    {0x3, 0x10, "OHCI Controller"},
    {0x3, 0x20, "EHCI (USB2) Controller"},
    {0x3, 0x30, "XHCI (USB3) Controller"},
    {0x3, 0x80, "USB Controller (Unspecified)"},
    {0x3, 0xFE, "USB Device"},
    {0x3, -1, "USB Controller"},

    {0x4, -1, "Fibre Channel"},
    {0x5, -1, "SMBus Controller"},
    {0x6, -1, "InfiniBand Controller"},
    
    {0x7, 0x0, "IPMI Interface, SMIC"},
    {0x7, 0x1, "IPMI Interface, Keyboard Controller Style"},
    {0x7, 0x2, "IPMI Interface, Block Transfer"},
    {0x7, -1, "IPMI Interface"},

    {0x8, -1, "SERCOS Interface (IEC 61491)"},
    {0x9, -1, "CANbus Controller"},
    
    {0x80, -1, "Other"},
    
    {0xFF, -1, "Unknown"} // Signifies the end of this table, do NOT remove
};

pci_descriptors_t* pci_get_descriptors(uint16_t vendor, uint16_t device_id, uint8_t class_, uint8_t subclass, uint8_t prog_if) {
    pci_descriptors_t* descriptors = (pci_descriptors_t*) malloc(sizeof(pci_descriptors_t));

    /* Assigning the vendor descriptor */

    descriptors->vendor = "Unknown";

    int vendor_i = 0;
    pci_vendor_kv_t key_value = {0, "Unknown"};
    do {
        key_value = pci_vendor_descriptors[vendor_i];

        if(key_value.vendor == vendor) {
            descriptors->vendor = key_value.descriptor;
            break;
        }

        vendor_i++;
    } while(key_value.vendor != 0xFFFF && vendor_i < 0xFFFF);

    /* Assigning the class descriptor */

    if((class_ >= 0x14 && class_ < 0x40) || (class_ > 0x40 && class_ < 0xFF)) {
        descriptors->class = "Reserved";
    } else if(class_ == 0x40) {
        descriptors->class = "Co-Processor";
    } else if(class_ == 0xFF) {
        descriptors->class = "Unknown";
    } else {
        descriptors->class = pci_class_descriptors[class_];
    }

    /* Assigning the subclass */

    switch(class_) {
        case 0x1: { // Mass Storage Controllers
            descriptors->subclass = pci_get_subclass_descriptor(pci_mass_storage_descriptors, subclass, prog_if);
        } break;
        case 0x2: { // Network Controllers
            descriptors->subclass = pci_get_subclass_descriptor(pci_network_cont_descriptors, subclass, prog_if);
        } break;
        case 0x3: { // Display Controllers
            descriptors->subclass = pci_get_subclass_descriptor(pci_display_cont_descriptors, subclass, prog_if);
        } break;

        case 0x6: { // Bridges
            descriptors->subclass = pci_get_subclass_descriptor(pci_bridge_descriptors, subclass, prog_if);
        } break;

        case 0xC: { // Serial Bus Controllers
            descriptors->subclass = pci_get_subclass_descriptor(pci_serial_bus_cont_descriptors, subclass, prog_if);
        } break;


        case 0xFF: { // Unsassigned Class (vendor specific)
            descriptors->subclass = "Unassigned Class (vendor specific)";
        } break;

        default: {
            descriptors->subclass = "Unknown";
        } break;
    }

    return descriptors;
}

const char* pci_get_subclass_descriptor(pci_subclass_kv_t* descriptor_arr, uint8_t subclass, uint8_t prog_if) {
    int i = 0;
    pci_subclass_kv_t key_value = descriptor_arr[i];
    do {
        // If this is the right subclass, pick if prog_if matches, or if this has prog_if = -1 (-1 is the "catch all")
        if(key_value.subclass == subclass) {
            if(key_value.prog_if == prog_if || key_value.prog_if == -1) {
                return key_value.descriptor;
            }
        }

        i++;
        key_value = descriptor_arr[i];
    } while(key_value.subclass != 0xFF && i < 0xFF); // 0xFF signifies the end of the table

    return "Unknown";
}
