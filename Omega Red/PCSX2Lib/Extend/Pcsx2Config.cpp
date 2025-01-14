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
#include "Utilities\FixedPointTypes.h"
#include "Utilities\FixedPointTypes.inl"

#include "Config.h"
#include "AppConfig.h"

extern LimiterModeType g_LimiterMode;

void TraceLogFilters::LoadSave(IniInterface& ini)
{
}

Pcsx2Config::SpeedhackOptions::SpeedhackOptions()
{
	DisableAll();

	// Set recommended speedhacks to enabled by default. They'll still be off globally on resets.
	//WaitLoop = true;
	//IntcStat = true;
	//vuFlagHack = true;
}

Pcsx2Config::SpeedhackOptions& Pcsx2Config::SpeedhackOptions::DisableAll()
{
	bitset = 0;
	EECycleRate = 0;
	EECycleSkip = 0;

	return *this;
}

void Pcsx2Config::SpeedhackOptions::LoadSave(IniInterface& ini)
{
}

void Pcsx2Config::ProfilerOptions::LoadSave(IniInterface& ini)
{
}

Pcsx2Config::RecompilerOptions::RecompilerOptions()
{
	bitset = 0;

	//StackFrameChecks	= false;
	//PreBlockCheckEE	= false;

	// All recs are enabled by default.

	//EnableEE = true;
	//EnableEECache = false;
	//EnableIOP = true;
	//EnableVU0 = true;
	//EnableVU1 = true;

	//UseMicroVU0 = true;
	//UseMicroVU1 = true;

	// vu and fpu clamping default to standard overflow.
	//vuOverflow = true;
	//vuExtraOverflow = false;
	//vuSignOverflow = false;
	//vuUnderflow = false;

	//fpuOverflow = true;
	//fpuExtraOverflow = false;
	//fpuFullMode = false;
}

void Pcsx2Config::RecompilerOptions::ApplySanityCheck()
{
	bool fpuIsRight = true;

	if (fpuExtraOverflow)
		fpuIsRight = fpuOverflow;

	if (fpuFullMode)
		fpuIsRight = fpuOverflow && fpuExtraOverflow;

	if (!fpuIsRight)
	{
		// Values are wonky; assume the defaults.
		fpuOverflow = RecompilerOptions().fpuOverflow;
		fpuExtraOverflow = RecompilerOptions().fpuExtraOverflow;
		fpuFullMode = RecompilerOptions().fpuFullMode;
	}

	bool vuIsOk = true;

	if (vuExtraOverflow) vuIsOk = vuIsOk && vuOverflow;
	if (vuSignOverflow) vuIsOk = vuIsOk && vuExtraOverflow;

	if (!vuIsOk)
	{
		// Values are wonky; assume the defaults.
		vuOverflow = RecompilerOptions().vuOverflow;
		vuExtraOverflow = RecompilerOptions().vuExtraOverflow;
		vuSignOverflow = RecompilerOptions().vuSignOverflow;
		vuUnderflow = RecompilerOptions().vuUnderflow;
	}
}

void Pcsx2Config::RecompilerOptions::LoadSave(IniInterface& ini)
{
}

Pcsx2Config::CpuOptions::CpuOptions()
{
	sseMXCSR.bitmask = DEFAULT_sseMXCSR;
	sseVUMXCSR.bitmask = DEFAULT_sseVUMXCSR;
}

void Pcsx2Config::CpuOptions::ApplySanityCheck()
{
	//sseMXCSR.ClearExceptionFlags().DisableExceptions();
	//sseVUMXCSR.ClearExceptionFlags().DisableExceptions();

	Recompiler.ApplySanityCheck();
}

void Pcsx2Config::CpuOptions::LoadSave(IniInterface& ini)
{
}

// Default GSOptions
Pcsx2Config::GSOptions::GSOptions()
{
	FrameLimitEnable = true;
	FrameSkipEnable = false;
	VsyncEnable = VsyncMode::Off;

	SynchronousMTGS = false;
	//DisableOutput = false;
	VsyncQueueSize = 2;

	FramesToDraw = 2;
	FramesToSkip = 2;

	LimitScalar = 1.0;
	FramerateNTSC = 59.94;
	FrameratePAL = 50.0;
}

void Pcsx2Config::GSOptions::LoadSave(IniInterface& ini)
{
}

int Pcsx2Config::GSOptions::GetVsync() const
{
	if (g_LimiterMode == Limit_Turbo || !FrameLimitEnable)
		return 0;

	// D3D only support a boolean state. OpenGL waits a number of vsync
	// interrupt (negative value for late vsync).
	switch (VsyncEnable) {
	case VsyncMode::Adaptive: return -1;
	case VsyncMode::Off: return 0;
	case VsyncMode::On: return 1;

	default: return 0;
	}
}

const wxChar *const tbl_GamefixNames[] =
{
	L"VuAddSub",
	L"VuClipFlag",
	L"FpuCompare",
	L"FpuMul",
	L"FpuNegDiv",
	L"XGKick",
	L"IpuWait",
	L"EETiming",
	L"SkipMpeg",
	L"OPHFlag",
	L"DMABusy",
	L"VIFFIFO",
	L"VIF1Stall",
	L"GIFFIFO",
	L"FMVinSoftware",
	L"GoemonTlb",
	L"ScarfaceIbit"
};

const __fi wxChar* EnumToString(GamefixId id)
{
	return tbl_GamefixNames[id];
}

// all gamefixes are disabled by default.
Pcsx2Config::GamefixOptions::GamefixOptions()
{
	DisableAll();
}

Pcsx2Config::GamefixOptions& Pcsx2Config::GamefixOptions::DisableAll()
{
	bitset = 0;
	return *this;
}

// Enables a full list of gamefixes.  The list can be either comma or pipe-delimited.
//   Example:  "XGKick,IpuWait"  or  "EEtiming,FpuCompare"
// If an unrecognized tag is encountered, a warning is printed to the console, but no error
// is generated.  This allows the system to function in the event that future versions of
// PCSX2 remove old hacks once they become obsolete.
void Pcsx2Config::GamefixOptions::Set(const wxString& list, bool enabled)
{
}

void Pcsx2Config::GamefixOptions::Set(GamefixId id, bool enabled)
{
	EnumAssert(id);
	switch (id)
	{
	case Fix_VuAddSub:		VuAddSubHack = enabled;	break;
	case Fix_FpuCompare:	FpuCompareHack = enabled;	break;
	case Fix_FpuMultiply:	FpuMulHack = enabled;	break;
	case Fix_FpuNegDiv:		FpuNegDivHack = enabled;	break;
	case Fix_XGKick:		XgKickHack = enabled;	break;
	case Fix_IpuWait:		IPUWaitHack = enabled;	break;
	case Fix_EETiming:		EETimingHack = enabled;	break;
	case Fix_SkipMpeg:		SkipMPEGHack = enabled;	break;
	case Fix_OPHFlag:		OPHFlagHack = enabled;  break;
	case Fix_DMABusy:		DMABusyHack = enabled;  break;
	case Fix_VIFFIFO:		VIFFIFOHack = enabled;  break;
	case Fix_VIF1Stall:		VIF1StallHack = enabled;  break;
	case Fix_GIFFIFO:		GIFFIFOHack = enabled;  break;
	case Fix_GoemonTlbMiss: GoemonTlbHack = enabled;  break;
	case Fix_ScarfaceIbit:  ScarfaceIbit = enabled;  break;
	case Fix_CrashTagTeamIbit: CrashTagTeamRacingIbit = enabled; break;
	case Fix_VU0Kickstart:	VU0KickstartHack = enabled; break;
		jNO_DEFAULT;
	}
}

bool Pcsx2Config::GamefixOptions::Get(GamefixId id) const
{
	EnumAssert(id);
	switch (id)
	{
	case Fix_VuAddSub:		return VuAddSubHack;
	case Fix_FpuCompare:	return FpuCompareHack;
	case Fix_FpuMultiply:	return FpuMulHack;
	case Fix_FpuNegDiv:		return FpuNegDivHack;
	case Fix_XGKick:		return XgKickHack;
	case Fix_IpuWait:		return IPUWaitHack;
	case Fix_EETiming:		return EETimingHack;
	case Fix_SkipMpeg:		return SkipMPEGHack;
	case Fix_OPHFlag:		return OPHFlagHack;
	case Fix_DMABusy:		return DMABusyHack;
	case Fix_VIFFIFO:		return VIFFIFOHack;
	case Fix_VIF1Stall:		return VIF1StallHack;
	case Fix_GIFFIFO:		return GIFFIFOHack;
	case Fix_GoemonTlbMiss: return GoemonTlbHack;
	case Fix_ScarfaceIbit:  return ScarfaceIbit;
	case Fix_CrashTagTeamIbit: return CrashTagTeamRacingIbit;
	case Fix_VU0Kickstart:	return VU0KickstartHack;
		jNO_DEFAULT;
	}
	return false;		// unreachable, but we still need to suppress warnings >_<
}

void Pcsx2Config::GamefixOptions::LoadSave(IniInterface& ini)
{
}


Pcsx2Config::DebugOptions::DebugOptions()
{
	ShowDebuggerOnStart = false;
	AlignMemoryWindowStart = true;
	FontWidth = 8;
	FontHeight = 12;
	WindowWidth = 0;
	WindowHeight = 0;
	MemoryViewBytesPerRow = 16;
}

void Pcsx2Config::DebugOptions::LoadSave(IniInterface& ini)
{
}




Pcsx2Config::Pcsx2Config()
{
	bitset = 0;
	// Set defaults for fresh installs / reset settings
	McdEnableEjection = true;
	McdFolderAutoManage = true;
	EnablePatches = true;
	BackupSavestate = true;

	CdvdVerboseReads = true;

	UseBOOT2Injection = false;
}

void Pcsx2Config::LoadSave(IniInterface& ini)
{
}

bool Pcsx2Config::MultitapEnabled(uint port) const
{
	pxAssert(port < 2);
	return (port == 0) ? MultitapPort0_Enabled : MultitapPort1_Enabled;
}

void Pcsx2Config::Load(const wxString& srcfile)
{
}

void Pcsx2Config::Save(const wxString& dstfile)
{
}