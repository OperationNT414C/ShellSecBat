# ShellSecBat

Henkaku plugin that shows seconds and battery percent in status bar.

This is a fusion between "ShellBat" and "LastSeconds" plugins.
It also solves "12 hours" format issue when those plugins were manually combined.

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellSecBat.png?raw=true)

Configurated ShellSecBat allow to add the date to the status bar (in addition to the time with seconds and the battery percent).

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellDateSecBat.png?raw=true)

You can also visualize your drive state (total size and remaining space) by maintaining SELECT button and L or R trigger.

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellDriveState.png?raw=true)

On a PlayStation TV, the useless battery percent is hidden (even if the configuration tries to enable it). All other features (seconds, date and drive state) are still available.

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellPSTVDisplay.png?raw=true)


### Installation

Add the plugin under `*main` section in `ur0:tai/config.txt` or `ux0:tai/config.txt`:

```
*main
ux0:tai/shellsecbat.suprx
```

Please remove any other plugin which impacts the status bar display (like "ShellBat", "ShellDateSecBat" or "LastSeconds").


### Configuration

ShellSecBat can be configurated through a "ShellSecBat.txt" file. The file size should not exceed 64 bytes and its content should look like:

```
Features:11
Time:110/
```

This configuration means:

```
Features:[Drives display enabled][Battery display enabled]
Time:[Seconds display enabled][Date display enabled][Year display disabled][Date separation character]
```

You should be aware that this configuration file reading is very basic (and it could easily fail if there is even some slightest mistake).
Keywords `Feature:` and `Time:` are used to find parts of configuration, there must the configurations parameters (1 to enable, 0 to disable) just after the `:` character (without any blank character between).

 * Invalid configuration: `Features: 11`
 * Valid configuration: `Features:11`

Only the following paths are supported for the configuration file (they will be check in the same order):

 * `ux0:/data/ShellSecBat.txt`
 * `ux0:/tai/ShellSecBat.txt`
 * `ur0:/tai/ShellSecBat.txt`
 * `ur0:/plugins/ShellSecBat.txt`



### Credits

 * nowrep for "ShellBat" (https://github.com/nowrep/vita-shellbat)
 * theorywrong for "LastSeconds" (https://github.com/theorywrong/LastSeconds)
