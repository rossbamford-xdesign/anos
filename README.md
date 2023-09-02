## anos - An Operating System 💾

> **Note**: This is not yet an operating system. It's not even a toy.
> I may continue to grow it, or I may not. I'm really just doing
> it to learn some things I skipped last time I played around with
> OS dev.

A toy operating system I'm using as a vehicle for learning more about
the low level, figuring out things I skipped last time (like long mode)
and really getting to grips with things I just copy-pasted from other
people (like paging).

This time, I'm copy-pasting nothing. I'm trying to not even _look_ 
at other people's code - as much as possible I'm just going from 
official reference materials (like the Intel manuals).

  - Ross Bamford, August 2023

### Building

Everything is built with `make`. You'll want a cross-compiler
toolchain for `x86_64` (for `ld` at the moment, though I'll almost
certainly end up using `gcc` too) along with cross `binutils` 
(for `objcopy` and `objdump` at least). Latest versions of these
are recommended.

For running and debugging, you'll want `qemu-system-x86_64`. 
Bochs is also supported if you prefer (there's a minimal `bochsrc`
in the repo that will get you going).

To build the FAT filesystem for the floppy image, `mtools` is
needed, specifically `mformat`, `mcopy` - but I don't know if 
you can get just those, and it's best to get the whole suite
anyway as it can be useful for debugging things.

You'll need a sane build environment (i.e. a UNIX) with `make` 
etc. FWIW I work on macOS, YMMV on Linux or WSL (but I expect
it should work fine).

There are probably some test programs and helpers that I use 
in the repo (e.g. `fat.c`). These will need a native Clang or GCC
toolchain - again, sane build environment recommended... 😜

To build, just do:

```shell
make clean all
```

As part of the build, all the objects are built to ELF format with
debugging info, and a disassembly file is created. These are then
stripped to binary

### Running

#### In an emulator

You can use either qemu or Bochs. Obviously you'll need them
installed. 

> **Note** If you're on Mac and want to use Bochs, it's best to 
> build your own from source. The one in brew is kinda broken, 
> in that the display doesn't always work right and the debugger
> has bad keyboard support (no history etc).

To run in qemu:

```shell
make qemu
```

Or Bochs:

```shell
make bochs
```

This latter one is really just running `bochs` directly, but will
also handle building the code and floppy image automatically for 
you. Of course, you can just run `bochs` directly yourself if 
you like - I'm not one to judge.

#### On real hardware

If you want to run this on real hardware, you'll need something
that will either write the `img` file to a floppy as raw sectors,
or something that can burn bootable USB sticks.

Full disclosure, I haven't tested this on actual hardware yet, 
and to be honest I'd be pretty surprised if it worked. I _think_
I've got all the basics covered (retrying disk accesses when they
fail and enabling A20 are both unnecessary in the emulators, but 
the code is written and tested as much as I possibly can) but 
there's every chance things won't work right anyway...

### Debugging

The recommended way to debug is with qemu. Bochs _is_ still supported,
and the debugger built into it isn't _bad_, but full-fat GDB is pretty
hard to beat and it works well with qemu.

For convenience, a `.gdbinit` file is provided that will automate
loading the symbols and connecting to qemu. This can be easily
kicked off with:

```shell
make debug-qemu
```

This will build what needs to be built, start qemu with debugging,
and launch GDB automatically.

> **Note** You'll want NASM 2.16 or higher if you have a modern
> GDB (13 and up) - I've observed issues with loading DWARF data
> generated by NASM 2.15, which causes symbol clashes and sources 
> not to be loaded.

Because symbols and sources are loaded, you can set breakpoints 
easily based on labels or line numbers, e.g:

```gdb
b stage2.asm:_start
b stage1.asm:35
```

If you prefer to use debugging in an IDE or have some other alternative
GDB frontend you like to use, you can just run:

```shell
make debug-qemu-start
```

which will skip starting GDB for you, allowing you to launch 
your frontend and connect (`localhost:9666` by default).


