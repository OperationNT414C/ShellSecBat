/*
 *  ShellSecBat plugin
 *  Copyright (c) 2017 OperationNT
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/rtc.h>
#include <psp2/power.h>
#include <psp2/io/fcntl.h>

#include <taihen.h>

#include <psp2/registrymgr.h>
#include <psp2/system_param.h>

#include <psp2/ctrl.h>
#include <psp2/io/devctl.h>

static char *devices[] = {
    "imc0:",
    "ur0:",
	"ux0:",
	"uma0:"
};
#define N_DEVICES (sizeof(devices) / sizeof(char **))


// Configuration format:
// Features:[Drives display][Battery display]
// Time:[Seconds display][Date display][Year display][Date separator]
// Drives:[Skip unmounted][imc0:][ur0:][ux0:][uma0:]
// File content example "Features:11 Time:100/ Drives:11111"

static char *configPathes[] = {
    "ux0:/data/ShellSecBat.txt",
    "ux0:/tai/ShellSecBat.txt",
    "ur0:/tai/ShellSecBat.txt",
    "ur0:/plugins/ShellSecBat.txt"
};
#define N_PATHES (sizeof(configPathes) / sizeof(char **))

static int displayDrives = 1;
static int displayBattery = 1;

static int displaySeconds = 1;
static int displayDate = 0;
static int displayYear = 0;
static char dateSeparator = '/';

static int skipUnmounted = 1;
static int displayedDrives[N_DEVICES] = {1,1,1,1};

static char* seekChar(char* iStr, char iChar)
{
    while (*iStr != iChar && *iStr != '\0')
        iStr++;
    return (*iStr != '\0') ? iStr : NULL;
}


static char *units[] = {
    "B ",
	"KB",
	"MB",
	"GB",
    "TB"
};
#define N_UNITS (sizeof(units) / sizeof(char **))

static int unitType(SceOff iBytes, int* oConv, int* oCent)
{
    SceOff convBytes = iBytes;
    SceOff divider = 1;
    int res = 0;
    while (convBytes > 1023 && res < N_UNITS-1)
    {
        divider *= 1024;
        convBytes /= 1024;
        res++;
    }
    *oConv = (int)convBytes;
    *oCent = (int)(iBytes%divider)/(divider/102);
    return res;
}

static int displayed_disk = N_DEVICES;

/*
offset 0x1844f0:
int status_draw_battery_patched(int a1, uint8_t a2);
*/

static SceUID g_hooks[2];

uint32_t text_addr, text_size, data_addr, data_size;

// Functions

/**
 * ScePafWidget_39B15B98
 *
 * @param widget  Widget ptr
 * @param size    Font size (22.0 - standard font size in statusbar)
 * @param unk0    Unknown, pass 1
 * @param pos     Start of part of text to set size
 * @param len     Length of part of text to set size from pos
 */
static int (*scePafWidgetSetFontSize)(void *widget, float size, int unk0, int pos, int len);


static void get_functions_retail()
{
    scePafWidgetSetFontSize = (void*) text_addr + 0x45ce80;
}

static void get_functions_retail_365()
{
    scePafWidgetSetFontSize = (void*) text_addr + 0x45d2c8;
}

static void get_functions_testkit()
{
    scePafWidgetSetFontSize = (void*) text_addr + 0x453038;
}

static void get_functions_devkit()
{
    scePafWidgetSetFontSize = (void*) text_addr + 0x44E5F8;
}


static int digit_len(int num)
{
    if (num < 10) {
        return 1;
    } else if (num < 100) {
        return 2;
    } else {
        return 3;
    }
}

typedef struct {
    int start;
    int length;
    float height;
} TextMod;

#define N_FONT_CHANGE 3
static TextMod fontChange[N_FONT_CHANGE] = { {-1, 2, 16.0} , {-1, 0, 20.0} , {-1, 1, 16.0} };

static int in_draw_time = 0;

static tai_hook_ref_t ref_hook0;
static int status_draw_time_patched(void *a1, int a2)
{
    in_draw_time = 1;
    int out = TAI_CONTINUE(int, ref_hook0, a1, a2);
    in_draw_time = 0;
    if (a1) {
        for (int i = 0; i < N_FONT_CHANGE; i++)
        {
            TextMod* curFont = &fontChange[i];
            if (curFont->start != -1)
                scePafWidgetSetFontSize(a1, curFont->height, 1, curFont->start, curFont->length);
        }
    }
    return out;
}

static tai_hook_ref_t ref_hook1;
static uint16_t **some_strdup_patched(uint16_t **a1, uint16_t *a2, int a2_size)
{
    if (in_draw_time)
    {
        char buff[32];
        int len;
        int i;

        if (displayDrives)
        {
            // Here, buttons state is checked every 1.001 seconds.
            SceCtrlData pad;
            sceCtrlPeekBufferPositive(0, &pad, 1);
            if ((pad.buttons & (SCE_CTRL_SELECT|SCE_CTRL_RTRIGGER)) == (SCE_CTRL_SELECT|SCE_CTRL_RTRIGGER))
                displayed_disk = (displayed_disk+1)%(N_DEVICES+1);
            if ((pad.buttons & (SCE_CTRL_SELECT|SCE_CTRL_LTRIGGER)) == (SCE_CTRL_SELECT|SCE_CTRL_LTRIGGER))
                displayed_disk = (displayed_disk+N_DEVICES)%(N_DEVICES+1);

            if (displayed_disk < N_DEVICES)
            {
                SceIoDevInfo info;
                int res = sceIoDevctl(devices[displayed_disk], 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));

                if (res >= 0)
                {
                    int freeConv = 0;
                    int freeDec = 0;
                    int freeUnit = unitType(info.free_size, &freeConv, &freeDec);
                    
                    int maxConv = 0;
                    int maxDec = 0;
                    int maxUnit = unitType(info.max_size, &maxConv, &maxDec);
                    
                    if (freeDec > 0)
                    {
                        len = sceClibSnprintf(buff, 32, "%s %d.%02d%s/%d.%02d%s", devices[displayed_disk],
                                    freeConv, freeDec, units[freeUnit],
                                    maxConv, maxDec, units[maxUnit]);
                    }
                    else
                    {
                        len = sceClibSnprintf(buff, 32, "%s %d%s/%d.%02d%s", devices[displayed_disk],
                                    freeConv, units[freeUnit],
                                    maxConv, maxDec, units[maxUnit]);
                    }

                    i = 6;
                    while (i < len && buff[i] != '/') i++;
                    fontChange[0].start = i-2;
                    fontChange[0].length = 2;

                    fontChange[2].start = len-2;
                    fontChange[2].length = 2;
                }
                else
                {
                    // The 2 spaces at string end avoid a font size issue.
                    len = sceClibSnprintf(buff, 32, "%s Not mounted  ", devices[displayed_disk]);

                    fontChange[0].start = -1;
                    fontChange[2].start = -1;
                }

                for (i = 0; i < len; ++i) {
                    a2[i] = buff[i];
                }
                a2[len] = 0;
                
                fontChange[1].start = 0;
                fontChange[1].length = 4;
                return TAI_CONTINUE(uint16_t**, ref_hook1, a1, a2, len);
            }
        }

        SceDateTime time_local;
        SceDateTime time_utc;
        sceRtcGetCurrentClock(&time_utc, 0);

        SceRtcTick tick;
        sceRtcGetTick(&time_utc, &tick);
        sceRtcConvertUtcToLocalTime(&tick, &tick);
        sceRtcSetTick(&time_local, &tick);

        if (displayDate)
        {
            int date_format = SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY;
            sceRegMgrGetKeyInt("/CONFIG/DATE", "date_format", &date_format);
            
            if (displayYear)
            {
                if (SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY == date_format)
                    len = sceClibSnprintf(buff, 32, "%02d%c%02d%c%04d  ", time_local.day, dateSeparator, time_local.month, dateSeparator, time_local.year);
                else if (SCE_SYSTEM_PARAM_DATE_FORMAT_YYYYMMDD == date_format)
                    len = sceClibSnprintf(buff, 32, "%04d%c%02d%c%02d  ", time_local.year, dateSeparator, time_local.month, dateSeparator, time_local.day);
                else
                    len = sceClibSnprintf(buff, 32, "%02d%c%02d%c%04d  ", time_local.month, dateSeparator, time_local.day, dateSeparator, time_local.year);
            }
            else
            {
                if (SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY == date_format)
                    len = sceClibSnprintf(buff, 32, "%02d%c%02d  ", time_local.day, dateSeparator, time_local.month);
                else
                    len = sceClibSnprintf(buff, 32, "%02d%c%02d  ", time_local.month, dateSeparator, time_local.day);
            }

            uint16_t tmp[16];
            for (i = 0; i < a2_size; ++i) {
                tmp[i] = a2[i];
            }
            for (i = 0; i < len; ++i) {
                a2[i] = buff[i];
            }
            for (i = 0; i < a2_size; ++i) {
                a2[len+i] = tmp[i];
            }
            a2_size += len;
        }

        int is_ampm = (a2[a2_size - 1] == 'M');
        if (displaySeconds)
        {
            len = sceClibSnprintf(buff, 32, ":%02d", time_local.second);
            if (is_ampm)
            {
                for (i = 0; i < 3; ++i) {
                    a2[a2_size + len - 3 + i] = a2[a2_size - 3 + i];
                }
                for (i = 0; i < len; ++i) {
                    a2[a2_size - 3 + i] = buff[i];
                }
            }
            else
            {
                for (i = 0; i < len; ++i) {
                    a2[a2_size + i] = buff[i];
                }
            }
            a2_size += len;
        }

        if (displayBattery)
        {
            static int oldpercent = 0;
            int percent = scePowerGetBatteryLifePercent();
            if (percent < 0 || percent > 100) {
                percent = oldpercent;
            }
            oldpercent = percent;

            len = sceClibSnprintf(buff, 32, "  %d%%", percent);
            for (i = 0; i < len; ++i) {
                a2[a2_size + i] = buff[i];
            }

            fontChange[0].start = is_ampm ? (a2_size - 2) : -1;
            
            fontChange[1].start = a2_size + 2;
            fontChange[1].length = digit_len(percent);
            
            fontChange[2].start = fontChange[1].start + fontChange[1].length;
            fontChange[2].length = 1;
            
            a2_size += len;
        }
        else
        {
            fontChange[0].start = is_ampm ? (a2_size - 2) : -1;
            fontChange[1].start = -1;
            fontChange[2].start = -1;
        }
        a2[a2_size] = 0;

        return TAI_CONTINUE(uint16_t**, ref_hook1, a1, a2, a2_size);
    }
    return TAI_CONTINUE(uint16_t**, ref_hook1, a1, a2, a2_size);
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args)
{
    tai_module_info_t info;
    info.size = sizeof(info);
    int ret = taiGetModuleInfo("SceShell", &info);
    if (ret < 0) {
        return SCE_KERNEL_START_FAILED;
    }

    // Modified from TheOfficialFloW/Adrenaline
    SceKernelModuleInfo mod_info;
    mod_info.size = sizeof(SceKernelModuleInfo);
    ret = sceKernelGetModuleInfo(info.modid, &mod_info);
    if (ret < 0) {
        return SCE_KERNEL_START_FAILED;
    }
    text_addr = (uint32_t) mod_info.segments[0].vaddr;
    text_size = (uint32_t) mod_info.segments[0].memsz;
    data_addr = (uint32_t) mod_info.segments[1].vaddr;
    data_size = (uint32_t) mod_info.segments[1].memsz;

    uint32_t offsets[2];

    switch (info.module_nid) {
    case 0x0552F692: // retail 3.60 SceShell
        offsets[0] = 0x183ea4;
        offsets[1] = 0x40e0b4;
        get_functions_retail();
        break;

    case 0x5549BF1F: // retail 3.65 SceShell
        offsets[0] = 0x183f6c;
        offsets[1] = 0x40e4fc;
        get_functions_retail_365();
        break;

    case 0xEAB89D5C: // testkit 3.60 SceShell
        offsets[0] = 0x17c2d8;
        offsets[1] = 0x404828;
        get_functions_testkit();
        break;

    case 0x6CB01295: // PDEL 3.60 SceShell
        offsets[0] = 0x17B8DC;
        offsets[1] = 0x400028;
        get_functions_devkit();
        break;

    default:
        return SCE_KERNEL_START_FAILED;
    }

    g_hooks[0] = taiHookFunctionOffset(&ref_hook0,
                                       info.modid,
                                       0,          // segidx
                                       offsets[0], // offset
                                       1,          // thumb
                                       status_draw_time_patched);
    g_hooks[1] = taiHookFunctionOffset(&ref_hook1,
                                       info.modid,
                                       0,          // segidx
                                       offsets[1], // offset
                                       1,          // thumb
                                       some_strdup_patched);

    // Setup configuration from file
    displayBattery = (sceKernelGetModel() != SCE_KERNEL_MODEL_VITATV);

    SceUID fd = -1;
    int i = 0;
    int fileSize = 0;
    while (i < N_PATHES && fd < 0)
    {
        fd = sceIoOpen(configPathes[i], SCE_O_RDONLY, 0666);
        if (fd >= 0)
        {
            fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);
            if (fileSize > 64)
            {
                sceIoClose(fd);
                fd = -1;
            }
        }
        i++;
    }

    if (fd >= 0)
    {
        char configStr[72];
        sceIoLseek(fd, 0, SCE_SEEK_SET);
        sceIoRead(fd, configStr, fileSize);
        sceIoClose(fd);
        sceClibMemset(&(configStr[fileSize]), '\0', 12);
        
        // Look for features
        char* seeked = configStr;
        if ((seeked = seekChar(seeked, 'F')) && (seeked = seekChar(seeked, ':')))
        {
            displayDrives = ('1' == seeked[1]);
            displayBattery = (displayBattery && '1' == seeked[2]);
        }
        
        seeked = configStr;
        if ((seeked = seekChar(seeked, 'T')) && (seeked = seekChar(seeked, ':')))
        {
            displaySeconds = ('1' == seeked[1]);
            displayDate = ('1' == seeked[2]);
            displayYear = ('1' == seeked[3]);
            dateSeparator = seeked[4];
        }

        seeked = configStr;
        if ((seeked = seekChar(seeked, 'D')) && (seeked = seekChar(seeked, ':')))
        {
            skipUnmounted = ('1' == seeked[1]);
            for (i = 0; i < N_DEVICES; i++)
                displayedDrives[i] = ('1' == seeked[2+i]);
        }
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    if (g_hooks[0] >= 0) taiHookRelease(g_hooks[0], ref_hook0);
    if (g_hooks[1] >= 0) taiHookRelease(g_hooks[1], ref_hook1);

    return SCE_KERNEL_STOP_SUCCESS;
}
