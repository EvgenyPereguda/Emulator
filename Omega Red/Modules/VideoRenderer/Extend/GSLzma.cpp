/*
 *	Copyright (C) 2015-2015 Gregory hainaut
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSLzma.h"

GSDumpFile::GSDumpFile(char* filename, const char* repack_filename)
{
	m_fp = fopen(filename, "rb");
	if (m_fp == nullptr)
	{
		fprintf(stderr, "failed to open %s\n", filename);
		throw "BAD"; // Just exit the program
	}

	m_repack_fp = nullptr;
	if (repack_filename)
	{
		m_repack_fp = fopen(repack_filename, "wb");
		if (m_repack_fp == nullptr)
			fprintf(stderr, "failed to open %s for repack\n", repack_filename);
	}
}

void GSDumpFile::Repack(void* ptr, size_t size)
{
	if (m_repack_fp == nullptr)
		return;

	size_t ret = fwrite(ptr, 1, size, m_repack_fp);
	if (ret != size)
		fprintf(stderr, "Failed to repack\n");
}

GSDumpFile::~GSDumpFile()
{
	if (m_fp)
		fclose(m_fp);
	if (m_repack_fp)
		fclose(m_repack_fp);
}

/******************************************************************/
GSDumpLzma::GSDumpLzma(char* filename, const char* repack_filename)
	: GSDumpFile(filename, repack_filename)
{

}

void GSDumpLzma::Decompress()
{
}

bool GSDumpLzma::IsEof()
{
	return true;
}

bool GSDumpLzma::Read(void* ptr, size_t size)
{
	return false;
}

GSDumpLzma::~GSDumpLzma()
{
}

/******************************************************************/

GSDumpRaw::GSDumpRaw(char* filename, const char* repack_filename)
	: GSDumpFile(filename, repack_filename)
{
	m_buff_size = 0;
	m_area      = nullptr;
	m_inbuf     = nullptr;
	m_avail     = 0;
	m_start     = 0;
}

bool GSDumpRaw::IsEof()
{
	return !!feof(m_fp);
}

bool GSDumpRaw::Read(void* ptr, size_t size)
{
	return false;
}
