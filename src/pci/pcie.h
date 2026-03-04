#pragma once

#include <stdint.h>

void pcie_probe();

void pcie_probe_bus(uint64_t base_addr, uint64_t start_bus, uint64_t bus);
void pcie_probe_slot(uint64_t base_addr, uint64_t start_bus, uint64_t bus, uint64_t slot);
void pcie_probe_function(uint64_t base_addr, uint64_t start_bus, uint64_t bus, uint64_t slot, uint64_t function);
