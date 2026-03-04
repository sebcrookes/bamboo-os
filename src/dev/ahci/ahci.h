#pragma once

#include "../../pci/pci.h"

#define AHCI_GHC_HR 0b1			// HBA Reset
#define AHCI_GHC_IE 0b1 << 1	// Interrupt Enable
#define AHCI_GHC_MRSM 0b1 << 2	// MSI Revert to Single Message
#define AHCI_GHC_AE 0b1 << 31	// AHCI Enable

#define	AHCI_CAP2_BOH 0b1 		// BIOS/OS Handoff supported
#define	AHCI_CAP2_NVMP 0b1 << 1 // NVMHCI Present
#define	AHCI_CAP2_APST 0b1 << 2	// Automatic Partial to Slumber Transitions supported
#define	AHCI_CAP2_SDS 0b1 << 3	// Supports Device Sleep
#define	AHCI_CAP2_SADM 0b1 << 4	// Supports Aggressive Device Sleep Management
#define	AHCI_CAP2_DESO 0b1 << 5	// DevSleep Entrance from Slumber Only

typedef struct __attribute__((packed)) {
    uint32_t clb;
	uint32_t clbu;
	uint32_t fb;
	uint32_t fbu;
	uint32_t is;
	uint32_t ie;
	uint32_t cmd;
	uint32_t rsv0;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sctl;
	uint32_t serr;
	uint32_t sact;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t rsv1[11];
	uint32_t vendor[4];
} hba_port_t;

typedef struct __attribute__((packed)) {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_pts;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;

    uint8_t reserved[0x74];

    uint8_t vendor[0x60];

    hba_port_t ports[1];
} hba_mem_t;

void ahci_init(pci_device_t* device);
