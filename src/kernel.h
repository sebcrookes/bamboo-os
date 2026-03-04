#pragma once

#include <stdint.h>

#include "bootdata.h"

void kernel_init(boot_data_t* bootdata);

void kernel_print_logo();
