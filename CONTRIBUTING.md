# Contributing

Contributing to eshttpd is very simple, you just have to fork the repository, make the changes you want, then ask to merge.

The code that you contribute <u>must</u> be C89/ANSI compliant, and also be as portable as possible. Always aim for portability over speed. Another thing that can help is making the code smaller. Something about code size is that things like comments, newlines, and tabs are ignored by compilers, so you can put the entire bible in a comment in the file, and the executable will be the same size as one without the bible.<sup>(a)</sup>

AI can be used, but only for code, everything else has to be written by a person, because AI has no feelings. It is a math equation.

Code comments don't have to be strictly formal, they can be written however which way you want. Something to remember is that uou can be a comedian, but you still need to get your point across good enough so that others can understand the code. Signing comments is allowed.<sup>(b)</sup>

<sup>(a)</sup> Please do not do that :sob: :wilted-rose:

<sup>(b)</sup> eg. 
```c
/* This bit of code is where the server running eshttpd sends over the requested file over to the client. -vmunix */
```
