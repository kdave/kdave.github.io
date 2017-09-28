---
layout: page
category: b
tags: [ btrfs, kernel ]
feed: pko
date: 2019-12-11
status: finished
title: Btrfs hilights in 5.3
---

A bit more detailed overview of a btrfs update that I find interesting, see the
[pull request](https://git.kernel.org/linus/a18f8775419d3df282dd83efdb51c5a64d092f31)
for the rest.

## CRC32C uses accelerated versions on other architectures

... than just Intel-based ones. There was a hard coded check for the intel SSE
feature providing the accelerated instruction, but this totally skipped other
architectures. A brief check in `linux.git/arch/*/crypto` for files
implementing the accelerated versions revealed that there's a bunch of them:
ARM, ARM64, MIPS, PowerPC, s390 and SPARC. I don't have enough hardware to show
the improvements, though.

## Automatically remove incompat bit for RAID5/6

While this is not the fix everybody is waiting on, it's demonstrating
user-developer-user cycle how things can be improved. A filesystem created
with RAID5 or -6 profiles sets an incompatibility bit. That's supposed to be
there as long as there's any chunk using the profiles. User expectation is that
the bit should be removed once the chunks are eg. balanced away. This is what
got implemented and serves as a prior example for any future feature that
might get removed on a given filesystem. (Note from the future: I did that for the
RAID1C34 as well).

Side note: for the chunk profile it's easy because the runtime check of
presence is quick and requires only going through a list of chunk profiles
types. Example of the worst case would be dropping the incompat bit for LZO
after there are no compressed files using that algorithm. You can easily see
that this would require either enumerating all extent metadata (one time check)
or keeping track of count since mkfs time. This is IMHO not a typical request
and can be eventually done using the `btrfstune` tool on an unmounted
filesystem.
