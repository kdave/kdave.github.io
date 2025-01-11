---
layout: page
category: b
tags: [ ntfs, btrfs ]
date: 2024-12-18
status: final
title: "Case study: link C++ standalone executable to another C executable (ntfs2btrfs to btrfs-convert)"
---

*Problem statement*: there's a tool written in C, `btrfs-convert`, that
implements conversion from one filesystem to btrfs. For ext2/3/4 and reiserfs
this is done using existing C libraries. There's a standalone project
`ntfs2btrfs` written in C++, not providing a library interface at all, but the
equivalent functionality.

*The question*: can the functionality of `ntfs2btrfs` be provided by
`btrfs-convert` while minimizing the difficulty of the integration? Ideally
automated with only a few necessary fix-ups.

* Contents
{:toc}

# Inputs

[**btrfs-progs.git**](https://github.com/kdave/btrfs-progs.git)

- primary build system and target
- C sources
- autoconf, make

[**ntfs2btrfs.git**](https://github.com/maharmstone/ntfs2btrfs)
- C++ sources
- cmake

What can certainly be done is to `git clone` the sources, run their respective
configure and build steps and get lots of `.o` files.

# Initial problems

The `ntfs2btrfs` conversion tool is in a different repository, git submodules
are not an option so this needs to be cloned and pulled at the time
`btrfs-convert` is built.

The other build system should be ideally run without any additional hurdles,
so we can assume it's in a known separate directory and produces the object files
we'll use later. But this also builds a standalone executable, so there's
a symbol `main` defined, which will collide with the one defined by the
`btrfs-convert` sources.

The language difference is not that significant but in the glue interface
we need to handle potential name mangling or other symbol duplication.

There are duplicate implementations of low-level functions like checksumming.
The estimated size is tens of kilobytes, so we won't mind that as long as
the linker is able to resolve everything.

# Solution outline

To integrate the projects the steps will be in a script that will basically:

- clone the `ntfs2btrfs.git` repository
- configure it and build with some sane options (preferring maximum compatibility)
- grab the resulting `.o` files
- link `btrfs-convert` and the `.o` files

The last point can be tricky and the missing part is how to hand over
the functionality.

# Create the entry point

The entry point of the standalone `ntfs2btrfs` utility is `main` so we need
to inject a public function that will not be a mangled as symbol, export
it in a header and use it from `btrfs-convert`.

Also we may need to translate the user options set as command line options.
They'll be parsed in the `ntfs2btrfs` project and passed as parameters to the
glue function and mapped to the conversion options if possible. There's some
feature mismatch so this is not 1:1.

Minimal glue function (:

```cpp
extern "C" {
        void convert_entry(const char *fn_c) {
                enum btrfs_compression compression;
                enum btrfs_csum_type csum_type;
                bool nocsum = false;

                csum_type = btrfs_csum_type::crc32c;
                compression = btrfs_compression::none;

                string fn(fn_c);
                ntfs dev(fn);

                convert(dev, compression, csum_type, nocsum);
        }
}
```

This can be appended/included to the main file so it lives in the scope of the
`ntfs2btrfs` where its `main()` is, so we have access to the translation unit
static and global variables (that can store the representation of the command
line options).

```sh
echo '#include "convert-entry.c"' >> "ntfs2btrfs.git/src/ntfs2btrfs.cpp"
```

Symbol name mangling is avoided by `extern "C"`, otherwise the parameters are
set to defaults for simplicity. The path of the file/device must be translated
to a C++ `string` but this happens inside the glue function, no
interoperability problems. After creation of the filesystem object `ntfs` we
can finally hand control to `convert()`.

There are some assumptions. The conversion detects the NTFS signature and does
other validation of the image as we'll call that from `btrfs-convert`
unconditionally.

# What about the two main()s

Actually this is not hard at all. Linkage is done by comparing plain string
representations, so we'll just edit it as needed. The colliding symbol name is
`main` so let's rename it to `MAIN`. This is a standard symbol in C++ that's not
mangled so we can simply find in the right `.o` file and change it. We can
use `sed`.

```sh
$ nm build/.../ntfs2btrfs.o | grep main
00000000000140ed T main
...
$ sed -i -e 's/main/MAIN/' build/.../ntfs2btrfs.o
$ nm build/.../ntfs2btrfs.o | grep MAIN
00000000000140ed T MAIN
...
```

This is crude and renames several other symbols like lambdas and templates of
`std::basic_string` (`00000000000174c9 t
std::__cxx11::basic_string<MAIN::{lambda()#1}::operator()()
const::FMT_COMPILE_STRING::char_type, ...`). I haven't found a standard utility
to that so `sed` it is, fortunately there's no other string `main` in the
binary e.g. in an error message.

# Final link

No remaining problems, we have a bunch of `.o` files, no conflicting symbol
names, one executable entry point `main` and one final binary. So we now can
link `btrfs-convert` with all the other objects. This is basic makefile
integration, not that interesting, also some conditional build support in the
`btrfs-convert` sources. With everything in place

```sh
$ ./btrfs-convert --help
...
    Supported filesystems:
        ext2/3/4: yes
        reiserfs: no
        ntfs: yes
$ mkfs.ntfs /dev/sdx
$ ./btrfs-convert /dev/sdx
btrfs-convert from btrfs-progs v6.12
...
Using CRC32C for checksums.
```

And the last question: *does anybody what that?*
