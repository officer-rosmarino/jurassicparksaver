# Jurassic Park ScreenSaver

I was really surprised that I wasn't able to find a Jurassic
Park look-a-like screensaver out there.
My first thought was of course "this is not possible", and maybe I just searched
bad. But whatever, if it already exists, I hope you enjoy this version too.

More information on how to write a screensaver module can be found [here](http://www.dis.uniroma1.it/~liberato/screensaver/)

**NOTE: this module has only been tested on my laptop**

![screenshot](https://raw.githubusercontent.com/toohottohoot/jurassicparksaver/master/screensaver.png)

## Installation

Find the directory where `libX11.so` is by running:
```
find /usr -name libX11.so
```

Compile by running:
```
gcc -pthread -o jurassicpark jurassicpark-saver/jurassicpark.c -Ldirectory -lX11 -lXpm
```
where `directory` is the directory containing the `libX11.so`.

Place the compiled program into a directory in your PATH.
Add the module to the xscreensaver configuration file. This is done by creating a file .xscreensaver in your home directory. For a single module, this contains a line like:
```
programs: simplesquares
```

More information about how to install the module can be found [here](http://www.dis.uniroma1.it/~liberato/screensaver/install.html)

## Donate
If you like this project, consider buying me a beer (if we meet, because I still need to set this up) :)

Cheers!
