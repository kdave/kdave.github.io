---
layout: page
category: b
tags: [ qemu ]
feed: pko
date: 2018-06-08
status: finished
title: qemu 2.12 does not support if=scsi
---

After update of qemu to version 2.12, my testing vms stopped to just warn
about the `if=scsi` (with a bit more cryptic message), and did not want to
start.

```
qemu-system-x86_64: -drive file=root,if=scsi,media=disk,cache=none,index=0,format=raw: machine type does not support if=scsi,bus=0,unit=0
```

The `if=scsi` shortcut was handy, the maze of qemu command line options may
take some time to comprehend but then you can do wonders.

The [release notes of version 2.12](https://wiki.qemu.org/ChangeLog/2.12#Deprecated_options_and_features) do
mention that the support is gone and that other options should be used instead,
also mentions which ones. But it does not tell how exactly. As this should be a
simple syntax transformation from one option to another, an example would save
a lot of time.

I was not able to find any ready-to-use example in a few minutes so had to
experiment a bit (this saved me documentation reading time).

The setup I use is a file-backed root image for a virtual machine, nothing
really fancy. The file name is `root`, sparse file, raw ie. no qcow2, no
caching.

Here you are:

```
-device virtio-scsi-pci,id=scsi
-drive file=root,id=root-img,if=none,format=raw,cache=none
-device scsi-hd,drive=root-img
```

First you need to define the bus, otherwise the 3rd line will complain that
there's no SCSI. Second line is to define the file backed drive and the third
one puts that together.

Using SCSI might not be the best idea for a qemu VM as the emulated driver is
buggy and crashes, so I'd recommend to use virtio, but for a almost read-only
root image it's fine. Also the device names are predictable.
