---
layout: page
category: b
tags: [ btrfs, blake2, kernel ]
feed: pko
date: 2019-10-08
status: finished
title: Selecting the next checksum for btrfs
---

The currently used and the only one checksum algorithm that's implemented in
btrfs is crc32c, with 32bit digest. It has served well for the years but has
some weaknesses so we're looking for a replacements and also for enhancing the
use cases based on possibly better hash algorithms.

The advantage of crc32c is it's simplicity of implementation, various optimized
versions exist and hardware CPU instruction-level support. The error detection
strength is not great though, the collisions are easy to generate. Note that
crc32c has not been used in cryptographically sensitive context (eg.
deduplication).

Side note: the collision generation weakness is used in the filesystem image
dump tool to preserve hashes of directory entries while obscuring the real
names.


## Future use cases

The natural use case is still to provide checksumming for data and metadata
blocks. With strong hashes, the same checksums can be used to aid deduplication
or verification (HMAC instead of plain hash). Due to different requirements,
one hash algorithm cannot possibly satisfy all requirements, namely speed vs.
strength. Or can it?


## The criteria

The frequency of checksumming is high, every data block needs that, every
metadata block needs that.

During the discussions in the community, there were several candidate hash
algorithms proposed and as it goes users want different things but we
developers want to keep the number of features sane or at least maintainable. I
think and hope the solution will address that.

The first hash type is to replace crc32c with focus on **speed** and not
necessarily strength (ie. collision resistance).

The second type focus is **strength**, in the cryptographic sense.

In addition to the technical aspects of the hashes, there are some requirements
that would allow free distribution and use of the implementations:

* implementation available under a GPL2 compatible license
* available in the linux kernel, either as a library function or as a module
* license that allows use in external tools like bootloaders (namely grub2)

Constraints posed by btrfs implementation:

* maximum digest width is 32 bytes
* blocks of size from 4KiB up to 64KiB

For the enhanced use case of data verification (using HMAC), there's a
requirement that might not interest everybody but still covers a lot of
deployments. And this is standard compliance and certification:

* standardized by FIPS

And maybe last but not least, use something that is in wide use already, proven by time.

### Speed

Implementation of all algorithms should be performant on common hardware, ie.
64bit architectures and hopefully not terrible on 32bit architectures or older
and weaker hardware.  By hardware I mean the CPU, not specialized hardware
cards.

The crypto API provided by linux kernel can automatically select the best
implementation of a given algorithm, eg. optimized implementation in assembly
and on multiple architectures.

### Strength

For the fast hash the collisions could be generated but hopefully not that
easily as for crc32c. For strong hash it's obvious that finding a collision
would be jackpot.

In case of the fast hash the quality can be evaluated using the SMHasher suite.


## The contenders

The following list of hashes has been mentioned and considered or evaluated:

* xxhash
* XXH3
* SipHash
* CRC64
* SHA256
* SHA3
* BLAKE2

### xxhash

*criteria*: license OK, implementation OK, digest size OK, not standardized but
in wide use

The hash is quite fast as it tries to exploit the CPU features that allow
instruction parallelism. The SMHasher score is 10, that's great. The linux kernel
implementation landed in 5.3.

Candidate for *fast hash*.

### XXH3

Unfortunately the hash is not yet finalized and cannot be in the final round,
but for evaluation of speed it was considered. The hash comes from the same
author as xxhash.

### SipHash

This hash is made for hash tables and hashing short strings but we want 4KiB or
larger blocks. Not a candidate.

### CRC64

Similar to crc32 and among the contenders only because it was easy to evaluate
but otherwise is not in the final round. It has shown to be slow in the
microbenchmark.

### SHA256

*criteria*: license OK, implementation OK, digest size OK, standardized in FIPS

The SHA family of hashes is well known, has decent support in CPU and is
standardized. Specifically, SHA256 is the strongest that still fits into the
available 32 bytes.

Candidate for *strong hash*.

### SHA3

*criteria*: license OK, implementation OK, digest size OK, standardized in FIPS

Winner of the 2012 hash contest, we can't leave it out. From the practical
perspective of checksum, the speed is bad even for the strong hash type. One of
the criteria stated above is performance without special hardware, unlike what
was preferred during the SHA3 contest.

Candidate for *strong hash*.

### BLAKE2

*criteria*: license OK, implementation OK, digest size OK, not standardized

From the family of BLAKE that participated in the 2012 SHA contest, the '2'
provides a trade-off speed vs. strength. More and more projects adopt it.

Candidate for *strong hash*.


## Final round

I don't blame you if you skipped all the previous paragraphs. The (re)search
for the next hash was quite informative and fun so it would be shame not to
share it, also to document the selection process for some transparency. This is
a committee driven process though.

* fast hash: **xxhash**
* strong hash: **BLAKE2** and **SHA256**

Two hashes selected for the strong type is a compromise to get a
fast-but-strong hash yet also something that's standardized.

The specific version of BLAKE2 is going to be the 'BLAKE2b-256' variant, ie.
optimized for 64bit (2b) but with 256bit digest.


### Microbenchmark

A microbenchmark gives more details about performance of the hashes:

Block: 4KiB (4096 bytes), 
Iterations: 100000

| Hash | Total cycles | Cycles/iteration
---: | ---: | ---: |
|         NULL-NOP |    56888626 |    568
|      NULL-MEMCPY |    60644484 |    606
|            CRC64 |  3240483902 |  32404
|           CRC32C |   174338871 |   1743
|        CRC32C-SW |   174388920 |   1743
|           XXHASH |   251802871 |   2518
|             XXH3 |   193287384 |   1932
| BLAKE2b-256-arch |  1798517566 |  17985
|      BLAKE2b-256 |  2358400785 |  23584
|     BLAKE2s-arch |  2593112451 |  25931
|          BLAKE2s |  3451879891 |  34518
|           SHA256 | 10674261873 | 106742
|         SHA3-256 | 29152193318 | 291521

Machine: Intel(R) Xeon(R) CPU E5-1620 v3 @ 3.50GHz (AVX2)

Hash implementations are the reference ones in C:

* NULL-NOP -- stub to measure overhead of the framework
* NULL-MEMCPY -- simple memcpy of the input buffer
* CRC64 -- linux kernel lib/crc64.c
* CRC32C -- hw assisted crc32c, (linux)
* CRC32C-SW -- software implementation, table-based (linux)
* XXHASH -- reference implementation
* XXH3 -- reference implementation
* BLAKE2b-256-arch -- 64bit optimized reference version (for SSE2/SSSE3/SSE41/AVX/AVX2)
* BLAKE2b-256 -- 64bit reference implementation
* BLAKE2s-arch -- 32bit optimized reference version
* BLAKE2s -- 32bit reference implementation
* SHA256 -- RFC 6234 reference implementation
* SHA3-256 -- C, based on canonical implementation

There aren't optimized versions for all hashes so for fair comparison the
unoptimized reference implementation should be used. As BLAKE2 is my personal
favourite I added the other variants and optimized versions to observe the
relative improvements.


### Evaluation

CRC64 was added by mere curiosity how does it compare to the rest. Well,
curiosity satisfied.

SHA3 is indeed slow on a CPU.


### What isn't here

There are non-cryptographic hashes like CityHash, FarmHash, Murmur3 and more,
that were found unsuitable or not meeting some of the basic criteria.  Others
like FNV or Fletcher used in ZFS are of comparable strength of crc32c, so that
won't be a progress.


## Merging

All the preparatory work in btrfs landed in version 5.3. Hardcoded assumptions
of crc32c were abstracted, linux crypto API wired in, with additional cleanups
or refactoring. With that in place, adding a new has is a matter of a few lines
of code adding the specifier string for crypto API.

The work on btrfs-progs is following the same path.

Right now, the version 5.4 is in development but new features can't be added,
so the target for the new hashes is **5.5**. The BLAKE2 algorithm family still
needs to be submitted and merged, hopefully they'll make it to 5.5 as well.

One of my merge process requirements was to do a call for public testing, so
we're going to do that once all the kernel and progs code is ready for testing.
Stay tuned.
