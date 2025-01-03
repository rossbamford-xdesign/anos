/*
 * stage3 - Syscall implementations
 * anos - An Operating System
 *
 * Copyright (c) 2024 Ross Bamford
 */

#include "syscalls.h"
#include "debugprint.h"
#include "printhex.h"

#include <stdint.h>

static SyscallResult handle_testcall(SyscallArg arg0, SyscallArg arg1,
                                     SyscallArg arg2, SyscallArg arg3,
                                     SyscallArg arg4) {
    debugstr("TESTCALL: ");
    printhex8(arg0, debugchar);
    debugstr(" : ");
    printhex8(arg1, debugchar);
    debugstr(" : ");
    printhex8(arg2, debugchar);
    debugstr(" : ");
    printhex8(arg3, debugchar);
    debugstr(" : ");
    printhex8(arg4, debugchar);
    debugstr(" = ");

    return 42;
}

static SyscallResult handle_debugprint(char *message) {
    if (((uint64_t)message & 0xf000000000000000) == 0) {
        debugstr(message);
    }

    return SYSCALL_OK;
}

SyscallResult handle_syscall_69(SyscallArg arg0, SyscallArg arg1,
                                SyscallArg arg2, SyscallArg arg3,
                                SyscallArg arg4, SyscallArg syscall_num) {
    switch (syscall_num) {
    case 0:
        return handle_testcall(arg0, arg1, arg2, arg3, arg4);
    case 1:
        return handle_debugprint((char *)arg0);
    default:
        return SYSCALL_BAD_NUMBER;
    }
}