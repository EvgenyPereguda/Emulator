/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#include "PrecompiledHeader.h"
#include "CDVD\IsoFileFormats.h"
#include "IopCommon.h"

#include <errno.h>

OutputIsoFile::OutputIsoFile()
{
}

OutputIsoFile::~OutputIsoFile()
{
}

void OutputIsoFile::_init()
{
	m_version = 0;

	m_offset = 0;
	m_blockofs = 0;
	m_blocksize = 0;
	m_blocks = 0;
}

void OutputIsoFile::Create(const wxString& filename, int version)
{
}

// Generates format header information for blockdumps.
void OutputIsoFile::WriteHeader(int _blockofs, uint _blocksize, uint _blocks)
{
}

void OutputIsoFile::WriteSector(const u8* src, uint lsn)
{
}

void OutputIsoFile::Close()
{
}

void OutputIsoFile::WriteBuffer(const void* src, size_t size)
{
}

bool OutputIsoFile::IsOpened() const
{
	return false;
}

u32 OutputIsoFile::GetBlockSize() const
{
	return m_blocksize;
}
