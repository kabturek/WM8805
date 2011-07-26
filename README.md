Arduino WM8805 library
======================

This is the library that i use to comunicate with WM8805 from arduino. The
basic operations are supported (powering the device, selecting input, changing
the clock)

Hardware
--------
WM8805 is a 3.3V device so if your Arduino runs on 5V you need a i2c level
switching IC (or 2 mosfets). You can use ADG3304 from Analog devices or MAX3378
from Maxim(thats the one i use). 
[Here](http://ics.nxp.com/support/documents/interface/pdf/an97055.pdf) is a good document about connecting 3.3v and 5v devices on one bus.

By default the WM8805 is configured to use an external 12mhz oscillator.

Using
-----
There are no examples *yet* - just add the library:

    #include <WM8805.h>

Initialize the object:

    WM8805 wm;

Look at the header file for constructor options (You have to provide some
PLL values from the datasheet if your crystal is different than 12mhz)

And in you _setup()_ function call

    wm.init()

_WM8805::init()_ only argument is the initially selected input


Changelog
---------
0.1 - first version


License
-------
LGPL like the official arduino libs
