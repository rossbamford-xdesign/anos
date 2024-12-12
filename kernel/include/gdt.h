/*
 * stage3 - GDT manipulation and setup routines
 * anos - An Operating System
 *
 * Copyright (c) 2024 Ross Bamford
 */

#ifndef __ANOS_KERNEL_GDT_H
#define __ANOS_KERNEL_GDT_H

#include <stdint.h>

#define GDT_ENTRY_ACCESS_ACCESSED 0x01
#define GDT_ENTRY_ACCESS_READ_WRITE 0x02
#define GDT_ENTRY_ACCESS_DOWN_CONFORMING 0x04
#define GDT_ENTRY_ACCESS_EXECUTABLE 0x08
#define GDT_ENTRY_ACCESS_NON_SYSTEM 0x10
#define GDT_ENTRY_ACCESS_DPL_MASK 0x60
#define GDT_ENTRY_ACCESS_PRESENT 0x80

#define GDT_ENTRY_ACCESS_DPL(dpl) (((dpl & 0x03) << 5))

#define GDT_ENTRY_ACCESS_RING0 0x00
#define GDT_ENTRY_ACCESS_RING1 0x20
#define GDT_ENTRY_ACCESS_RING2 0x40
#define GDT_ENTRY_ACCESS_RING3 0x60

#define GDT_ENTRY_FLAGS_LONG_MODE 0x20
#define GDT_ENTRY_FLAGS_SIZE 0x40
#define GDT_ENTRY_FLAGS_GRANULARITY 0x80

#define GDT_ENTRY_FLAGS_64BIT ((GDT_ENTRY_FLAGS_LONG_MODE))

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

// Structure representing a GDT entry
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t flags_limit_h;
    uint8_t base_high;
} __attribute__((packed)) GDTEntry;

// Execute `lgdt` to load a variable with the GDTR
static inline void load_gdtr(GDTR *gdtr) {
    __asm__ __volatile__("lgdt (%0)" : : "r"(gdtr));
}

// Execute `sgdt` to load GDTR from a variable
static inline void store_gdtr(GDTR *gdtr) {
    __asm__ __volatile__("sgdt (%0)" : : "r"(gdtr));
}

// Function to get a GDT entry given a GDTR and index
GDTEntry *get_gdt_entry(GDTR *gdtr, int index);

// Update values in a GDT entry. Caller should disable interrupts!
void init_gdt_entry(GDTEntry *entry, uint32_t base, uint32_t limit,
                    uint8_t access, uint8_t flags_limit_h);

#endif //__ANOS_KERNEL_GDT_H
