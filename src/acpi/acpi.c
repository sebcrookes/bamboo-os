#include "acpi.h"

#include "../inc/faults.h"
#include "../inc/stdio.h"

#include "madt.h"
#include "mcfg.h"

#include "../mem/vmm.h"

bool acpi_is_xsdt = false;

rsdp_t* rsdp_v1;
rsdp_v2_t* rsdp_v2;
sdt_hdr_t* rsdt;
sdt_hdr_t* xsdt;

uint64_t acpi_higher_half_offset = 0;

void acpi_init(boot_data_t* bootdata) {
    acpi_higher_half_offset = bootdata->higher_half.offset;
    acpi_parse_rsdp((void*) bootdata->rsdp.address);

    if(acpi_is_xsdt) {
        acpi_parse_xsdt();
    } else {
        acpi_parse_rsdt();
    }

    printf("%C[ACPI]%C - Validated RSDP and ", COLOUR_KERNEL_INFO, COLOUR_PRINT);

    if(acpi_is_xsdt) {
        printf("XSDT. Found:\n");
        acpi_parse_xsdt_tables();
    } else {
        printf("RSDT. Found:\n");
        acpi_parse_rsdt_tables();
    }
}

void acpi_parse_rsdp(void* rsdp_address) {
    /* === Loading the RSDP V1 structure === */

    rsdp_t* table_v1 = (rsdp_t*)(void*)((uint64_t) rsdp_address + acpi_higher_half_offset);

    /* === Validating RSDP signature === */
    // Checking that signature == "RSD PTR "

    bool valid_signature = true;
    const char* expected_sig = "RSD PTR ";

    for(int i = 0; i < 8; i++) {
        if(table_v1->signature[i] != expected_sig[i]) {
            valid_signature = false;
            break;
        }
    }

    faults_assert(valid_signature, "Invalid RSDP V1 signature");

    /* === Validating RSDP V1.0 checksum === */
    /* To validate, add up the values of every byte in the structure, then
        cast to a uint8_t. If the value == 0, the structure is valid */

    uint64_t sum = 0;
    for(uint64_t i = 0; i < sizeof(rsdp_t); i++) {
        sum += ((uint8_t*) table_v1)[i];
    }

    faults_assert(sum % 0x100 == 0, "Invalid RSDP V1 checksum");

    /* === Checking revision === */

    if(table_v1->revision == 0) { // V1 - we use the RSDT
        rsdp_v1 = table_v1;
    } else if(table_v1->revision == 2) { // V2-current - extended RSDP and we use the XSDT
        /* === Loading the full RSDP V2 structure === */

        rsdp_v2_t* table_v2 = (rsdp_v2_t*)(void*)((uint64_t) rsdp_address + acpi_higher_half_offset);

        /* === Validating RSDP V2 checksum === */
        // To validate, do the same as with V1, with the whole table

        uint64_t sum2 = 0;
        for(uint64_t i = 0; i < sizeof(rsdp_v2_t); i++) {
            sum2 += ((uint8_t*) table_v2)[i];
        }

        faults_assert(sum2 % 0x100 == 0, "Invalid RSDP V2 checksum");

        // Everything is in order and the table is valid
        rsdp_v2 = table_v2;
    }
}

void acpi_parse_rsdt() {
    sdt_hdr_t* rsdt_hdr = (sdt_hdr_t*)(void*)(rsdp_v1->rsdt_address + acpi_higher_half_offset);

    bool signature_valid = acpi_is_signature_equal(rsdt_hdr, "RSDT");
    faults_assert(signature_valid, "Invalid RSDT header signature");

    bool valid = acpi_is_valid_sdt_checksum(rsdt_hdr);
    faults_assert(valid, "Invalid RSDT header checksum");

    rsdt = rsdt_hdr;
}

void acpi_parse_rsdt_tables() {
    uint64_t entries = (rsdt->length - sizeof(sdt_hdr_t)) / 4;

    // After the RSDT header, there is an array of pointers to other SDTs
    uint32_t* sdts = (uint32_t*)(rsdt + 1);

    for(int i = 0; i < entries; i++) {
        printf(" - ");
        sdt_hdr_t* header = (sdt_hdr_t*)(void*)(((uint64_t) sdts[i]) + acpi_higher_half_offset);
        for(int j = 0; j < 4; j++) {
            printf("%c", header->signature[j]);
        }

        /* Check if table should be parsed */

        bool has_parsed = true;

        if(acpi_is_signature_equal(header, "APIC")) {
            // printf(": ");
            // acpi_parse_madt();
            printf("\n");
        } else if(acpi_is_signature_equal(header, "MCFG")) {
            printf(": ");
            acpi_parse_mcfg();
        } else {
            has_parsed = false;
            printf("\n");
        }
    }
}

void acpi_parse_xsdt() {
    sdt_hdr_t* xsdt_hdr = (sdt_hdr_t*)(void*)(rsdp_v2->xsdt_address + acpi_higher_half_offset);

    bool signature_valid = acpi_is_signature_equal(xsdt_hdr, "XSDT");
    faults_assert(signature_valid, "Invalid XSDT header signature");

    bool valid = acpi_is_valid_sdt_checksum(xsdt_hdr);
    faults_assert(valid, "Invalid XSDT header checksum");

    xsdt = xsdt_hdr;
}

void acpi_parse_xsdt_tables() {
    uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

    // After the XSDT header, there is an array of pointers to other SDTs
    uint64_t* sdts = (uint64_t*)(xsdt + 1);

    for(int i = 0; i < entries; i++) {
        printf(" - ");
        sdt_hdr_t* header = (sdt_hdr_t*)(void*)(sdts[i] + acpi_higher_half_offset);
        for(int j = 0; j < 4; j++) {
            printf("%c", header->signature[j]);
        }

        /* Check if table should be parsed */

        bool has_parsed = true;

        if(acpi_is_signature_equal(header, "APIC")) {
            printf("\n");
            // parse_madt();
        } else if(acpi_is_signature_equal(header, "MCFG")) {
            printf(": ");
            acpi_parse_mcfg();
        } else {
            has_parsed = false;
            printf("\n");
        }
    }
}

sdt_hdr_t* acpi_get_table(const char* signature) {
    if(acpi_is_xsdt) {
        uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

        // After the XSDT header, there is an array of pointers to other SDTs
        uint64_t* sdts = (uint64_t*)(xsdt + 1);

        for(int i = 0; i < entries; i++) {
            sdt_hdr_t* hdr = (sdt_hdr_t*)(((uint64_t) sdts[i]) + acpi_higher_half_offset);

            if(acpi_is_signature_equal(hdr, signature)) return hdr;
        }
    } else {
        uint64_t entries = (rsdt->length - sizeof(sdt_hdr_t)) / 4;

        // After the XSDT header, there is an array of pointers to other SDTs
        uint32_t* sdts = (uint32_t*)(rsdt + 1);

        for(int i = 0; i < entries; i++) {
            sdt_hdr_t* hdr = (sdt_hdr_t*)(((uint64_t)sdts[i]) + acpi_higher_half_offset);

            if(acpi_is_signature_equal(hdr, signature)) return hdr;
        }
    }

    return NULL;
}

/* Helper functions */

bool acpi_table_exists(const char* signature) {
    uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

    // After the XSDT header, there is an array of pointers to other SDTs
    uint64_t* sdts = (uint64_t*)(xsdt + 1);

    for(int i = 0; i < entries; i++) {
        sdt_hdr_t* hdr = (sdt_hdr_t*)(sdts[i]);

        if(acpi_is_signature_equal(hdr, signature)) return true;
    }

    return false;
}

bool acpi_is_signature_equal(sdt_hdr_t* hdr, const char* signature) {
    for(int i = 0; i < 4; i++) {
        if(hdr->signature[i] != signature[i]) return false;
    }

    return true;
}

bool acpi_is_valid_sdt_checksum(sdt_hdr_t* hdr) {
    /* This function sums all of the bytes in the header (including the checksum)
        If the result (modulo 0x100, aka. cast to a uint8) is 0, then the hdr is valid */
    uint64_t sum = 0;
    for(uint64_t i = 0; i < hdr->length; i++) {
        sum += ((uint8_t*) hdr)[i];
    }
    return (sum % 0x100 == 0);
}
