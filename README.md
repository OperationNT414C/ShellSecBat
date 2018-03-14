# ShellSecBat

Henkaku plugin that status bar displayed content customization.

This is a fusion between "ShellBat" and "LastSeconds" plugins.
It also solves "12 hours" format issue when those plugins were manually combined.

![Screenshot](https://github.com/OperationNT414C/ShellSecBat/blob/master/doc/ShellSecBat.png?raw=true)

Configurated ShellSecBat allows to add the date to the status bar (in addition to the time with seconds and the battery percent).

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

ShellSecBat can be configurated through a "ShellSecBat.txt" file. The file size should not exceed 116 bytes and its content should look like:

```
Features:11
Time:100/
Drives:11.11111
LeftKey:101
RightKey:201
```

This configuration sample matches the official default ShellSecBat settings (when there is no found configuration file).

Only the following paths are supported for the configuration file (they will be check in this same order):

 * `ux0:/data/ShellSecBat.txt`
 * `ux0:/tai/ShellSecBat.txt`
 * `ur0:/tai/ShellSecBat.txt`
 * `ur0:/plugins/ShellSecBat.txt`

The file must follows the following rules:

```
Features:[Drives display][Battery display]
Time:[Seconds display][Date display][Year display][Date separator]
Drives:[Skip unmounted][Free space display][Space decimal separator][imc0:][ur0:][ux0:][uma0:][grw0:]
LeftKey:[Keys combination for previous drive display]
RightKey:[Keys combination for next drive display]
```

You should be aware that configuration file reading is very basic (and it could easily fail if there is even the slightest mistake).

Keywords `Feature:`, `Time:`, `Drives:`, `LeftKey:` and `RightKey:`  are used to find configuration parts.
Once a configuration part is found, configuration parameters must directly follow the `:` character (without any blank character between):

 * Valid configuration part: `Features:01`
 * Invalid configuration part: `Features: 01`

For parameters which can be activated (all parameters except keys and separators), `1` means that it is enabled and any other character means that it is disabled.

`LeftKey:` and `RightKey:` must be followed by an hexadecimal value which describes the key combination. Hexadecimal flags for each key can be found here (on "SceCtrlButtons" section):

https://github.com/vitasdk/vita-headers/blob/master/include/psp2/ctrl.h

Additional rules are applied:
 * Keywords can appear in any order in the configuration but parameters must always exactly match what the keyword expects
 * `Drives:`, `LeftKey:` and `RightKey:` won't be parsed if "[Drives display]" is disabled
 * On PlayStation TV, "[Battery display]" is ignored as it will always be disabled

You can simulate ShellSecBat and ShellDateSecBat previous versions (prior to configuration file support) with the following configurations:

 * ShellSecBat V4 behavior: `Features:11 Time:100/ Drives:01.01110 LeftKey:101 RightKey:201`
 * ShellDateSecBat V4 behavior: `Features:11 Time:110/ Drives:01.01110 LeftKey:101 RightKey:201`



### Credits

 * nowrep for "ShellBat" (https://github.com/nowrep/vita-shellbat)
 * theorywrong for "LastSeconds" (https://github.com/theorywrong/LastSeconds)
