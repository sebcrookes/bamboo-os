#pragma once

#include <stdbool.h>

__attribute__((noreturn)) void faults_hang();
__attribute__((noreturn)) void faults_panic(char* message);
void faults_assert(bool condition, char* failure_message);
