---
layout: page
category: b
tags: [ crypto, linux, blake2 ]
feed: pko
date: 2019-04-27
status: finished
title: "Linux crypto: testing blake2s"
---

The BLAKE2 algorithm is out for some time, yet there's no port to linux crypto
API. The recent push of WireGuard would add it to linux kernel, but using a
different API (zinc) than the existing one. I haven't explored zinc, I assume
there will be a way to use it from inside kernel, but this would need another
layer to switch the APIs according to the algorithm.

As btrfs is going to use more hashing algos, we are in the process of
selection.  One of the contenders is BLAKE2, though everybody would have a
different suggestion.  In order to test it, a port is needed. Which basically is
a glue code between the linux crypto API and the BLAKE2 implementation.

I'm not going to reimplement crypto or anything, so the the default
implementation is going to be the reference one found in blake2 git.

*Note: due to the maximum space of 32 bytes available in the btrfs metadata
blocks, the version is BLAKE2s.*

## The BLAKE2s sources

From the repository [https://github.com/BLAKE2/BLAKE2](https://github.com/BLAKE2/BLAKE2):

* ref/blake2s-ref.c
* ref/blake2.h
* ref/blake2-impl.h

Briefly skimming over the sources, there's nothing that'll cause trouble.
Standard C code, some definitions. Adapting for linux kernel would need to
replace the `stdint.h` types (`uint8_t` etc) with the uXX (`u8` etc) and change
path to includes. For simplicity, let's remove the ifdefs for C++ and MSVC too.

## Add the new algorithm definition (CRA)

Though it's possible to prepare a module for an out-of-tree build (see below),
let's do it inside the `linux.git/crypto/` path for now. There's also plenty of
sources to copy & paste. I've used `crc32c_generic.c`, and it turned out to be
useful.

The crypto hash description is defined in `struct shash_alg`, contains the
technical description like type of the algorithm, length of the context and
callbacks for various phases of the hash calculation (init, update, final), and
identification of the algorithm and the implementation. The default
implementations are C and use the string "-generic" in the name.

The crc32c module came with a few stub callbacks (`checksum_init` etc), that
will only call into the blake2 functions and the definition can stay. Simple
search and replace from crc32c to blake2s will do most of the conversion.

## Add the glue code for crypto API

Now we have the `blake2s.c` with the reference implementation, crypto algorithm
definition. The glue code connects the API with the implementation. We need 2
helper structures that hold the context once the user starts digest calculation.
The private blake2s context is embedded into one of them.  The intermediate
results will be stored there.

And the rest is quite easy, each callback will call into the respective blake2s
function, passing the context, data and lengths. One thing that I copied from
the examples is the key initialization that's in `blake2s_cra_init`, that
appears to be global and copied to the context each time a new one is
initialized.

Here the choice of using `crc32c.c` helped as there were the stub callback with
the right parameters, calling the blake2s functions that can retain their
original signature. This makes later updates easier.  All the functions are
static so the compiler will most probably optimize the code that there will be
no unnecessary overhead.

Well and that's it. Let's try to compile it and insert the module:

```
linux.git$ make crypto/blake2s.o
...
  CC [M]  crypto/blake2s.o
linux.git$ make crypto/blake2s.ko
...
  CC      crypto/blake2s.mod.o
  LD [M]  crypto/blake2s.ko
...
```

Check that it's been properly loaded

```
linux.git/crypto$ sudo insmod ./blake2s.ko
```

and registered

```
linux.git$ cat /proc/crypto
name         : blake2s
driver       : blake2s-generic
module       : blake2s
priority     : 100
refcnt       : 1
selftest     : passed
internal     : no
type         : shash
blocksize    : 1
digestsize   : 32
...
```

The selftest says it passed, but there no such thing so far. There are test
values provided in blake2 git so it would be nice to have too (tm). But
otherwise it looks good.

To do actual test, we'd need something inside kernel to utilize the new hash.
One option is to implement a module that will do that or use the userspace
library `libkcapi` that can forward the requests from userspace to the
available kernel implementations.

## Test it with libkcapi

The `libkcapi` project at
[http://www.chronox.de/libkcapi.html](http://www.chronox.de/libkcapi.html)
provides an API that uses the `AF_ALG` socket type to exchange data with
kernel. The library provides a command line tool that we can use right away and
don't need to code anything.

```
$ kcapi-dgst -c blake2s --hex < /dev/null
48a8997da407876b3d79c0d92325ad3b89cbb754d86ab71aee047ad345fd2c49
```

The test vectors provided by blake2 confirm that this is hash of empty string
with the default key (0x000102..1f).

## Out-of-tree build

Sources in the linux.git require one additional line to Makefile, build it
unconditionally as a module. Proper submission to linux kernel would need the
Kconfig option.

```
obj-m += blake2s.o
```

The standalone build needs a Makefile with a few targets that use the existing
build of kernel. Note that you'd need a running kernel with the same built
sources. This is usually provided by the `kernel-*-devel` packages. Otherwise,
if you build kernels from git, you know what to do, right?

```
KDIR ?= /lib/modules/`uname -r`/build
obj-m := blake2s.o

default:
        $(MAKE) -C $(KDIR) M=$$PWD

clean:
        $(MAKE) -C $(KDIR) M=$$PWD clean

modules_install:
        $(MAKE) -C $(KDIR) M=$$PWD modules_install
```

After running `make`, the kernel module is ready for use.

## What next?

Send it upstream. Well, after some work of course.

* update the coding style of the blake2 sources
* add Kconfig
* write self-tests
* optionally add the optimized implementations

All the files can be found here:

* [Makefile](Makefile): out-of-tree build
* [blake2.h](blake2.h): copied and updated for linux
* [blake2-impl.h](blake2.h): copied and updated for linux
* [blake2s.c](blake2.h): copied and updated for linux

## References

* [https://blake2.net](https://blake2.net)
* [https://github.com/BLAKE2/BLAKE2](https://github.com/BLAKE2/BLAKE2):
* [http://www.chronox.de/libkcapi.html](http://www.chronox.de/libkcapi.html)
* [https://www.kernel.org/doc/html/latest/crypto/userspace-if.html](https://www.kernel.org/doc/html/latest/crypto/userspace-if.html)
* [https://www.kernel.org/doc/html/latest/crypto/intro.html](https://www.kernel.org/doc/html/latest/crypto/intro.html)
