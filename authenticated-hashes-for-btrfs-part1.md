---
layout: page
category: b
tags: [ btrfs, kernel, blake2, xxhash, sha256 ]
feed: pko
date: 2021-05-24
status: finished
title: "Authenticated hashes for btrfs (part 1)"
---

There was a request to provide authenticated hashes in btrfs, natively as one of the btrfs checksum algorithms. Sounds fun but there's always more to it, even if this sounds easy to implement.

Johaness T. at that time in SUSE sent the patchset adding the support for SHA256 [[1]](https://lore.kernel.org/linux-fsdevel/20200428105859.4719-1-jth@kernel.org/) with a Labs conference paper, summarizing existing solutions and giving details about the proposed implementation and use cases.

The first version of the patchset posted got some feedback, issues were found and some ideas suggested. Things have stalled a bit, but the feature is still very interesting and really not hard to implement. The support for additional checksums has provided enough support code to just plug in the new algorithm and enhance the existing interfaces to provide the key bytes. So until now I've assumed you know what an authenticated hash means, but for clarity and in simple terms: a checksum that depends on a key. The main point is that it's impossible to generate the same checksum for given data without knowing the key, where *impossible* is used in the cryptographic-strength sense, there's an almost zero probability doing that by chance and brute force attack is not practical.

## Auth hash, fsverity

Notable existing solution for that is *fsverity* that works in read-only fashion, where the key is securely hidden and used only to verify that data that are read from media haven't been tampered with. A typical use case is an OS image in your phone. But that's not all. Images of OS appear in all sorts of boxed devices, IoT. Nowadays, with explosion of edge computing, assuring integrity of the end devices is a fundamental requirement.

Where btrfs can add some value is the read AND write support, with an authenticated hash. This brings questions around key handling, and not everybody is OK with a device that could potentially store malicious/invalid data with a proper authenticated checksum. So yeah, use something else, this is not your use case, or maybe there's another way how to make sure the key won't be compromised easily. This is beyond the scope of what filesystem can do, though.

As an example use case of writable filesystem with authenticated hash: detect outside tampering with on-disk data, eg. when the filesystem was unmounted. Filesystem metadata formats are public, interesting data can be located by patterns on the device, so changing a few bytes and updating the checksum(s) is not hard.

There's one issue that was brought up and I think it's not hard to observe anyway: there's a total dependency on the key to verify a basic integrity of the data. Ie. without the key it's not possible to say if the data are valid as if a basic checksum was used. This might be still useful for a read-only access to the filesystem, but absence of key makes this impossible.

## Existing implementations

As was noted in the LWN discussion [[2]](https://lwn.net/Articles/819143/), what ZFS does, there are two checksums. One is the authenticated and one is not. I point you to the comment stating that, as I was not able to navigate far enough in the ZFS code to verify the claim, but the idea is clear. It's said that the authenticated hash is eg. SHA512 and the plain hash is SHA256, split half/half in the bytes available for checksum. The way the hash is stored is a simple trim of the first 16 bytes of each checksum and store them consecutively. As both hashes are cryptographically strong, the first 16 bytes *should* provide enough strength despite the truncation. Where 16 bytes is 128 bits.

When I was thinking about that, I had a different idea how to do that. Not that copying the scheme would not work for btrfs, anything that the linux kernel crypto API provides is usable, the same is achievable. I'm not judging the decisions what hashes to use or how to do the split, it works and I don't see a problem in the strength. Where I see potential for an improvement is performance, without sacrificing strength *too much*. Trade-offs.

The CPU or software implementation of SHA256 is comparably slower to checksums with hardware aids (like CRC32C instructions) or hashes designed to perform well on CPUs. That was the topic of the previous round of new hashes, so we now compete against BLAKE2b and XXHASH. There are CPUs with native instructions to calculate SHA256 and the performance improvement is noticeable, orders of magnitude better. But the support is not as widespread as eg. for CRC32C. Anyway, there's always choice and hardware improves over time. The number of hashes may seem to explode but as long as it's manageable inside the filesystem, we take it. And a coffee please.

## Secondary hash

The checksum scheme proposed is to use a cryptographic hash and a non-cryptographic one. Given the current support for SHA256 and BLAKE2b, the cryptographic hash is given. There are two of them and that's fine. I'm not drawing an exact parallel with ZFS, the common point for the cryptographic hash is that there are limited options and the calculation is expensive by design. This is where the non-cryptographic hash can be debated. Also I want to call it *secondary* hash, with obvious meaning that it's not too important by default and comes second when the authenticated hash is available.

We have CRC32C and XXHASH to choose from. Note that there are already two hashes from the start so supporting both secondary hashes would double the number of final combinations. We've added XXHASH to enhance the checksum collision space from 32 bits to 64 bits. What I propose is to use just XXHASH as the secondary hash, resulting in two new hashes for the authenticated and secondary hash. I haven't found a good reason to also include CRC32C.

Another design point was where to do the split and truncation. As the XXHASH has fixed length, this could be defined as 192 bits for the cryptographic hash and 64 bits for full XXHASH.

Here we are, we could have authenticated SHA256 accompanied by XXHASH, or the same with BLAKE2b. The checksum split also splits the decision tree what to do when the checksum partially matches. For a single checksum it's a simple *yes/no* decision. The partial match is the interesting case:

* primary (key available) hash matches, secondary does not -- as the authenticated hash is hard to forge, it's trusted (even if it's not full length of the digest)
* primary (key available) does not match, secondary does not -- checksum mismatch for the same reason as above
* primary (key not available) does not match, secondary does -- this is the prime time for the secondary hash, the floor is yours

This leads to 4 outcomes of the checksum verification, compared to 2. A boolean type can simply represent the yes/no outcome but for two hashes it's not that easy. It depends on the context, though I think it still should be straightforward to decide what to do that in the code. Nevertheless, this has to be updated in all calls to checksum verification and has to reflect the key availability eg. in case where the data are auto-repaired during scrub or when there's a copy.

## Performance considerations

The performance comparison should be now clear: we have the potentially slow SHA256 but fast XXHASH, for each metadata and data block, vs slow SHA512 and slow SHA256. As I reckon it's possible to also select SHA256/SHA256 split in ZFS, but that can't beat SHA256/XXHASH.

The key availability seems to be the key point in all that, puns notwithstanding. The initial implementation assumed for simplicity to provide the raw key bytes to kernel and to the userspace utilities. This is maybe OK for a prototype but under any circumstances can't survive until a final release. There's key management wired deep into linux kernel, there's a library for the whole API and command line tools. We ought to use that. Pass the key by name, not the raw bytes.

Key management has it's own culprits and surprises (key owned vs possessed), but let's assume that there's a standardized way how to obtain the key bytes from the key name. In kernel its "READ\_USER\_KEY\_BYTES", in userspace it's either *keyctl_read* from *libkeyutils* or a raw syscall to *keyctl*. Problem solved, on the low-level. But, well, don't try that over *ssh*.

Accessing a btrfs image for various reasons (check, image, restore) now needs the key to verify data or even the key itself to perform modifications (check + repair). The command line interface has to be extended for all commands that interact with the filesystem offline, ie. the image and not the mounted filesystem.

This results to a global option, like `btrfs --auth-key 1234 ispect-internal dump-tree`, compared to `btrfs inspect-internal dump-tree --auth-key 1234`. This is not finalized, but a global option is now the preferred choice.

## Final words

I have a prototype, that does not work in all cases but at least passes `mkfs` and `mount`. The number of checksum verification cases got above what I was able to fix by the time of writing this. I think this has enough matter on itself so I'm pushing it out out as part 1. There are open questions regarding the command line interface and also a some kind of proof or discussion regarding attacks. Stay tuned.

References:

- [1] [https://lore.kernel.org/linux-fsdevel/20200428105859.4719-1-jth@kernel.org/](https://lore.kernel.org/linux-fsdevel/20200428105859.4719-1-jth@kernel.org/)
- [2] [https://lwn.net/Articles/819143/](https://lwn.net/Articles/819143/) LWN discussion under [Authenticated Btrfs (2020)](https://lwn.net/Articles/818842/) 
