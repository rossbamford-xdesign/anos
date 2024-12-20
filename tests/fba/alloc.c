/*
 * Tests for the fixed-block allocator
 * anos - An Operating System
 *
 * Copyright (c) 2024 Ross Bamford
 */

#include <stdint.h>
#include <stdlib.h>

#include "fba/alloc.h"
#include "munit.h"
#include "test_pmm.h"
#include "test_vmm.h"
#include "vmm/vmmapper.h"

#define TEST_PML4_ADDR (((uint64_t *)0x100000))
#define TEST_PAGE_COUNT ((4)) // only need allocated mem for the bitmap...

// These are conditionally defined in the main code, which sucks, but works "for
// now"...
uintptr_t test_fba_check_begin();
uintptr_t test_fba_check_size();

static void *test_setup(const MunitParameter params[], void *user_data) {
    void *page_area_ptr;
    posix_memalign(&page_area_ptr, 0x1000, TEST_PAGE_COUNT << 12);
    return page_area_ptr;
}

static void test_teardown(void *page_area_ptr) {
    free(page_area_ptr);
    test_pmm_reset();
    test_vmm_reset();
}

static MunitResult test_fba_init_zero(const MunitParameter params[],
                                      void *param) {
    bool result = fba_init(0, 0, 0);

    // succeeds
    munit_assert_true(result);

    // State is set correctly
    munit_assert_uint64((uint64_t)test_fba_check_begin(), ==, 0);
    munit_assert_uint64((uint64_t)test_fba_check_size(), ==, 0);

    // No pages allocated for the bitmap because zero size...
    munit_assert_uint32(test_pmm_get_total_page_allocs(), ==, 0);

    return MUNIT_OK;
}

static MunitResult test_fba_init_unaligned_begin(const MunitParameter params[],
                                                 void *param) {
    bool result;

    result = fba_init(TEST_PML4_ADDR, 0x001, 100);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0xfff, 100);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0x1001, 100);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0x1fff, 100);
    munit_assert_false(result);

    return MUNIT_OK;
}

static MunitResult
test_fba_init_size_not_multiple(const MunitParameter params[], void *param) {
    bool result;

    result = fba_init(TEST_PML4_ADDR, 0x1000, 1);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0x1000, 32767);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0x1000, 32769);
    munit_assert_false(result);

    result = fba_init(TEST_PML4_ADDR, 0x1000, 65535);
    munit_assert_false(result);

    return MUNIT_OK;
}

static MunitResult test_fba_init_32768_ok(const MunitParameter params[],
                                          void *test_page_area) {
    bool result = fba_init(TEST_PML4_ADDR, (uintptr_t)test_page_area, 32768);

    // succeeds
    munit_assert_true(result);

    // State is set correctly
    munit_assert_ptr((void *)test_fba_check_begin(), ==, test_page_area);
    munit_assert_uint64((uint64_t)test_fba_check_size(), ==, 32768);

    // One page allocated for bitmap (32768 bits)
    munit_assert_uint32(test_pmm_get_total_page_allocs(), ==, 1);
    munit_assert_uint32(test_vmm_get_total_page_maps(), ==, 1);

    // Page was mapped into the correct place (first page in the area)...
    munit_assert_uint64(test_vmm_get_last_page_map_paddr(), ==,
                        TEST_PMM_NOALLOC_START_ADDRESS);
    munit_assert_uint64(test_vmm_get_last_page_map_vaddr(), ==,
                        (uint64_t)test_page_area);
    munit_assert_uint16(test_vmm_get_last_page_map_flags(), ==,
                        WRITE | PRESENT);
    munit_assert_uint64(test_vmm_get_last_page_map_pml4(), ==,
                        (uint64_t)TEST_PML4_ADDR);

    return MUNIT_OK;
}

static MunitResult test_fba_init_65536_ok(const MunitParameter params[],
                                          void *test_page_area) {
    bool result = fba_init(TEST_PML4_ADDR, (uintptr_t)test_page_area, 65536);

    // succeeds
    munit_assert_true(result);

    // State is set correctly
    munit_assert_ptr((void *)test_fba_check_begin(), ==, test_page_area);
    munit_assert_uint64((uint64_t)test_fba_check_size(), ==, 65536);

    // Two pages allocated for bitmap (65536 bits)
    munit_assert_uint32(test_pmm_get_total_page_allocs(), ==, 2);
    munit_assert_uint32(test_vmm_get_total_page_maps(), ==, 2);

    // Last page was mapped into the correct place (second page in the area)...
    munit_assert_uint64(test_vmm_get_last_page_map_paddr(), ==,
                        TEST_PMM_NOALLOC_START_ADDRESS + 0x1000);
    munit_assert_uint64(test_vmm_get_last_page_map_vaddr(), ==,
                        ((uint64_t)test_page_area) + 0x1000);
    munit_assert_uint16(test_vmm_get_last_page_map_flags(), ==,
                        WRITE | PRESENT);
    munit_assert_uint64(test_vmm_get_last_page_map_pml4(), ==,
                        (uint64_t)TEST_PML4_ADDR);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
        {(char *)"/init_zero", test_fba_init_zero, NULL, NULL,
         MUNIT_TEST_OPTION_NONE, NULL},
        {(char *)"/init_unaligned_begin", test_fba_init_unaligned_begin, NULL,
         NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *)"/init_size_not_multiple", test_fba_init_size_not_multiple,
         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *)"/init_32768_ok", test_fba_init_32768_ok, test_setup,
         test_teardown, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *)"/init_65536_ok", test_fba_init_65536_ok, test_setup,
         test_teardown, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

static const MunitSuite test_suite = {(char *)"/fba", test_suite_tests, NULL, 1,
                                      MUNIT_SUITE_OPTION_NONE};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, (void *)"µnit", argc, argv);
}
