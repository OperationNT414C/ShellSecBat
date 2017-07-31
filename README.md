# ShellSecBat and ShellDateSecBat

Henkaku plugin that shows seconds and battery percent in status bar.

This is a fusion between "ShellBat" and "LastSeconds" plugins.
It also solves "12 hours" format issue when those plugins were manually combined.

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellSecBat.png?raw=true)

ShellDateSecBat adds the date to the status bar (in addition to the time with seconds and the battery percent).

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellDateSecBat.png?raw=true)


### Installation

Add one of those plugins under `*main` section in `ux0:tai/config.txt`:

```
*main
ux0:tai/shellsecbat.suprx
```

OR:

```
*main
ux0:tai/shelldatesecbat.suprx
```

DO NOT add both!

Please remove any other plugin which impacts the status bar display (like "ShellBat" or "LastSeconds").


### Credits

 * nowrep for "ShellBat" (https://github.com/nowrep/vita-shellbat)
 * theorywrong for "LastSeconds" (https://github.com/theorywrong/LastSeconds)
