# eshttpd
Extremely Small httpd, from ELKS

# Features

 - Very small code size
 - Can serve files 
 - More TBA

# Goal

The goal of this project is to have a small HTTP server within a single file of C. Some extra brownie points is for it to be very portable, being able to run on old hardware and software, on lots of CPU architectures other than x86, and differing libcs and kernels, such as BSD's, Linux, Windows, MacOS, RTOS's, etc. etc. Something that would make this easier is if this depends on as little things as possible, so it can run on the most constrained of libc's. Another thing that would be really nice is for it to be very readable, with almost everything having comments, so people can learn things about C89, and coding in general.

# Why ELKS httpd

The reason I chose to use the httpd version from ELKS is because ELKS is a very simple operating system, and requires things that are written for it to also be simple. Because of this simplicity, it makes programs written for ELKS very portable to other platforms. Another reason I chose ELKS httpd is because it is the bare minimum httpd required, making it perfect for extending it to something better.

# Contributing

See CONTRIBUTING.md
