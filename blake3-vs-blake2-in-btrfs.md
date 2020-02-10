---
layout: page
category: b
tags: [ blake2, blake3, btrfs ]
feed: pko
date: 2020-01-21
status: finished
title: BLAKE3 vs BLAKE2 for BTRFS
---

Irony isn't it. The paint of BLAKE2 as BTRFS checksum algorithm hasn't dried
yet, 1-2 weeks to go but there's a successor to it. Faster, yet still supposed to
be strong. For a second or two I considered ripping out all the work and ... no
not really but I do admit the excitement.

Speed and strength are competing goals for a hash algorithm. The speed can be
evaluated by anyone, not so much for the strength. I am no cryptographer and
for that area rely on expertise and opinion of others. That BLAKE was a SHA3
finalist is a good indication, where BLAKE2 is it's successor, weakened but not
weak. BLAKE3 is yet another step trading off strength and speed.

Regarding BTRFS, BLAKE2 is going to be the faster of strong hashes for now (the
other one is SHA256). The argument I have for it now is proof of time. It's
been deployed in many projects (even crypto currencies!), there are optimized
implementations, various language ports.

The look ahead regarding more checksums is to revisit them in about 5 years.
Hopefully by that time there will be deployments, real workload performance
evaluations and overall user experience that will back future decisions.

Maybe there are going to be new strong yet fast hashes developed. During my
research I learned about Kangaroo 12 that's a reduced version of SHA3 (Keccak).
The hash is constructed in a different way, perhaps there might be a Kangaroo 2π
one day on par with BLAKE3. Or something else. Why not EDON-R, it's #1 in many
of the cr.yp.to/hash benchmarks? Another thing I learned during the research is
that hash algorithms are twelve in a dozen, IOW too many to choose from. That
Kangaroo 12 is internally of a different construction might be a point for
selecting it to have wider range of "building block types".

## Quick evaluation

For BTRFS I have a micro benchmark, repeatedly hashing a 4 KiB block and using
cycles per block as a metric.

* Block size: 4KiB
* Iterations: 10000000
* Digest size: 256 bits (32 bytes)

|           Hash | Total cycles | Cycles/iteration | Perf vs BLAKE3 | Perf vs BLAKE2b
|           :--- |         ---: |             ---: |           :--- | :---
| BLAKE3  (AVX2) | 111260245256 |            11126 | 1.0            | 0.876 (-13%)
| BLAKE2b (AVX2) | 127009487092 |            12700 | 1.141 (+14%)   | 1.0
| BLAKE2b (AVX)  | 166426785907 |            16642 | 1.496 (+50%)   | 1.310 (+31%)
| BLAKE2b (ref)  | 225053579540 |            22505 | 2.022 (+102%)  | 1.772 (+77%)

Right now there's only the reference Rust implementation and a derived C
implementation of BLAKE3, claimed not to be optimized but from my other
experience the compiler can do a good job optimizing programmers ideas away.
There's only one BLAKE3 entry with the AVX2 implementation, the best hardware
support my testing box provides. As I had the other results of BLAKE2 at hand,
they're in the table for comparison, but the most interesting pair are the AVX2
versions anyway.

The improvement is 13-14%. Not much ain't it, way less that the announced 4+x
faster than BLAKE2b. Well, it's always important to interpret results of a
benchmark with respect to the environment of measurement and the tested
parameters.

For BTRFS filesystem the block size is always going to be in kilobytes. I can't
find what was the size of the official benchmark results, the bench.rs script
iterates over various sizes, so I assume it's an average. Short input buffers
can skew the results as the setup/output overhead can be significant, while for
long buffers the compression phase is significant. I don't have explanation for
the difference and won't draw conclusions about BLAKE3 in general.

One thing that I dare to claim is that I can sleep well because upon the above
evaluation, BLAKE3 won't bring a notable improvement if used as a checksum
hash.

## References

* [new hash for BTRFS selection](https://kdave.github.io/selecting-hash-for-BTRFS), same testing box for the measurements
* https://github.com/BLAKE3-team/BLAKE3 -- top commit 02250a7b7c80ded, 2020-01-13, upstream version 0.1.1
* [DJB's hash menu](https://bench.cr.yp.to/primitives-hash.html) and [per-machine results](https://bench.cr.yp.to/results-hash.html) with numbers

## Personal addendum

During the evaluations now and in the past, I've found it convenient if there's
an offer of implementations in various languages. That eg. Keccak project pages
does not point me directly to a C implementation slightly annoyed me, but the
reference implementation in C++ was worse than BLAKE2 I did not take the next
step to compare the C version, wherever I would find it.

BLAKE3 is fresh and Rust seems to be the only thing that has been improved
since the initial release. A plain C implementation without any
warning-not-optimized labels would be good. I think that C versions will appear
eventually, besides that Rust is now the new language hotness, there are
projects not yet *"let's rewrite it in Rust"*. Please Bear with us.
