---
layout: page
category: b
tags: [ cuda, nvidia, resume, linux ]
date: 2022-01-12
status: finished
title: CUDA not initialized after resume (unknown error)
---

`tl;dr` *CUDA returning an **unknown error** upon initialization after resume on
linux can be solved by reloading kernel module nvidia_uvm.*

There are some problems reported where with CUDA returns an unknown error upon
initialization, while there's no clear cause nor a clear way how to fix it.
I've found
[https://victorwyee.com/engineering/failed-call-to-cuinit-cuda-error-unknown/](https://victorwyee.com/engineering/failed-call-to-cuinit-cuda-error-unknown/)
listing various solutions from rebooting to tweaking environment variables.

I've hit another problem that does not seem to be explicitly mentioned as the
cause or it could have been misdiagnosed. After switching the built in Intel
graphic card to be primary and NVIDIA card only for computations, it was not
possible to run any application using CUDA. All it reported back were *unknown
error*s.

The Leela chess `lc0` reported CUDA version 0.0.0 with message like:

       $ ./lc0
	      _
       |   _ | |
       |_ |_ |_| v0.28.2 built Dec 13 2021
       ucinewgame
       Found pb network file: ./192x15-2021_1212_2050_25_709.pb.gz
       Creating backend [cuda-auto]...
       Switching to [cuda]...
       CUDA Runtime version: 0.0.0
       WARNING: CUDA Runtime version mismatch, was compiled with version 11.4.0
       Latest version of CUDA supported by the driver: 11.4.0
       error CUDA error: unknown error (../../src/neural/cuda/network_cuda.cc:134)

The line in source file is simply reporting errors:

       134     ReportCUDAErrors(cudaGetDeviceCount(&total_gpus));

This is the first place where the errors are checked, a quick trivial CUDA
program fails on the first memory allocation, with the same unkown error.

I guess this stresses out the importance of error checking, namely when some
piece of hardware is involved. Though this fails on the first attempt to use the
API, there are possibly more cases during a long computation and many tasks using
the GPU in parallel.

A reboot fixed the problem, until the next suspend, which is impractical and
setting up the work environment gets annoying. I have tried to look for some
utility/command that is supposed to be run after resume. There's a script called
`nvidia-sleep.sh` that is the callback script for suspend and resume. Running
that with parameter `suspend` manually while the system was up was not a good
idea as it switched me out of the graphical desktop to some virtual terminal
without any way to undo that. Also running that with `resume` did not help
eiter (and does not even make sense after the first proper resume).

There are no obvious signs that something could be wrong after resume, nothing
in the logs and running games utilizing acceleration works (judging by FPS).
One advice recommends to run `nvidia-modprobe`, in case some of the 4 modules is
not loaded (nvidia\_uvm, nvidia\_drm, nvidia\_modeset, nvidia). Besides the UVM
module, all are unused.

Right after resume the modules look like this:

     nvidia_drm             69632  2
     nvidia_modeset       1204224  3 nvidia_drm
     nvidia_uvm           2531328  0
     nvidia              35373056  90 nvidia_uvm,nvidia_modeset

That there's 0 (usage by other modules) brought some suspicion, because
according to `nvidia-smi`, there's a least Xorg keeping a few megabytes of
allocated memory (which may or may not be related though).

I've reloaded the nvidia\_uvm module, because it was the only thing possible to
do with the modules:

     # modprobe -r nvidia_uvm
     # modprobe nvidia_uvm

And it started to work. Then quick check, lc0, works again. Now the module list is:

     nvidia_uvm           2531328  2
     nvidia_drm             69632  2
     nvidia_modeset       1204224  3 nvidia_drm
     nvidia              35373056  204 nvidia_uvm,nvidia_modeset

While this is still a workaround, it's much better than rebooting and a
post-resume hook should be able to make it work automatically.

The suspend script is in `/usr/lib/systemd/system-sleep/`:

     #!/bin/sh
     case "$1" in
	 post)
		     sleep 4
		     modprobe -r nvidia_uvm
		     sleep 1
		     modprobe nvidia_uvm
	     ;;
     esac
