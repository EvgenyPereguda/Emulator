//GiGaHeRz's SPU2 Driver
//Copyright (c) 2003-2008, David Quintana <gigaherz@gmail.com>
//
//This library is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
//

#include "..\src\Global.h"
#include "..\src\spu2.h"

#ifdef _MSC_VER
#include "Windows.h"
#endif

FILE *s2rfile;

void s2r_write16(s16 data)
{
    fwrite(&data, 2, 1, s2rfile);
}

void s2r_write32(u32 data)
{
    fwrite(&data, 4, 1, s2rfile);
}

static void EMITC(u32 i, u32 a)
{
    s2r_write32(((i & 0x7u) << 29u) | (a & 0x1FFFFFFFu));
}

int s2r_open(u32 ticks, char *filename)
{
    //s2rfile = fopen(filename, "wb");
    //if (s2rfile)
    //    s2r_write32(ticks);
    return s2rfile ? 0 : -1;
}

void s2r_readreg(u32 ticks, u32 addr)
{
    if (!s2rfile)
        return;
    s2r_write32(ticks);
    EMITC(0, addr);
}

void s2r_writereg(u32 ticks, u32 addr, s16 value)
{
    if (!s2rfile)
        return;
    s2r_write32(ticks);
    EMITC(1, addr);
    s2r_write16(value);
}

void s2r_writedma4(u32 ticks, u16 *data, u32 len)
{
    u32 i;
    if (!s2rfile)
        return;
    s2r_write32(ticks);
    EMITC(2, len);
    for (i = 0; i < len; i++, data++)
        s2r_write16(*data);
}

void s2r_writedma7(u32 ticks, u16 *data, u32 len)
{
    u32 i;
    if (!s2rfile)
        return;
    s2r_write32(ticks);
    EMITC(3, len);
    for (i = 0; i < len; i++, data++)
        s2r_write16(*data);
}

void s2r_close()
{
    if (!s2rfile)
        return;
    fclose(s2rfile);
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// replay code

bool replay_mode = false;

u16 dmabuffer[0xFFFFF];

const u32 IOP_CLK = 768 * 48000;
const u32 IOPCiclesPerMS = 768 * 48;
u32 CurrentIOPCycle = 0;

u64 HighResFreq;
u64 HighResPrev;
double HighResScale;

bool Running = false;

#ifdef _MSC_VER

int conprintf(const char *fmt, ...)
{
    return 0;
    //#ifdef _WIN32
//    char s[1024];
//    va_list list;
//
//    va_start(list, fmt);
//    vsprintf(s, fmt, list);
//    va_end(list);
//
//    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
//    if (handle == INVALID_HANDLE_VALUE)
//        return 0;
//
//    DWORD written = 0;
//    WriteConsoleA(handle, s, strlen(s), &written, 0);
//    FlushFileBuffers(handle);
//
//    return written;
//#else
//    va_list list;
//    va_start(list, fmt);
//    int ret = vsprintf(stderr, fmt, list);
//    va_end(list);
//    return ret;
//#endif
}

void dummy1()
{
}

void dummy4()
{
    SPU2interruptDMA4();
}

void dummy7()
{
    SPU2interruptDMA7();
}

u64 HighResFrequency()
{
    u64 freq;
#ifdef _WIN32
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
#else
// TODO
#endif
    return freq;
}

u64 HighResCounter()
{
    u64 time;
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER *)&time);
#else
// TODO
#endif
    return time;
}

void InitWaitSync() // not extremely accurate but enough.
{
    HighResFreq = HighResFrequency();
    HighResPrev = HighResCounter();
    HighResScale = (double)HighResFreq / (double)IOP_CLK;
}

u32 WaitSync(u32 TargetCycle)
{
    u32 WaitCycles = (TargetCycle - CurrentIOPCycle);
    u32 WaitTime = WaitCycles / IOPCiclesPerMS;
    if (WaitTime > 10)
        WaitTime = 10;
    if (WaitTime == 0)
        WaitTime = 1;
    SleepEx(WaitTime, TRUE);

    // Refresh current time after sleeping
    u64 Current = HighResCounter();
    u32 delta = (u32)floor((Current - HighResPrev) / HighResScale + 0.5); // We lose some precision here, cycles might drift away over long periods of time ;P

    // Calculate time delta
    CurrentIOPCycle += delta;
    HighResPrev += (u64)floor(delta * HighResScale + 0.5); // Trying to compensate drifting mentioned above, not necessarily useful.

    return delta;
}

#ifdef _WIN32
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    Running = false;
    return TRUE;
}
#endif


#endif
