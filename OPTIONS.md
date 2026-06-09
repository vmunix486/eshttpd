# Options

Now, one of the goals of eshttpd is to be very modular, and as to not forcefully introduce new features that could cut off support on other platforms, so you can turn off certain libraries or functions in your libc doesn't have in order to target as wide of hardware and software as possible. How can a single file be modular, though? Well, with lots of `#ifdef`s, `#else`s, and `#endif`s. Using this, you can make a single file modular. If you are wondering how you can add a modular feature yourself, then look in `eshttpd.c` and look at how time setting gets handled. Anyways, here's the options you can set.

| Command | Description |
| --- | --- |
| `-D_TIME` | Enable time setting (requires `time.h`) |
| `-D_BSD` | Fixes some warnings that are caused by redefinitions on BSD's |
