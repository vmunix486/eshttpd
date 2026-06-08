# eshttpd
Extremely Small httpd, from ELKS

# Features

 - Very small code size (305 lines, 266 SLOC)
 - Can serve files 
 - Very Optional (Can turn off dynamic time checking lol)
 - More TBA

# Goal

The goal of this project is to have a small HTTP server within a single file of C. Some extra brownie points is for it to be very portable, being able to run on old hardware and software, on lots of CPU architectures other than x86, and differing libcs and kernels, such as BSD's, Linux, Windows, MacOS, RTOS's, etc. etc. Something that would make this easier is if this depends on as little things as possible, so it can run on the most constrained of libc's. Another thing that would be really nice is for it to be very readable, with almost everything having comments, so people can learn things about C89, and coding in general.

Something that I'd like is to be very modular, so you can tune it to your needs, or your software limitations. One example of this is with `<time.h>`, in which, setting a dynamic time is actually optional if your libc doesn't have a `<time.h>`. This will be extended to other things as well, such as SSL/TLS, cgi-bin, HTTPS, and newer HTTP versions (like 2.0, which will improve performance on the client).

# Why ELKS httpd

The reason I chose to use the httpd version from ELKS is because ELKS is a very simple operating system, and requires things that are written for it to also be simple. Because of this simplicity, it makes programs written for ELKS very portable to other platforms. Another reason I chose ELKS httpd is because it is the bare minimum httpd required, making it perfect for extending it to something better.

# Contributing

See CONTRIBUTING.md
