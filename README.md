# Jurassic Park ScreenSaver

I was really surprised that I wasn't able to find a Jurassic
Park look-a-like screensaver out there.
My first thought was of course "this is not possible", and maybe I just searched
bad. But whatever, if it already exists, I hope you enjoy this version too.

More information on how to write a screensaver module can be found [here](http://www.dis.uniroma1.it/~liberato/screensaver/)

**NOTE: this module has only been tested on my laptop**

![screenshot](https://raw.githubusercontent.com/toohottohoot/jurassicparksaver/master/screensaver.png)

## Installation

Compile by make:
```
make
```

Place the compiled program `jurassicpark` into a directory in your PATH.
Add the module to the xscreensaver configuration file. This is done by creating a file `.xscreensaver` in your home directory. For a single module, this contains a line like:
```
programs: simplesquares
```

If you already have an `.xscreensaver` file, it will probably look something like this:

```
[...]
- GL: 				cubetwist -root				    \n\
- GL: 				discoball -root				    \n\
- GL: 				hexstrut -root				    \n\
- GL: 				splodesic -root				    \n\
- GL: 				jurassicpark -root		    \n\   <-- the line to add
[...]
```

More information about how to install the module can be found [here](http://www.dis.uniroma1.it/~liberato/screensaver/install.html)

## Donate
If you like this project, consider buying me a beer (if we meet, because I still need to set this up) :)

Cheers!
