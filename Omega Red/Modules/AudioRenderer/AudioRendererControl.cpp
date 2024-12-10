
#include "PrecompiledHeader.h"
#include "src\spu2.h"
#include "AudioRenderer.h"
#include "PCSX2Lib_API.h"

// DSP
bool dspPluginEnabled = false;
int dspPluginModule = 0;
wchar_t dspPlugin[256];


extern void CALLBACK SPU2setDMABaseAddr(uptr baseaddr);

void CALLBACK s_SPU2readDMA4Mem(uint16 *pMem, int32 size)
{
    SPU2readDMA4Mem(pMem, (u32)size);
}

void CALLBACK s_SPU2writeDMA4Mem(uint16 *pMem, int32 size)
{
    SPU2writeDMA4Mem(pMem, (u32)size);
}


void CALLBACK s_SPU2readDMA7Mem(uint16 *pMem, int32 size)
{
    SPU2readDMA7Mem(pMem, (u32)size);
}

void CALLBACK s_SPU2writeDMA7Mem(uint16 *pMem, int32 size)
{
    SPU2writeDMA7Mem(pMem, (u32)size);
}


void CALLBACK s_SPU2irqCallback(void(*SPU2callback)(), void(*DMA4callback)(), void(*DMA7callback)())
{
	g_SPU2callback = SPU2callback;

	g_DMA4callback = DMA4callback;

	g_DMA7callback = DMA7callback;
}


SPU2writeCallback g_SPU2writeCallback;

SPU2readCallback g_SPU2readCallback;

SPU2TimeUpdateCallback g_SPU2TimeUpdateCallback;

void CALLBACK SPU2finishDMACallback(void(*SPU2writeCallback)(int32 counter), void(*SPU2readCallback)(int32 counter))
{
	g_SPU2writeCallback = SPU2writeCallback;

	g_SPU2readCallback = SPU2readCallback;
}

void CALLBACK s_SPU2TimeUpdateCallback(void(*SPU2writeCallback)(int32 counter))
{
	g_SPU2TimeUpdateCallback = SPU2writeCallback;
}


PCSX2Lib::API::SPU2_API g_API = {
	SPU2reset,
	SPU2write,
	SPU2read,
    s_SPU2readDMA4Mem,
    s_SPU2writeDMA4Mem,
	SPU2interruptDMA4,
    s_SPU2readDMA7Mem,
    s_SPU2writeDMA7Mem,
    SPU2setDMABaseAddr,
	SPU2interruptDMA7,
	SPU2ReadMemAddr,
	SPU2WriteMemAddr,
	s_SPU2irqCallback,
	SPU2setClockPtr,
	SPU2async,
	SPU2finishDMACallback,
	s_SPU2TimeUpdateCallback
};