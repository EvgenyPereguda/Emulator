
#include "PrecompiledHeader.h"
#include "../pcsx2/CDVD/CDVDaccess.h"
#include "Plugins.h"
#include "../Interface.h"
#include "../pcsx2/R3000A.h"
#include "../pcsx2/MemoryTypes.h"
#include "../pcsx2/IopDma.h"
#include "../pcsx2/IopCounters.h"



u32 FWread32(u32 addr)
{
	throw "Unimplemented";
}

void FWwrite32(u32 addr, u32 value)
{
	throw "Unimplemented";
}


_SPU2reset s_SPU2reset;
_SPU2write s_SPU2write;
_SPU2read s_SPU2read;
_SPU2async s_SPU2async;
_SPU2readDMA4Mem s_SPU2readDMA4Mem;
_SPU2writeDMA4Mem s_SPU2writeDMA4Mem;
_SPU2interruptDMA4 s_SPU2interruptDMA4;
_SPU2readDMA7Mem s_SPU2readDMA7Mem;
_SPU2writeDMA7Mem s_SPU2writeDMA7Mem;
_SPU2irqCallback s_SPU2irqCallback;
_SPU2setClockPtr s_SPU2setClockPtr;
_SPU2setDMABaseAddr s_SPU2setDMABaseAddr;
_SPU2interruptDMA7 s_SPU2interruptDMA7;
_SPU2WriteMemAddr s_SPU2WriteMemAddr;
_SPU2ReadMemAddr s_SPU2ReadMemAddr;
PCSX2Lib::API::_SPU2finishDMACallback s_SPU2finishDMACallback;
PCSX2Lib::API::_SPU2TimeUpdateCallback s_SPU2TimeUpdateCallback;

void SPU2async(u32 cycles)
{
	s_SPU2async(cycles);
}

u16 SPU2read(u32 rmem)
{
	return s_SPU2read(rmem);
}

void SPU2write(u32 rmem, u16 value)
{
	s_SPU2write(rmem, value);
}

u32 SPU2ReadMemAddr(int core)
{
	return s_SPU2ReadMemAddr(core);
}
void SPU2WriteMemAddr(int core, u32 value)
{
	s_SPU2WriteMemAddr(core, value);
}

void SPU2setDMABaseAddr(uptr baseaddr)
{
	s_SPU2setDMABaseAddr(baseaddr);
}

void SPU2readDMA4Mem(u16* pMem, u32 size) // size now in 16bit units
{
	s_SPU2readDMA4Mem(pMem, size);
}

void SPU2writeDMA4Mem(u16* pMem, u32 size) // size now in 16bit units
{
	s_SPU2writeDMA4Mem(pMem, size);
}

void SPU2interruptDMA4()
{
	s_SPU2interruptDMA4();
}

void SPU2interruptDMA7()
{
	s_SPU2interruptDMA7();
}

void SPU2readDMA7Mem(u16* pMem, u32 size)
{
	s_SPU2readDMA7Mem(pMem, size);
}

void SPU2writeDMA7Mem(u16* pMem, u32 size)
{
	s_SPU2writeDMA7Mem(pMem, size);
}

void SPU2finishDMAwrite(int32 counter)
{
	auto DMAICounter = counter;
	
	if (((psxCounters[6].sCycleT + psxCounters[6].CycleT) - psxRegs.cycle) > (u32)DMAICounter)
	{
		psxCounters[6].sCycleT = psxRegs.cycle;
		psxCounters[6].CycleT = DMAICounter;

		psxNextCounter -= (psxRegs.cycle - psxNextsCounter);
		psxNextsCounter = psxRegs.cycle;
		if (psxCounters[6].CycleT < psxNextCounter)
			psxNextCounter = psxCounters[6].CycleT;
	}
}

void SPU2finishDMAread(int32 counter)
{
	auto DMAICounter = counter;

	if (((psxCounters[6].sCycleT + psxCounters[6].CycleT) - psxRegs.cycle) > (u32)DMAICounter)
	{
		psxCounters[6].sCycleT = psxRegs.cycle;
		psxCounters[6].CycleT = DMAICounter;

		psxNextCounter -= (psxRegs.cycle - psxNextsCounter);
		psxNextsCounter = psxRegs.cycle;
		if (psxCounters[6].CycleT < psxNextCounter)
			psxNextCounter = psxCounters[6].CycleT;
	}
}

void SPU2TimeUpdateCallback(int32 counter)
{
	if (((psxCounters[6].sCycleT + psxCounters[6].CycleT) - psxRegs.cycle) > (u32)counter)
	{
		psxCounters[6].sCycleT = psxRegs.cycle;
		psxCounters[6].CycleT = counter;

		psxNextCounter -= (psxRegs.cycle - psxNextsCounter);
		psxNextsCounter = psxRegs.cycle;
		if (psxCounters[6].CycleT < psxNextCounter)
			psxNextCounter = psxCounters[6].CycleT;
	}
}

void SPU2finishDMACallback(void(*SPU2writeCallback)(int32 counter), void(*SPU2readCallback)(int32 counter))
{
	s_SPU2finishDMACallback(SPU2writeCallback, SPU2readCallback);
}

s32 SPU2reset()
{
	SPU2finishDMACallback(SPU2finishDMAwrite, SPU2finishDMAread);

	s_SPU2TimeUpdateCallback(SPU2TimeUpdateCallback);

	return s_SPU2reset();
}

s32 SPU2ps1reset()
{
	throw "Unimplemented";
}

void SPU2setClockPtr(u32* ptr)
{
	s_SPU2setClockPtr(ptr);
}


PCSX2_EXPORT void STDAPICALLTYPE setSPU2(PCSX2Lib::API::SPU2_API *a_API)
{
	if (a_API == nullptr)
		return;

	s_SPU2reset = a_API->SPU2_Reset;
	s_SPU2write = a_API->SPU2write;
	s_SPU2read = a_API->SPU2read;
	s_SPU2readDMA4Mem = a_API->SPU2readDMA4Mem;
	s_SPU2writeDMA4Mem = a_API->SPU2writeDMA4Mem;
	s_SPU2interruptDMA4 = a_API->SPU2interruptDMA4;
	s_SPU2readDMA7Mem = a_API->SPU2readDMA7Mem;
	s_SPU2writeDMA7Mem = a_API->SPU2writeDMA7Mem;
	s_SPU2setDMABaseAddr = a_API->SPU2setDMABaseAddr;
	s_SPU2interruptDMA7 = a_API->SPU2interruptDMA7;
	s_SPU2ReadMemAddr = a_API->SPU2ReadMemAddr;
	s_SPU2WriteMemAddr = a_API->SPU2WriteMemAddr;
	s_SPU2irqCallback = a_API->SPU2irqCallback;
	s_SPU2setClockPtr = a_API->SPU2setClockPtr;
	s_SPU2async = a_API->SPU2async;
	s_SPU2finishDMACallback = a_API->SPU2finishDMACallback;
	s_SPU2TimeUpdateCallback = a_API->SPU2TimeUpdateCallback;
}

PCSX2_EXPORT bool STDAPICALLTYPE openPlugin_SPU2Func()
{
#ifdef ENABLE_NEW_IOPDMA_SPU2
	SPU2irqCallback(spu2Irq);
#else
	s_SPU2irqCallback(spu2Irq, spu2DMA4Irq, spu2DMA7Irq);
	if (s_SPU2setDMABaseAddr != NULL)
		s_SPU2setDMABaseAddr((uptr)iopMem->Main);
#endif
	if (s_SPU2setClockPtr != NULL)
		s_SPU2setClockPtr(&psxRegs.cycle);
	return true;
}



_DEV9async s_DEV9async;
_DEV9readDMA8Mem s_DEV9readDMA8Mem;
_DEV9writeDMA8Mem s_DEV9writeDMA8Mem;
_DEV9read8 s_DEV9read8;
_DEV9read16 s_DEV9read16;
_DEV9read32 s_DEV9read32;
_DEV9write8 s_DEV9write8;
_DEV9write16 s_DEV9write16;
_DEV9write32 s_DEV9write32;
DEV9handler s_dev9Handler;
_DEV9open s_DEV9open;
_DEV9irqCallback s_DEV9irqCallback;
_DEV9irqHandler s_DEV9irqHandler;


u8 DEV9read8(u32 addr)
{
	return s_DEV9read8(addr);
}

u16 DEV9read16(u32 addr)
{
	return s_DEV9read16(addr);
}

u32 DEV9read32(u32 addr)
{
	return s_DEV9read32(addr);
}

void DEV9write8(u32 addr, u8 value)
{
	s_DEV9write8(addr, value);
}

void DEV9write16(u32 addr, u16 value)
{
	s_DEV9write16(addr, value);
}

void DEV9write32(u32 addr, u32 value)
{
	s_DEV9write32(addr, value);
}

void DEV9readDMA8Mem(u32* pMem, int size)
{
	s_DEV9readDMA8Mem(pMem, size);
}

void DEV9writeDMA8Mem(u32* pMem, int size)
{
	s_DEV9writeDMA8Mem(pMem, size);
}

void DEV9async(u32 cycles)
{
	s_DEV9async(cycles);
}

int DEV9irqHandler(void)
{
	throw "Unimplemented";
}

void _DEV9irq(int cause, int cycles)
{
	throw "Unimplemented";
}



PCSX2_EXPORT void STDAPICALLTYPE setDEV9(PCSX2Lib::API::DEV9_API *a_API)
{
	if (a_API == nullptr)
		return;

	s_DEV9read8 = a_API->DEV9read8;
	s_DEV9read16 = a_API->DEV9read16;
	s_DEV9read32 = a_API->DEV9read32;
	s_DEV9write8 = a_API->DEV9write8;
	s_DEV9write16 = a_API->DEV9write16;
	s_DEV9write32 = a_API->DEV9write32;
	s_DEV9readDMA8Mem = a_API->DEV9readDMA8Mem;
	s_DEV9writeDMA8Mem = a_API->DEV9writeDMA8Mem;
	s_DEV9irqCallback = a_API->DEV9irqCallback;
	s_DEV9irqHandler = a_API->DEV9irqHandler;
	s_DEV9async = a_API->DEV9async;
}


PCSX2_EXPORT void STDAPICALLTYPE openPlugin_DEV9Func()
{
	s_dev9Handler = nullptr;
	s_DEV9irqCallback(dev9Irq);
	s_dev9Handler = s_DEV9irqHandler();
}



_USBread8 s_USBread8;
_USBread16 s_USBread16;
_USBread32 s_USBread32;
_USBwrite8 s_USBwrite8;
_USBwrite16 s_USBwrite16;
_USBwrite32 s_USBwrite32;
_USBasync s_USBasync;
_USBirqCallback s_USBirqCallback;
_USBirqHandler s_USBirqHandler;
_USBsetRAM s_USBsetRAM;
USBhandler s_usbHandler;


u8 USBread8(u32 addr)
{
	return s_USBread8(addr);
}

u16 USBread16(u32 addr)
{
	return s_USBread16(addr);
}

u32 USBread32(u32 addr)
{
	return s_USBread32(addr);
}

void USBwrite8(u32 addr, u8 value)
{
	s_USBwrite8(addr, value);
}

void USBwrite16(u32 addr, u16 value)
{
	s_USBwrite16(addr, value);
}

void USBwrite32(u32 addr, u32 value)
{
	s_USBwrite32(addr, value);
}

void USBsetRAM(void* mem)
{
	s_USBsetRAM(mem);
}

void USBasync(u32 cycles)
{
	s_USBasync(cycles);
}



PCSX2_EXPORT void STDAPICALLTYPE setUSB(PCSX2Lib::API::USB_API *a_API)
{
	if (a_API == nullptr)
		return;

	s_USBread8 = a_API->USBread8;
	s_USBread16 = a_API->USBread16;
	s_USBread32 = a_API->USBread32;
	s_USBwrite8 = a_API->USBwrite8;
	s_USBwrite16 = a_API->USBwrite16;
	s_USBwrite32 = a_API->USBwrite32;
	s_USBasync = a_API->USBasync;

	s_USBirqCallback = a_API->USBirqCallback;
	s_USBirqHandler = a_API->USBirqHandler;
	s_USBsetRAM = a_API->USBsetRAM;
}

PCSX2_EXPORT void STDAPICALLTYPE openPlugin_USBFunc()
{
	s_usbHandler = NULL;

	if (s_USBirqCallback != nullptr)
		s_USBirqCallback(usbIrq);

	if (s_USBirqHandler != nullptr)
		s_usbHandler = s_USBirqHandler();
}