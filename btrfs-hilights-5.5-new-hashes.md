---
layout: page
category: b
tags: [ btrfs, kernel, blake2, xxhash, sha256 ]
date: 2020-02-10
status: finished
title: "Btrfs hilights in 5.5: new hashes"
---

A bit more detailed overview of a btrfs update that I find interesting, see the
[pull request](https://git.kernel.org/linus/97d0bf96a0d0986f466c3ff59f2ace801e33dc69)
for the rest.

## More checksum algorithms

The first request for more checksums is several years old (2014), [it was
SHA256](https://lore.kernel.org/linux-btrfs/1416806586-18050-1-git-send-email-bo.li.liu@oracle.com).
There was no followup of the patch but the discussion in the mail thread
brought the topic into light. Questions about why SHA256 and why not something
else (welcome to the zoo: SpookyHash, BLAKE2, CityHash, MurmurHash3, CRC64),
preferences, use cases etc. Browsing the mail thread again, I don't see anybody
suggest SHA1 or MD5.  There were some open questions and suggestions and though
it's an old discussion, most of the points were valid at the time of pre-5.5
evaluation and served as a checklist.  This is what I really like about the
community projects and btrfs users in particular.  Getting useful feedback on
topics out of scope of the filesystem itself.

Fast forward to now, there are 3 new hashes:

| Name    | Digest size | Notes
| :---    |        ---: | :---
| CRC32C  |       32bit | fast, weak, default, CPU support, backward compatible
| XXHASH  |       64bit | fast, weak, good on modern CPUs
| SHA256  |      256bit | slow, strong, FIPS certified, CPU support
| BLAKE2B |      256bit | slow, strong, good on 64bit architectures

You can find more about the [selection process]({{ site.url }}/selecting-hash-for-btrfs/)
in another post.

## Quick start

```
# mkfs.btrfs --csum xxhash /dev/sda
```

And the output:

```
btrfs-progs v5.4.1
See http://btrfs.wiki.kernel.org for more information.

Label:              (null)
UUID:               9774b466-fa5d-4c2e-8b4f-479d0bb5873d
Node size:          16384
Sector size:        4096
Filesystem size:    10.00GiB
Block group profiles:
  Data:             single            8.00MiB
  Metadata:         DUP             256.00MiB
  System:           DUP               8.00MiB
SSD detected:       no
Incompat features:  extref, skinny-metadata
Checksum:           xxhash64
Number of devices:  1
Devices:
   ID        SIZE  PATH
    1    10.00GiB  /dev/sda
```

For convenience, the hash names can be specified by aliases so both *xxhash*
and *xxhash64* are accepted on command line, similarly for *blake2* it's
*blake2b*. This is based on a usability pattern, where humans don't have to
remember the exact specification and copy & paste from the output works as
well.

### Use cases and outlook

For ordinary checksumming, all the hashes are considered good enough. CRC32C is
the weakest but it's been with us since the beginning and has caught many
random bit flips.

XXHASH is supposed to add to the strength with a good performance. Increased
digest size reduces collisions, but can't compete with the crypto-strong
hashes. That can still be of great help for deduplication.

From my perspective, the number of hashes should stay limited as the support is
needed not just in kernel but for other tooling. For *btrfs-progs* it's
mandatory to support them, but also bootloaders (\*cough\*in case they verify
checksums\*cough\*). The kernel support requires the hash implementations and
the modules become a dependency of *btrfs.ko*.  This can increase overall size
and build time.

One of the suggestions was to allow users to choose the hash out of all what
linux crypto API offers, not just the fixed set of hashes selected by
developers. I understand that from the point of view of users who can make the
decisions about trade-offs for their use case this is better, but I argue that
such users are not in majority. Satisfying the common case was the preferred
option, hopefully providing good options also for the knowledgeable users.
Providing and maintaining implementations of seven and one hashes would be
a burden for several projects.

Yes, I'm aware of [BLAKE3](https://github.com/BLAKE3-team/BLAKE3), but let's
[wait before adding new hashes]({{ site.url }}/blake3-vs-blake2-in-btrfs). By
the time we do another round there might be more contenders or improved
versions.  There's [XXH3](https://fastcompression.blogspot.com/2019/03/presenting-xxh3.html)
from the author of XXHASH, producing 128bit digest and is slightly faster.
However it is still not recommended for production use due to fine tuning and
finalization.

The cryptographic hashes can be a building block for a keyed hash, ie.
checksums that can't be subverted. There's a
[patch implementing that](https://lore.kernel.org/linux-btrfs/20191015121405.19066-1-jthumshirn@suse.de/),
currently in an RFC state. While the code is straightforward, adding a new
interface could use a feedback round. Please contact us in case you have
comments.

Last but not least, strong hashes are a building block for deduplication. At
the moment btrfs supports only the out-of-band way, on demand by calling
an ioctl.  There are tools utilizing that, eg. [BEES](https://github.com/Zygo/bees)
or [duperemove](https://github.com/markfasheh/duperemove),
calculating the block checksums by themselves. Exporting the raw checksums
would help to decrease CPU time and also external tracking of the checksums.

### Implementation notes

With the single checksum CRC32C, lots of code had hard coded size (4 bytes/32
bits) or functions and identifiers contained 'crc32c'. This could not remain
once there are more hashes to be added, so a round of cleanups happened in
advance in 5.4.  I'd call it great preparatory work that paid off, because
adding new hashes was a matter of inserting new definition entries to a table.
The rest was behind abstraction of btrfs functions that call to the linux
crypto API.

In parallel to the btrfs patches, the *BLAKE2B* linux implementation was merged,
while *XXHASH* was merged in 5.3 and SHA256 was there forever.
All in all, everything was ready in the same target release.
