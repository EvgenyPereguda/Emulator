/*  PCSX2Lib - Kernel of PCSX2 PS2 Emulator for PCs
*
*  PCSX2Lib is free software: you can redistribute it and/or modify it under the terms
*  of the GNU Lesser General Public License as published by the Free Software Found-
*  ation, either version 3 of the License, or (at your option) any later version.
*
*  PCSX2Lib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE.  See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with PCSX2Lib.
*  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PrecompiledHeader.h"
#include "../pcsx2/CDVD/CDVDaccess.h"
#include "Plugins.h"
#include "System.h"
#include "../pcsx2/VU.h"
#include "../Interface.h"
#include "../../pcsx2/System.h"
#include "AppConfig.h"
#include "../pcsx2/IopBios.h"
#include "../pcsx2/System/SysThreads.h"
#include "../pcsx2/GS.h"
#include "../pcsx2/IopDma.h"
#include "../pcsx2/R3000A.h"
#include "../pcsx2/Counters.h"
#include "../pcsx2/Sif.h"
#include "../pcsx2/x86/newVif.h"
#include "../pcsx2/IPU/IPU.h"
#include "../pcsx2/IPU/IPU_Fifo.h"
#include "../pcsx2/IPU/mpeg2lib/Mpeg.h"

#include "wx/thread.h"

#include "MTVU.h" // for thread cancellation on shutdown

#include "../pcsx2/ps2/BiosTools.h"

#include "Extend.h"
#include "../pcsx2/Sio.h"
#include "../pcsx2/Elfheader.h"
#include "../pcsx2/Patch.h"

#define PUGIXML_WCHAR_MODE

#include "../../Common/PugiXML/pugixml.hpp"

__aligned16 SysCoreThread g_SysCoreThread;

SysCoreThread &GetCoreThread()
{
    return g_SysCoreThread;
}


CDVD_API CDVDapi_Disc;

CDVD_API CDVDapi_Iso;

// ----------------------------------------------------------------------------
// Yay, order of this array shouldn't be important. :)
//
const PluginInfo tbl_PluginInfo[] =
    {
        {"GS", PluginId_GS, PS2E_LT_GS, PS2E_GS_VERSION},
        //{"PAD", PluginId_PAD, PS2E_LT_PAD, PS2E_PAD_VERSION},
        //{"SPU2", PluginId_SPU2, PS2E_LT_SPU2, PS2E_SPU2_VERSION},
        //{ "CDVD", PluginId_CDVD, PS2E_LT_CDVD, PS2E_CDVD_VERSION },
        //{"USB", PluginId_USB, PS2E_LT_USB, PS2E_USB_VERSION},
        //{"FW", PluginId_FW, PS2E_LT_FW, PS2E_FW_VERSION},
        //{"DEV9", PluginId_DEV9, PS2E_LT_DEV9, PS2E_DEV9_VERSION},

        {NULL},

        // See PluginEnums_t for details on the MemoryCard plugin hack.
        {"Mcd", PluginId_Mcd, 0, 0},
};


_GSopen            GSopen;
_GSopen2           GSopen2;

_GScallbackopen GScallbackopen;
_GSvsync GSvsync;
_GSgifTransfer GSgifTransfer;
_GSirqCallback GSirqCallback;
_GSsetBaseMem GSsetBaseMem;
_GSsetGameCRC GSsetGameCRC;
_GSsetFrameSkip GSsetFrameSkip;
_GSsetVsync GSsetVsync;
_GSreset GSreset;
_GSinitReadFIFO GSinitReadFIFO;
_GSreadFIFO GSreadFIFO;
_GSinitReadFIFO2 GSinitReadFIFO2;
_GSreadFIFO2 GSreadFIFO2;
_GSgifSoftReset GSgifSoftReset;









_FWread32 FWread32;
_FWwrite32 FWwrite32;
_FWirqCallback FWirqCallback;



_PADstartPoll PADstartPoll;
_PADpoll PADpoll;
_PADsetSlot PADsetSlot;



PCSX2_EXPORT void STDAPICALLTYPE setFW(PCSX2Lib::API::FW_API *a_API)
{
    if (a_API == nullptr)
        return;

    FWread32 = a_API->FWread32;
    FWwrite32 = a_API->FWwrite32;
    FWirqCallback = a_API->FWirqCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE setPAD(PCSX2Lib::API::PAD_API *a_API)
{
    if (a_API == nullptr)
        return;

    PADstartPoll = a_API->PADstartPoll;
    PADpoll = a_API->PADpoll;
    PADsetSlot = a_API->PADsetSlot;
}

PCSX2Lib::API::_GSgifTransfer API_GSgifTransfer = NULL;

void CALLBACK s_GSgifTransfer(const u32 *pMem, u32 size)
{
	if (API_GSgifTransfer == NULL)
		return;

	API_GSgifTransfer((uint8*)(pMem), size);
}


PCSX2Lib::API::_GSinitReadFIFO API_GSinitReadFIFO = NULL;

void CALLBACK s_GSinitReadFIFO(u64 *pMem)
{
	if (API_GSinitReadFIFO == NULL)
		return;

	API_GSinitReadFIFO((uint8*)(pMem));
}

PCSX2Lib::API::_GSreadFIFO API_GSreadFIFO = NULL;

void CALLBACK s_GSreadFIFO(u64 *pMem)
{
	if (API_GSreadFIFO == NULL)
		return;

	API_GSreadFIFO((uint8*)(pMem));
}

PCSX2Lib::API::_GSinitReadFIFO2 API_GSinitReadFIFO2 = NULL;

void CALLBACK s_GSinitReadFIFO2(u64 *pMem, int qwc)
{
	if (API_GSinitReadFIFO2 == NULL)
		return;

	API_GSinitReadFIFO2((uint8*)(pMem), qwc);
}


PCSX2Lib::API::_GSreadFIFO2 API_GSreadFIFO2 = NULL;

void CALLBACK s_GSreadFIFO2(u64 *pMem, int qwc)
{
	if (API_GSreadFIFO2 == NULL)
		return;

	API_GSreadFIFO2((uint8*)(pMem), qwc);
}




PCSX2_EXPORT void STDAPICALLTYPE setGS(PCSX2Lib::API::GS_API *a_API)
{
    if (a_API == nullptr) {

        GScallbackopen = nullptr;
        GSvsync = nullptr;
        GSgifTransfer = nullptr;
        GSirqCallback = nullptr;
        GSsetBaseMem = nullptr;
        GSsetGameCRC = nullptr;
        GSsetFrameSkip = nullptr;
        GSsetVsync = nullptr;
        GSreset = nullptr;
        GSinitReadFIFO = nullptr;
        GSreadFIFO = nullptr;
        GSinitReadFIFO2 = nullptr;
        GSreadFIFO2 = nullptr;
        GSgifSoftReset = nullptr;
    
		return;
	}

	GSopen2 = a_API->GSopen2;
    GScallbackopen = a_API->GScallbackopen;
    GSvsync = a_API->GSvsync;
	API_GSgifTransfer = a_API->GSgifTransfer;
    GSirqCallback = a_API->GSirqCallback;
    GSsetBaseMem = a_API->GSsetBaseMem;
    GSsetGameCRC = a_API->GSsetGameCRC;
    GSsetFrameSkip = a_API->GSsetFrameSkip;
    GSsetVsync = a_API->GSsetVsync;
    GSreset = a_API->GSreset;
	API_GSinitReadFIFO = a_API->GSinitReadFIFO;
	API_GSreadFIFO = a_API->GSreadFIFO;
	API_GSinitReadFIFO2 = a_API->GSinitReadFIFO2;
	API_GSreadFIFO2 = a_API->GSreadFIFO2;
    GSgifSoftReset = a_API->GSgifSoftReset;

	GSgifTransfer = s_GSgifTransfer;
	GSinitReadFIFO = s_GSinitReadFIFO;
	GSreadFIFO = s_GSreadFIFO;
	GSinitReadFIFO2 = s_GSinitReadFIFO2;
	GSreadFIFO2 = s_GSreadFIFO2;
}

PCSX2_EXPORT void STDAPICALLTYPE setCDVD(PCSX2Lib::API::CDVD_API *aPtrCDVD)
{
    CDVD = (CDVD_API *)aPtrCDVD;
}

VmStateBuffer g_VmStateBuffer;

PCSX2_EXPORT void *STDAPICALLTYPE getFreezeInternalsFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    saveme.FreezeInternals();

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}


class BaseSavestateEntry
{
protected:
    BaseSavestateEntry() = default;

public:
    virtual ~BaseSavestateEntry() = default;

    virtual wxString GetFilename() const = 0;
    virtual void FreezeIn(VmStateBuffer *reader) const = 0;
    virtual void FreezeOut(SaveStateBase &writer) const = 0;
    virtual bool IsRequired() const = 0;
};

class MemorySavestateEntry : public BaseSavestateEntry
{
protected:
    MemorySavestateEntry() {}
    virtual ~MemorySavestateEntry() = default;

public:
    virtual void FreezeIn(VmStateBuffer *reader) const;
    virtual void FreezeOut(SaveStateBase &writer) const;
    virtual bool IsRequired() const { return true; }

protected:
    virtual u8 *GetDataPtr() const = 0;
    virtual uint GetDataSize() const = 0;
};

void MemorySavestateEntry::FreezeOut(SaveStateBase &writer) const
{
    writer.FreezeMem(GetDataPtr(), GetDataSize());
}

void MemorySavestateEntry::FreezeIn(VmStateBuffer *reader) const
{
    const uint entrySize = reader->GetSizeInBytes();
    const uint expectedSize = GetDataSize();

    if (entrySize < expectedSize) {
        Console.WriteLn(Color_Yellow, " '%s' is incomplete (expected 0x%x bytes, loading only 0x%x bytes)",
                        WX_STR(GetFilename()), expectedSize, entrySize);
    }

    uint copylen = std::min(entrySize, expectedSize);

    memcpy(GetDataPtr(), reader->GetPtr(), copylen);
}

class SavestateEntry_EmotionMemory : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_EmotionMemory() = default;

    wxString GetFilename() const { return L"eeMemory.bin"; }
    u8 *GetDataPtr() const { return eeMem->Main; }
    uint GetDataSize() const { return sizeof(eeMem->Main); }

    virtual void FreezeIn(VmStateBuffer *reader) const
    {
        SysClearExecutionCache();
        MemorySavestateEntry::FreezeIn(reader);
    }
};

class SavestateEntry_IopMemory : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_IopMemory() = default;

    wxString GetFilename() const { return L"iopMemory.bin"; }
    u8 *GetDataPtr() const { return iopMem->Main; }
    uint GetDataSize() const { return sizeof(iopMem->Main); }
};

class SavestateEntry_HwRegs : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_HwRegs() = default;

    wxString GetFilename() const { return L"eeHwRegs.bin"; }
    u8 *GetDataPtr() const { return eeHw; }
    uint GetDataSize() const { return sizeof(eeHw); }
};

class SavestateEntry_IopHwRegs : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_IopHwRegs() = default;

    wxString GetFilename() const { return L"iopHwRegs.bin"; }
    u8 *GetDataPtr() const { return iopHw; }
    uint GetDataSize() const { return sizeof(iopHw); }
};

class SavestateEntry_Scratchpad : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_Scratchpad() = default;

    wxString GetFilename() const { return L"Scratchpad.bin"; }
    u8 *GetDataPtr() const { return eeMem->Scratch; }
    uint GetDataSize() const { return sizeof(eeMem->Scratch); }
};

class SavestateEntry_VU0mem : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_VU0mem() = default;

    wxString GetFilename() const { return L"vu0Memory.bin"; }
    u8 *GetDataPtr() const { return vuRegs[0].Mem; }
    uint GetDataSize() const { return VU0_MEMSIZE; }
};

class SavestateEntry_VU1mem : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_VU1mem() = default;

    wxString GetFilename() const { return L"vu1Memory.bin"; }
    u8 *GetDataPtr() const { return vuRegs[1].Mem; }
    uint GetDataSize() const { return VU1_MEMSIZE; }
};

class SavestateEntry_VU0prog : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_VU0prog() = default;

    wxString GetFilename() const { return L"vu0MicroMem.bin"; }
    u8 *GetDataPtr() const { return vuRegs[0].Micro; }
    uint GetDataSize() const { return VU0_PROGSIZE; }
};

class SavestateEntry_VU1prog : public MemorySavestateEntry
{
public:
    virtual ~SavestateEntry_VU1prog() = default;

    wxString GetFilename() const { return L"vu1MicroMem.bin"; }
    u8 *GetDataPtr() const { return vuRegs[1].Micro; }
    uint GetDataSize() const { return VU1_PROGSIZE; }
};

PCSX2_EXPORT void *STDAPICALLTYPE getEmotionMemoryFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_EmotionMemory l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getIopMemoryFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_IopMemory l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getHwRegsFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_HwRegs l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getIopHwRegsFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_IopHwRegs l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getScratchpadFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_Scratchpad l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getVU0memFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_VU0mem l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getVU1memFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_VU1mem l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getVU0progFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_VU0prog l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

PCSX2_EXPORT void *STDAPICALLTYPE getVU1progFunc(uint32 *a_PtrSizeInBytes)
{
    if (a_PtrSizeInBytes == nullptr)
        return nullptr;

    g_VmStateBuffer.Dispose();

    memSavingState saveme(g_VmStateBuffer);

    SavestateEntry_VU1prog l_SavestateEntry;

    l_SavestateEntry.FreezeOut(saveme);

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}


PCSX2_EXPORT void STDAPICALLTYPE setFreezeInternalsFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

    VmStateBuffer buffer(a_SizeInBytes, L"StubBuffer"); // start with an 8 meg buffer to avoid frequent reallocation.

    memcpy(buffer.GetPtr(), a_PtrMemory, a_SizeInBytes);

    memLoadingState l(buffer);

    SaveStateBase &lt = l;

    try {
        lt = l.FreezeBios();
    } catch (...) {
    }

    lt.FreezeInternals();
}

PCSX2_EXPORT void STDAPICALLTYPE setEmotionMemoryFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_EmotionMemory l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setIopMemoryFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_IopMemory l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setHwRegsFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_HwRegs l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setIopHwRegsFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_IopHwRegs l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setScratchpadFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_Scratchpad l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setVU0memFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_VU0mem l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setVU1memFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_VU1mem l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setVU0progFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_VU0prog l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE setVU1progFunc(void *a_PtrMemory, uint32 a_SizeInBytes)
{
    if (a_PtrMemory == nullptr)
        return;

    if (a_SizeInBytes == 0)
        return;

	throw;

    //SavestateEntry_VU1prog l_SavestateEntry;

    //VmStateBuffer buffer(L"StubBuffer", (u8 *)a_PtrMemory, a_SizeInBytes); // start with an 8 meg buffer to avoid frequent reallocation.

    //l_SavestateEntry.FreezeIn(&buffer);

    //buffer.Reset();
}


_DoFreezeCallback DoFreezeCallback;

PCSX2_EXPORT extern void STDAPICALLTYPE setDoFreezeCallback(_DoFreezeCallback aDoFreezeCallback)
{
    DoFreezeCallback = aDoFreezeCallback;
}


Threading::MutexRecursive m_mtx_PluginStatus;

// For internal use only, unless you're the MTGS.  Then it's for you too!
// Returns false if the plugin returned an error.
bool DoFreeze(PluginsEnum_t pid, int mode, freezeData *data)
{
    if ((pid == PluginId_GS) && !GetMTGS().IsSelf()) {
        // GS needs some thread safety love...

        MTGS_FreezeData woot = {data, 0};
        GetMTGS().Freeze(mode, woot);
        return woot.retval != -1;
    } else {
        ScopedLock lock(m_mtx_PluginStatus);

        return DoFreezeCallback(data, mode, pid) != -1;
    }
}

static void FreezeOut(PluginsEnum_t pid)
{

    g_VmStateBuffer.Dispose();

    // No locking needed -- DoFreeze locks as needed, and this avoids MTGS deadlock.
    //ScopedLock lock( m_mtx_PluginStatus );

    freezeData fP = {0, NULL};
    if (!DoFreeze(pid, FREEZE_SIZE, &fP))
        return;
    if (!fP.size)
        return;

    g_VmStateBuffer.MakeRoomFor(fP.size);

    fP.data = (s8 *)g_VmStateBuffer.GetPtr();

    Console.Indent().WriteLn("Saving %s", tbl_PluginInfo[pid].shortname);

    if (!DoFreeze(pid, FREEZE_SAVE, &fP))
        g_VmStateBuffer.Dispose();
}

PCSX2_EXPORT void *STDAPICALLTYPE getFreezeOutFunc(uint32 *a_PtrSizeInBytes, uint32 a_ModuleID)
{

    // No locking needed -- DoFreeze locks as needed, and this avoids MTGS deadlock.
    //ScopedLock lock( m_mtx_PluginStatus );

    if (!GetMTGS().IsSelf()) {
        FreezeOut((PluginsEnum_t)a_ModuleID);
    }

    *a_PtrSizeInBytes = g_VmStateBuffer.GetSizeInBytes();

    return g_VmStateBuffer.GetPtr();
}

void FreezeIn(PluginsEnum_t pid, s8 *data)
{
    // No locking needed -- DoFreeze locks as needed, and this avoids MTGS deadlock.
    //ScopedLock lock( m_mtx_PluginStatus );

    freezeData fP = {0, NULL};
    if (!DoFreeze(pid, FREEZE_SIZE, &fP))
        fP.size = 0;

    Console.Indent().WriteLn("Loading %s", tbl_PluginInfo[pid].shortname);

    fP.data = data;

    if (!DoFreeze(pid, FREEZE_LOAD, &fP))
        throw;
}

PCSX2_EXPORT void STDAPICALLTYPE setFreezeInFunc(void *data, int32 a_ModuleID)
{

    // No locking needed -- DoFreeze locks as needed, and this avoids MTGS deadlock.
    //ScopedLock lock( m_mtx_PluginStatus );

    if (!GetMTGS().IsSelf()) {
        FreezeIn((PluginsEnum_t)a_ModuleID, (s8 *)data);
    }
}

static Pcsx2Config parsePcsx2Config(const wchar_t *a_config)
{
    using namespace pugi;

    Pcsx2Config l_Pcsx2Config;

    xml_document l_xmlDoc;

    auto l_XMLRes = l_xmlDoc.load_string(a_config);

    if (l_XMLRes.status == xml_parse_status::status_ok) {
        auto l_document = l_xmlDoc.document_element();

        if (l_document.empty())
            return l_Pcsx2Config;

        if (std::wstring(l_document.name()) == L"Pcsx2Config") {
            auto l_Attribute = l_document.attribute(L"bitset");

            if (!l_Attribute.empty()) {
                l_Pcsx2Config.bitset = l_Attribute.as_uint();
            }

            auto l_ChildNode = l_document.first_child();

            while (!l_ChildNode.empty()) {
                if (std::wstring(l_ChildNode.name()) == L"Cpu") {
                    auto l_CpuChildNode = l_ChildNode.first_child();

                    while (!l_CpuChildNode.empty()) {
                        l_Attribute = l_CpuChildNode.attribute(L"bitset");

                        if (std::wstring(l_CpuChildNode.name()) == L"Recompiler") {
                            if (!l_Attribute.empty()) {
                                l_Pcsx2Config.Cpu.Recompiler.bitset = l_Attribute.as_uint();
                            }
                        } else {
                            l_Attribute = l_CpuChildNode.attribute(L"bitmask");

                            if (std::wstring(l_CpuChildNode.name()) == L"sseMXCSR") {
                                if (!l_Attribute.empty()) {
                                    l_Pcsx2Config.Cpu.sseMXCSR.bitmask = l_Attribute.as_uint();
                                }
                            } else if (std::wstring(l_CpuChildNode.name()) == L"sseVUMXCSR") {
                                if (!l_Attribute.empty()) {
                                    l_Pcsx2Config.Cpu.sseVUMXCSR.bitmask = l_Attribute.as_uint();
                                }
                            }
                        }

                        l_CpuChildNode = l_CpuChildNode.next_sibling();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"GS") {
                    l_Attribute = l_ChildNode.attribute(L"DisableOutput");

                    if (!l_Attribute.empty()) {
                        //l_Pcsx2Config.GS.DisableOutput = l_Attribute.as_bool();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FrameLimitEnable");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FrameLimitEnable = l_Attribute.as_bool();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FramerateNTSC");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FramerateNTSC.SetRaw(l_Attribute.as_int());
                    }

                    l_Attribute = l_ChildNode.attribute(L"FrameratePAL");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FrameratePAL.SetRaw(l_Attribute.as_int());
                    }

                    l_Attribute = l_ChildNode.attribute(L"FrameSkipEnable");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FrameSkipEnable = l_Attribute.as_bool();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FramesToDraw");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FramesToDraw = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FramesToSkip");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.FramesToSkip = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"LimitScalar");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.LimitScalar.SetRaw(l_Attribute.as_int());
                    }

                    l_Attribute = l_ChildNode.attribute(L"SynchronousMTGS");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.SynchronousMTGS = l_Attribute.as_bool();
                    }

                    l_Attribute = l_ChildNode.attribute(L"VsyncEnable");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.VsyncEnable = (VsyncMode)l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"VsyncQueueSize");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.GS.VsyncQueueSize = l_Attribute.as_int();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"Speedhacks") {
                    l_Attribute = l_ChildNode.attribute(L"bitset");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Speedhacks.bitset = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"EECycleRate");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Speedhacks.EECycleRate = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"EECycleSkip");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Speedhacks.EECycleSkip = l_Attribute.as_int();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"Gamefixes") {
                    l_Attribute = l_ChildNode.attribute(L"bitset");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Gamefixes.bitset = l_Attribute.as_int();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"Profiler") {
                    l_Attribute = l_ChildNode.attribute(L"bitset");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Profiler.bitset = l_Attribute.as_int();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"Debugger") {
                    l_Attribute = l_ChildNode.attribute(L"bitset");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.bitset = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FontWidth");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.FontWidth = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"FontHeight");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.FontHeight = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"WindowWidth");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.WindowWidth = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"WindowHeight");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.WindowHeight = l_Attribute.as_int();
                    }

                    l_Attribute = l_ChildNode.attribute(L"MemoryViewBytesPerRow");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Debugger.MemoryViewBytesPerRow = l_Attribute.as_int();
                    }
                } else if (std::wstring(l_ChildNode.name()) == L"Trace") {
                    l_Attribute = l_ChildNode.attribute(L"Enabled");

                    if (!l_Attribute.empty()) {
                        l_Pcsx2Config.Trace.Enabled = l_Attribute.as_bool();
                    }


                    auto l_TraceChildNode = l_ChildNode.first_child();

                    while (!l_TraceChildNode.empty()) {
                        l_Attribute = l_TraceChildNode.attribute(L"bitset");

                        if (std::wstring(l_TraceChildNode.name()) == L"EE") {
                            if (!l_Attribute.empty()) {
                                l_Pcsx2Config.Trace.EE.bitset = l_Attribute.as_uint();
                            }
                        } else if (std::wstring(l_TraceChildNode.name()) == L"IOP") {
                            if (!l_Attribute.empty()) {
                                l_Pcsx2Config.Trace.IOP.bitset = l_Attribute.as_uint();
                            }
                        }

                        l_TraceChildNode = l_TraceChildNode.next_sibling();
                    }
                }

                l_ChildNode = l_ChildNode.next_sibling();
            }
        }
    }

    return l_Pcsx2Config;
}

PCSX2_EXPORT extern void STDAPICALLTYPE ApplySettingsFunc(const wchar_t *a_config)
{
    auto lPcsx2Config = parsePcsx2Config(a_config);

    GetCoreThread().ApplySettings(lPcsx2Config);

    SetGSConfig() = lPcsx2Config.GS;
}

PCSX2_EXPORT extern void STDAPICALLTYPE VTLB_Alloc_PpmapFinc()
{
    vtlb_Alloc_Ppmap();
}


PCSX2_EXPORT void STDAPICALLTYPE AllocateCoreStuffsFunc(const wchar_t *a_config)
{
    ApplySettingsFunc(a_config);

    GetVmReserve().ReserveAll();

    if (!m_CpuProviders) {
        // FIXME : Some or all of SysCpuProviderPack should be run from the SysExecutor thread,
        // so that the thread is safely blocked from being able to start emulation.

        m_CpuProviders = std::make_unique<SysCpuProviderPack>();

        if (m_CpuProviders->HadSomeFailures(g_Conf->EmuOptions.Cpu.Recompiler)) {
            Pcsx2Config::RecompilerOptions &recOps = g_Conf->EmuOptions.Cpu.Recompiler;
        }
    }
}

PCSX2_EXPORT void STDAPICALLTYPE DetectCpuAndUserModeFunc()
{
    AffinityAssert_AllowFrom_MainUI();

#ifdef _M_X86
    x86caps.Identify();
    x86caps.CountCores();
    x86caps.SIMD_EstablishMXCSRmask();

    if (!x86caps.hasStreamingSIMD2Extensions) {
        // This code will probably never run if the binary was correctly compiled for SSE2
        // SSE2 is required for any decent speed and is supported by more than decade old x86 CPUs
        throw Exception::HardwareDeficiency()
            .SetDiagMsg(L"Critical Failure: SSE2 Extensions not available.")
            .SetUserMsg(_("SSE2 extensions are not available.  PCSX2 requires a cpu that supports the SSE2 instruction set."));
    }
#endif
}

PCSX2_EXPORT void STDAPICALLTYPE PCSX2_Hle_SetElfPathFunc(const char *elfFileName)
{
    Hle_SetElfPath(elfFileName);
}

PCSX2_EXPORT void STDAPICALLTYPE SysThreadBase_ResumeFunc()
{
    InitCPUTicks();

    GetCoreThread().Resume();
}

PCSX2_EXPORT void STDAPICALLTYPE SysThreadBase_SuspendFunc()
{
    GetCoreThread().Suspend(true);
}

PCSX2_EXPORT void STDAPICALLTYPE SysThreadBase_ResetFunc()
{
    GetCoreThread().Reset();
}

PCSX2_EXPORT void STDAPICALLTYPE SysThreadBase_CancelFunc()
{
    GetCoreThread().Cancel();
}



_Callback PluginsInitCallback;

_Callback PluginsCloseCallback;

_Callback PluginsShutdownCallback;

_Callback PluginsOpenCallback;

_BoolCallback PluginsAreLoadedCallback;

PCSX2_EXPORT void STDAPICALLTYPE setPluginsInitCallback(_Callback aPluginsInitCallback)
{
    PluginsInitCallback = aPluginsInitCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE setPluginsCloseCallback(_Callback aPluginsCloseCallback)
{
    PluginsCloseCallback = aPluginsCloseCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE setPluginsShutdownCallback(_Callback aPluginsShutdownCallback)
{
    PluginsShutdownCallback = aPluginsShutdownCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE setPluginsOpenCallback(_Callback aPluginsOpenCallback)
{
    PluginsOpenCallback = aPluginsOpenCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE setPluginsAreLoadedCallback(_BoolCallback aPluginsAreLoadedCallback)
{
    PluginsAreLoadedCallback = aPluginsAreLoadedCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE resetCallbacksFunc()
{
    PluginsInitCallback = nullptr;

    PluginsCloseCallback = nullptr;

    PluginsShutdownCallback = nullptr;

    PluginsOpenCallback = nullptr;

    PluginsAreLoadedCallback = nullptr;
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_ResumeFunc()
{
    GetMTGS().Resume();
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_WaitForOpenFunc()
{
    GetMTGS().WaitForOpen();
}

PCSX2_EXPORT bool STDAPICALLTYPE MTGS_IsSelfFunc()
{
    return GetMTGS().IsSelf();
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_SuspendFunc()
{
    GetMTGS().Suspend();
}

extern void MTGS_ResetQuick();

PCSX2_EXPORT void STDAPICALLTYPE MTGS_ResetFunc()
{
    MTGS_ResetQuick();
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_CancelFunc()
{
    GetMTGS().Cancel();
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_FreezeFunc(int mode, void *data)
{
    GetMTGS().Freeze(mode, *((MTGS_FreezeData *)data));
}

PCSX2_EXPORT void STDAPICALLTYPE MTGS_WaitGSFunc()
{
    GetMTGS().WaitGS();
}

PCSX2_EXPORT void STDAPICALLTYPE MTVU_CancelFunc()
{
    vu1Thread.Cancel();
}


void spu2DMA4Irq();
void spu2DMA7Irq();
void spu2Irq();



PCSX2_EXPORT void STDAPICALLTYPE openPlugin_FWFunc()
{
    if (FWirqCallback != nullptr)
        FWirqCallback(fwIrq);
}

PCSX2_EXPORT void STDAPICALLTYPE ForgetLoadedPatchesFunc()
{
    ForgetLoadedPatches();
}


_Callback UI_EnableSysActionsCallback;

PCSX2_EXPORT void STDAPICALLTYPE setUI_EnableSysActionsCallback(_Callback aUI_EnableSysActionsCallback)
{
    UI_EnableSysActionsCallback = aUI_EnableSysActionsCallback;
}


extern void inifile_commandProxy(const wxString &cmd);


PCSX2_EXPORT void STDAPICALLTYPE inifile_commandFunc(const wchar_t *cmd)
{
    inifile_commandProxy(cmd);
}


_CallbackOneUInt LoadAllPatchesAndStuffCallback;

PCSX2_EXPORT void STDAPICALLTYPE setLoadAllPatchesAndStuffCallback(_CallbackOneUInt aLoadAllPatchesAndStuffCallback)
{
    LoadAllPatchesAndStuffCallback = aLoadAllPatchesAndStuffCallback;
}


PCSX2_EXPORT void STDAPICALLTYPE setSioSetGameSerialFunc(const wchar_t *serial)
{
    sioSetGameSerial(serial);
}

PCSX2_EXPORT bool STDAPICALLTYPE getGameStartedFunc()
{
    return g_GameStarted;
}

PCSX2_EXPORT bool STDAPICALLTYPE getGameLoadingFunc()
{
    return g_GameLoading;
}

PCSX2_EXPORT unsigned int STDAPICALLTYPE getElfCRCFunc()
{
    return ElfCRC;
}

PCSX2_EXPORT void STDAPICALLTYPE releaseWCHARStringFunc(wchar_t *aPtrString)
{
    if (aPtrString != nullptr)
        delete[] aPtrString;
}

PCSX2_EXPORT void STDAPICALLTYPE getSysGetBiosDiscIDFunc(wchar_t **aPtrPtrSysGetBiosDiscID)
{
    auto lSysGetBiosDiscID = SysGetBiosDiscID().ToStdWstring();

    *aPtrPtrSysGetBiosDiscID = new wchar_t[lSysGetBiosDiscID.size() + 1];

    wcscpy_s(*aPtrPtrSysGetBiosDiscID, lSysGetBiosDiscID.size() + 1, lSysGetBiosDiscID.c_str());
}

PCSX2_EXPORT void STDAPICALLTYPE setMcd(void *aVoidMcd)
{
    setInnerMcd(aVoidMcd);
}

void gsUpdateVSyncRate();

PCSX2_EXPORT void STDAPICALLTYPE gsUpdateFrequencyCallFunc()
{
    //switch (g_LimiterMode)
    //{
    //case LimiterModeType::Limit_Nominal:
    //	config.GS.LimitScalar = g_Conf->Framerate.NominalScalar;
    //	break;
    //case LimiterModeType::Limit_Slomo:
    //	config.GS.LimitScalar = g_Conf->Framerate.SlomoScalar;
    //	break;
    //case LimiterModeType::Limit_Turbo:
    //	config.GS.LimitScalar = g_Conf->Framerate.TurboScalar;
    //	break;
    //default:
    //	pxAssert("Unknown framelimiter mode!");
    //}
    gsUpdateVSyncRate();
}

PCSX2_EXPORT void STDAPICALLTYPE getSysGetDiscIDFunc(wchar_t **aPtrPtrSysGetDiscID)
{
    auto lSysGetDiscID = SysGetDiscID().ToStdWstring();

    *aPtrPtrSysGetDiscID = new wchar_t[lSysGetDiscID.size() + 1];

    wcscpy_s(*aPtrPtrSysGetDiscID, lSysGetDiscID.size() + 1, lSysGetDiscID.c_str());
}


_CallbackOneUINT8PtrOneUINT LoadBIOSCallback;

PCSX2_EXPORT void STDAPICALLTYPE setLoadBIOSCallbackCallback(_CallbackOneUINT8PtrOneUINT aLoadBIOSCallback)
{
    LoadBIOSCallback = aLoadBIOSCallback;
}


_CDVDNVMCallback CDVDNVMCallback;

PCSX2_EXPORT void STDAPICALLTYPE setCDVDNVMCallback(_CDVDNVMCallback aCDVDNVMCallback)
{
    CDVDNVMCallback = aCDVDNVMCallback;
}



_CallbackOneUINT8Ptr CDVDGetMechaVerCallback;

PCSX2_EXPORT void STDAPICALLTYPE setCDVDGetMechaVerCallback(_CallbackOneUINT8Ptr aCDVDGetMechaVerCallback)
{
    CDVDGetMechaVerCallback = aCDVDGetMechaVerCallback;
}

PCSX2_EXPORT void STDAPICALLTYPE vu1Thread_WaitVUFunc()
{
    vu1Thread.WaitVU();
}
