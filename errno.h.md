---
layout: page
category: b
tags: [ errno, debugging ]
date: 2017-11-23
update1: 2018-01-02
status: finished
title: errno.h for the masses
---

The `errno` values, in various formats you may encounter. The standard errno is
a positive number, linux kernel uses the negative values. Besides the sources
(going through like 5 include hops from `/usr/include/errno.h` to
`/usr/include/asm-generic/errno.h` and `/usr/include/asm-generic/errno-base.h`)
there's no direct way to get the symbolic <-> numeric mapping. The hexa values
commonly appear in the stack trace dumps.

| symbolic | dec | hex | neg hex | comment |
--- | ---: | ---: | ---: | --- |
| EPERM                  | 1 | 0x01 | 0x..ffff | Operation not permitted |
| ENOENT                 | 2 | 0x02 | 0x..fffe | No such file or directory |
| ESRCH                  | 3 | 0x03 | 0x..fffd | No such process |
| EINTR                  | 4 | 0x04 | 0x..fffc | Interrupted system call |
| EIO                    | 5 | 0x05 | 0x..fffb | I/O error |
| ENXIO                  | 6 | 0x06 | 0x..fffa | No such device or address |
| E2BIG                  | 7 | 0x07 | 0x..fff9 | Argument list too long |
| ENOEXEC                | 8 | 0x08 | 0x..fff8 | Exec format error |
| EBADF                  | 9 | 0x09 | 0x..fff7 | Bad file number |
| ECHILD                 | 10 | 0x0a | 0x..fff6 | No child processes |
| EAGAIN                 | 11 | 0x0b | 0x..fff5 | Try again |
| ENOMEM                 | 12 | 0x0c | 0x..fff4 | Out of memory |
| EACCES                 | 13 | 0x0d | 0x..fff3 | Permission denied |
| EFAULT                 | 14 | 0x0e | 0x..fff2 | Bad address |
| ENOTBLK                | 15 | 0x0f | 0x..fff1 | Block device required |
| EBUSY                  | 16 | 0x10 | 0x..fff0 | Device or resource busy |
| EEXIST                 | 17 | 0x11 | 0x..ffef | File exists |
| EXDEV                  | 18 | 0x12 | 0x..ffee | Cross-device link |
| ENODEV                 | 19 | 0x13 | 0x..ffed | No such device |
| ENOTDIR                | 20 | 0x14 | 0x..ffec | Not a directory |
| EISDIR                 | 21 | 0x15 | 0x..ffeb | Is a directory |
| EINVAL                 | 22 | 0x16 | 0x..ffea | Invalid argument |
| ENFILE                 | 23 | 0x17 | 0x..ffe9 | File table overflow |
| EMFILE                 | 24 | 0x18 | 0x..ffe8 | Too many open files |
| ENOTTY                 | 25 | 0x19 | 0x..ffe7 | Not a typewriter |
| ETXTBSY                | 26 | 0x1a | 0x..ffe6 | Text file busy |
| EFBIG                  | 27 | 0x1b | 0x..ffe5 | File too large |
| ENOSPC                 | 28 | 0x1c | 0x..ffe4 | No space left on device |
| ESPIPE                 | 29 | 0x1d | 0x..ffe3 | Illegal seek |
| EROFS                  | 30 | 0x1e | 0x..ffe2 | Read-only file system |
| EMLINK                 | 31 | 0x1f | 0x..ffe1 | Too many links |
| EPIPE                  | 32 | 0x20 | 0x..ffe0 | Broken pipe |
| EDOM                   | 33 | 0x21 | 0x..ffdf | Math argument out of domain of func |
| ERANGE                 | 34 | 0x22 | 0x..ffde | Math result not representable |
| EDEADLK                | 35 | 0x23 | 0x..ffdd | Resource deadlock would occur |
| ENAMETOOLONG           | 36 | 0x24 | 0x..ffdc | File name too long |
| ENOLCK                 | 37 | 0x25 | 0x..ffdb | No record locks available |
| ENOSYS                 | 38 | 0x26 | 0x..ffda | Function not implemented |
| ENOTEMPTY              | 39 | 0x27 | 0x..ffd9 | Directory not empty |
| ELOOP                  | 40 | 0x28 | 0x..ffd8 | Too many symbolic links encountered |
| EWOULDBLOCK            | EAGAIN | | | |
| ENOMSG                 | 42 | 0x2a | 0x..ffd6 | No message of desired type |
| EIDRM                  | 43 | 0x2b | 0x..ffd5 | Identifier removed |
| ECHRNG                 | 44 | 0x2c | 0x..ffd4 | Channel number out of range |
| EL2NSYNC               | 45 | 0x2d | 0x..ffd3 | Level 2 not synchronized |
| EL3HLT                 | 46 | 0x2e | 0x..ffd2 | Level 3 halted |
| EL3RST                 | 47 | 0x2f | 0x..ffd1 | Level 3 reset |
| ELNRNG                 | 48 | 0x30 | 0x..ffd0 | Link number out of range |
| EUNATCH                | 49 | 0x31 | 0x..ffcf | Protocol driver not attached |
| ENOCSI                 | 50 | 0x32 | 0x..ffce | No CSI structure available |
| EL2HLT                 | 51 | 0x33 | 0x..ffcd | Level 2 halted |
| EBADE                  | 52 | 0x34 | 0x..ffcc | Invalid exchange |
| EBADR                  | 53 | 0x35 | 0x..ffcb | Invalid request descriptor |
| EXFULL                 | 54 | 0x36 | 0x..ffca | Exchange full |
| ENOANO                 | 55 | 0x37 | 0x..ffc9 | No anode |
| EBADRQC                | 56 | 0x38 | 0x..ffc8 | Invalid request code |
| EBADSLT                | 57 | 0x39 | 0x..ffc7 | Invalid slot |
| EDEADLOCK              | EDEADLK | | | |
| EBFONT                 | 59 | 0x3b | 0x..ffc5 | Bad font file format |
| ENOSTR                 | 60 | 0x3c | 0x..ffc4 | Device not a stream |
| ENODATA                | 61 | 0x3d | 0x..ffc3 | No data available |
| ETIME                  | 62 | 0x3e | 0x..ffc2 | Timer expired |
| ENOSR                  | 63 | 0x3f | 0x..ffc1 | Out of streams resources |
| ENONET                 | 64 | 0x40 | 0x..ffc0 | Machine is not on the network |
| ENOPKG                 | 65 | 0x41 | 0x..ffbf | Package not installed |
| EREMOTE                | 66 | 0x42 | 0x..ffbe | Object is remote |
| ENOLINK                | 67 | 0x43 | 0x..ffbd | Link has been severed |
| EADV                   | 68 | 0x44 | 0x..ffbc | Advertise error |
| ESRMNT                 | 69 | 0x45 | 0x..ffbb | Srmount error |
| ECOMM                  | 70 | 0x46 | 0x..ffba | Communication error on send |
| EPROTO                 | 71 | 0x47 | 0x..ffb9 | Protocol error |
| EMULTIHOP              | 72 | 0x48 | 0x..ffb8 | Multihop attempted |
| EDOTDOT                | 73 | 0x49 | 0x..ffb7 | RFS specific error |
| EBADMSG                | 74 | 0x4a | 0x..ffb6 | Not a data message |
| EOVERFLOW              | 75 | 0x4b | 0x..ffb5 | Value too large for defined data type |
| ENOTUNIQ               | 76 | 0x4c | 0x..ffb4 | Name not unique on network |
| EBADFD                 | 77 | 0x4d | 0x..ffb3 | File descriptor in bad state |
| EREMCHG                | 78 | 0x4e | 0x..ffb2 | Remote address changed |
| ELIBACC                | 79 | 0x4f | 0x..ffb1 | Can not access a needed shared library |
| ELIBBAD                | 80 | 0x50 | 0x..ffb0 | Accessing a corrupted shared library |
| ELIBSCN                | 81 | 0x51 | 0x..ffaf | .lib section in a.out corrupted |
| ELIBMAX                | 82 | 0x52 | 0x..ffae | Attempting to link in too many shared libraries |
| ELIBEXEC               | 83 | 0x53 | 0x..ffad | Cannot exec a shared library directly |
| EILSEQ                 | 84 | 0x54 | 0x..ffac | Illegal byte sequence |
| ERESTART               | 85 | 0x55 | 0x..ffab | Interrupted system call should be restarted |
| ESTRPIPE               | 86 | 0x56 | 0x..ffaa | Streams pipe error |
| EUSERS                 | 87 | 0x57 | 0x..ffa9 | Too many users |
| ENOTSOCK               | 88 | 0x58 | 0x..ffa8 | Socket operation on non-socket |
| EDESTADDRREQ           | 89 | 0x59 | 0x..ffa7 | Destination address required |
| EMSGSIZE               | 90 | 0x5a | 0x..ffa6 | Message too long |
| EPROTOTYPE             | 91 | 0x5b | 0x..ffa5 | Protocol wrong type for socket |
| ENOPROTOOPT            | 92 | 0x5c | 0x..ffa4 | Protocol not available |
| EPROTONOSUPPORT        | 93 | 0x5d | 0x..ffa3 | Protocol not supported |
| ESOCKTNOSUPPORT        | 94 | 0x5e | 0x..ffa2 | Socket type not supported |
| EOPNOTSUPP             | 95 | 0x5f | 0x..ffa1 | Operation not supported on transport endpoint |
| EPFNOSUPPORT           | 96 | 0x60 | 0x..ffa0 | Protocol family not supported |
| EAFNOSUPPORT           | 97 | 0x61 | 0x..ff9f | Address family not supported by protocol |
| EADDRINUSE             | 98 | 0x62 | 0x..ff9e | Address already in use |
| EADDRNOTAVAIL          | 99 | 0x63 | 0x..ff9d | Cannot assign requested address |
| ENETDOWN               | 100 | 0x64 | 0x..ff9c | Network is down |
| ENETUNREACH            | 101 | 0x65 | 0x..ff9b | Network is unreachable |
| ENETRESET              | 102 | 0x66 | 0x..ff9a | Network dropped connection because of reset |
| ECONNABORTED           | 103 | 0x67 | 0x..ff99 | Software caused connection abort |
| ECONNRESET             | 104 | 0x68 | 0x..ff98 | Connection reset by peer |
| ENOBUFS                | 105 | 0x69 | 0x..ff97 | No buffer space available |
| EISCONN                | 106 | 0x6a | 0x..ff96 | Transport endpoint is already connected |
| ENOTCONN               | 107 | 0x6b | 0x..ff95 | Transport endpoint is not connected |
| ESHUTDOWN              | 108 | 0x6c | 0x..ff94 | Cannot send after transport endpoint shutdown |
| ETOOMANYREFS           | 109 | 0x6d | 0x..ff93 | Too many references: cannot splice |
| ETIMEDOUT              | 110 | 0x6e | 0x..ff92 | Connection timed out |
| ECONNREFUSED           | 111 | 0x6f | 0x..ff91 | Connection refused |
| EHOSTDOWN              | 112 | 0x70 | 0x..ff90 | Host is down |
| EHOSTUNREACH           | 113 | 0x71 | 0x..ff8f | No route to host |
| EALREADY               | 114 | 0x72 | 0x..ff8e | Operation already in progress |
| EINPROGRESS            | 115 | 0x73 | 0x..ff8d | Operation now in progress |
| ESTALE                 | 116 | 0x74 | 0x..ff8c | Stale NFS file handle |
| EUCLEAN                | 117 | 0x75 | 0x..ff8b | Structure needs cleaning |
| ENOTNAM                | 118 | 0x76 | 0x..ff8a | Not a XENIX named type file |
| ENAVAIL                | 119 | 0x77 | 0x..ff89 | No XENIX semaphores available |
| EISNAM                 | 120 | 0x78 | 0x..ff88 | Is a named type file |
| EREMOTEIO              | 121 | 0x79 | 0x..ff87 | Remote I/O error |
| EDQUOT                 | 122 | 0x7a | 0x..ff86 | Quota exceeded |
| ENOMEDIUM              | 123 | 0x7b | 0x..ff85 | No medium found |
| EMEDIUMTYPE            | 124 | 0x7c | 0x..ff84 | Wrong medium type |
| ECANCELED              | 125 | 0x7d | 0x..ff83 | Operation Canceled |
| ENOKEY                 | 126 | 0x7e | 0x..ff82 | Required key not available |
| EKEYEXPIRED            | 127 | 0x7f | 0x..ff81 | Key has expired |
| EKEYREVOKED            | 128 | 0x80 | 0x..ff80 | Key has been revoked |
| EKEYREJECTED           | 129 | 0x81 | 0x..ff7f | Key was rejected by service |
| EOWNERDEAD             | 130 | 0x82 | 0x..ff7e | Owner died |
| ENOTRECOVERABLE        | 131 | 0x83 | 0x..ff7d | State not recoverable |
| ERFKILL                | 132 | 0x84 | 0x..ff7c | Operation not possible due to RF-kill |
| EHWPOISON              | 133 | 0x85 | 0x..ff7b | Memory page has hardware error |
| ERESTARTSYS            | 512 | 0x200 | 0x..ff200 |  |
| ERESTARTNOINTR         | 513 | 0x201 | 0x..ff1ff |  |
| ERESTARTNOHAND         | 514 | 0x202 | 0x..ff1fe | restart if no handler.. |
| ENOIOCTLCMD            | 515 | 0x203 | 0x..ff1fd | No ioctl command |
| ERESTART_RESTARTBLOCK  | 516 | 0x204 | 0x..ff1fc | restart by calling sys_restart_syscall |
| EBADHANDLE             | 521 | 0x209 | 0x..ff1f7 | Illegal NFS file handle |
| ENOTSYNC               | 522 | 0x20a | 0x..ff1f6 | Update synchronization mismatch |
| EBADCOOKIE             | 523 | 0x20b | 0x..ff1f5 | Cookie is stale |
| ENOTSUPP               | 524 | 0x20c | 0x..ff1f4 | Operation is not supported |
| ETOOSMALL              | 525 | 0x20d | 0x..ff1f3 | Buffer or request is too small |
| ESERVERFAULT           | 526 | 0x20e | 0x..ff1f2 | An untranslatable error occurred |
| EBADTYPE               | 527 | 0x20f | 0x..ff1f1 | Type not supported by server |
| EJUKEBOX               | 528 | 0x210 | 0x..ff1f0 | Request initiated, but will not complete before timeout |
| EIOCBQUEUED            | 529 | 0x211 | 0x..ff1ef | iocb queued, will get completion event |
| EIOCBRETRY             | 530 | 0x212 | 0x..ff1ee | iocb queued, will trigger a retry |
