/*
 * Mock implementation of machine.c for hosted tests
 * anos - An Operating System
 *
 * Copyright (c) 2024 Ross Bamford
 *
 * Generally-useful machine-related routines
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#define BUFFER_SIZE 1024
#define BUFFER_MASK ((BUFFER_SIZE - 1))

// 128MiB of ring buffers, could be more efficient but whatever, this is test
// code...
static uint32_t out_buffer[65536][1024];
static uint32_t in_buffer[65536][1024];

static uint32_t in_buffer_read_ptr[65536];
static uint32_t in_buffer_write_ptr[65536];
static uint32_t out_buffer_read_ptr[65536];
static uint32_t out_buffer_write_ptr[65536];

void test_machine_reset(void) {
    for (int i = 0; i < 65536; i++) {
        in_buffer_read_ptr[i] = 0;
        in_buffer_write_ptr[i] = 0;
        out_buffer_read_ptr[i] = 0;
        out_buffer_write_ptr[i] = 0;
    }
}

inline bool test_machine_outl_avail(uint16_t port) {
    return out_buffer_read_ptr[port] != out_buffer_write_ptr[port];
}

uint32_t test_machine_read_outl_buffer(uint16_t port) {
    if (!test_machine_outl_avail(port)) {
        return 0;
    }

    uint32_t result = out_buffer[port][out_buffer_read_ptr[port]++];
    out_buffer_read_ptr[port] &= BUFFER_MASK;
    return result;
}

bool test_machine_write_outl_buffer(uint16_t port, uint32_t value) {
    if ((out_buffer_write_ptr[port] == out_buffer_read_ptr[port] - 1) ||
        (in_buffer_write_ptr[port] == 0xffff &&
         in_buffer_read_ptr[port] == 0)) {
        return false;
    }

    out_buffer[port][out_buffer_write_ptr[port]++] = value;
    out_buffer_write_ptr[port] &= BUFFER_MASK;

    return true;
}

inline bool test_machine_inl_avail(uint16_t port) {
    return in_buffer_read_ptr[port] != in_buffer_write_ptr[port];
}

uint32_t test_machine_read_inl_buffer(uint16_t port) {
    if (!test_machine_inl_avail(port)) {
        return 0;
    }

    uint32_t result = in_buffer[port][in_buffer_read_ptr[port]++];
    in_buffer_read_ptr[port] &= BUFFER_MASK;
    return result;
}

bool test_machine_write_inl_buffer(uint16_t port, uint32_t value) {
    if ((in_buffer_write_ptr[port] == in_buffer_read_ptr[port] - 1) ||
        (in_buffer_write_ptr[port] == 0xffff &&
         in_buffer_read_ptr[port] == 0)) {
        return false;
    }

    in_buffer[port][in_buffer_write_ptr[port]++] = value;
    in_buffer_write_ptr[port] &= BUFFER_MASK;

    return true;
}

/* Implementation of machine.c interface */

noreturn void halt_and_catch_fire(void) { exit(100); }

void outl(uint16_t port, uint32_t value) {
    if (!test_machine_write_outl_buffer(port, value)) {
        fprintf(stderr,
                "WARN: test_machine outl [port 0x%04x value 0x%8x] discarded: "
                "buffer full\n",
                port, value);
    }
}

uint32_t inl(uint16_t port) {
    if (!test_machine_inl_avail(port)) {
        fprintf(stderr,
                "WARN: test_machine inll [port 0x%04x] underflow: buffer "
                "empty\n",
                port);
        return 0;
    }

    return test_machine_read_inl_buffer(port);
}
