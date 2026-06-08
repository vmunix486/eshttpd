# TODO

 - Add support for Windows (would love for support for 95/NT 3.1)
 - Test support for MacOS (PPC, x86, cannot test ARM)
 - Test support for BSD's (FreeBSD, NetBSD, OpenBSD, 386BSD)
 - Test support for RTOS's (FreeRTOS, Zephyr, etc.)
 - Test support for UNIX (SCO OpenServer, Sun/Oracle Solaris, Intel UNIX)
 - Test support for Raspberry Pi Pico (2?)
 - Test support on other architectures (ARM, PPC, RISC-V)
 - Add more code comments
 - Add <b><u>optional</u></b> support for SSL/TLS (probably with something like WolfSSL or OpenSSL)
 - Add support for `cgi-bin`, eg. Lua, Sqlite, PHP
 - Add support for embedding CGI scripts into HTML files
 - Add support for other files, eg. SVG, PBM, Javascript, Java, etc. etc.
 - Add support for compiling with MSVC++ 2008 (for Windows shipment)
 - Have option to use `fork()` (looking at you, uclinux rofl)
 - Fix compilation notes and warnings
 - Add support for flags, such as verbose mode, help, running the program in the foreground, and changing the HTTP port.
 - Fix runtime errors on Android (Termux)
 - Switch `sprintf()` to `snprintf()` (right now, when sprintf is switched over to snprintf, it causes the client to only get plain text)
