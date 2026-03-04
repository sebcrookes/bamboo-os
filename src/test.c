#include "test.h"

#include "inc/stdio.h"
#include "inc/faults.h"

#include "mem/pmm.h"
#include "mem/vmm.h"

#include "inc/vector.h"

/* === stdio testing === */

void test_stdio() {
    /* Expected output:
        Test
        A string: hello
        A char: c
        An unsigned int: 154398
        Signed integers: 250, -250
        Hex: 0xFF5500, 0xff5500
        Multi-coloured text (coloured in red, text in green)
        Back to normal
    */
    printf("Test\n");
    printf("A string: %s\n", "hello");
    printf("A char: %c\n", 'c');
    printf("An unsigned int: %u\n", 154398);
    printf("Signed integers: %d, %d\n", 250, -250);
    printf("Hex: %X, %x\n", 0xFF5500, 0xFF5500);
    printf("Multi-%Ccoloured %Ctext\n", 0xFF0000, 0x00FF00);
    printf("Back to normal\n");
}

/* === VMM Testing === */

uint64_t VMM_TEST_MAP_TO = 0x10000;
#define VMM_TEST_CONSTANT 0xDEADBEEF

/* This function tests whether or not the VMM is working properly. It depends on the PMM
   functioning properly. It first allocates a page, and writes the VMM_TEST_CONSTANT to
   that page. It then maps that page to a virtual address (VMM_TEST_MAP_TO) and reads from
   that virtual address. If it matches the VMM_TEST_CONSTANT, map_page is functional. Next,
   we test vmm_virt_to_phys to ensure it returns the original physical address of the page.
   Finally, we unmap the page. */
void test_vmm() {
    // Allocation of page
    printf("%C[Test/VMM]%C - ", COLOUR_KERNEL_INFO, COLOUR_PRINT);
    uint64_t* virt_page_allocated = (uint64_t*)(pmm_alloc());
    uint64_t* phys_page_allocated = vmm_virt_to_phys((void*)virt_page_allocated);

    // Writing to the original page
    virt_page_allocated[0] = VMM_TEST_CONSTANT;

    // Mapping the page to VMM_TEST_MAP_TO
    vmm_map_page((void*) virt_page_allocated, (void*) VMM_TEST_MAP_TO);

    // Checking if the value in the page is correct
    uint64_t* test = (uint64_t*)VMM_TEST_MAP_TO;
        
    if(test[0] == VMM_TEST_CONSTANT) {
        printf("map_page success, ");
    } else {
        printf("%Cmap_page failed\n", COLOUR_PRINT_ERROR);
        faults_hang();
    }

    // Checking that virt_to_phys is working properly
    if((uint64_t)phys_page_allocated == (uint64_t)vmm_virt_to_phys((void*)VMM_TEST_MAP_TO)) {
        printf("virt_to_phys success (%X -> %X), ", phys_page_allocated, VMM_TEST_MAP_TO);
    } else {
        printf("%Cvirt_to_phys failed\n", COLOUR_PRINT_ERROR);
        faults_hang();
    }

    // Unmapping the page
    vmm_unmap_page((void*)VMM_TEST_MAP_TO);

    // Accessing VMM_TEST_MAP_TO now would cause a page fault, so we assume it worked
    printf("unmap_page success\n");

    vmm_map_page((void*) phys_page_allocated, (void*) virt_page_allocated);

    pmm_free(virt_page_allocated);
}
