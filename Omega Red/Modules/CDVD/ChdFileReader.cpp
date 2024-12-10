
#include "PrecompiledHeader.h"
#include "AsyncFileReader.h"
#include "./CDVD/ChdFileReader.h"

ChdFileReader::ChdFileReader(){}

bool ChdFileReader::Open(const wxString& fileName) 
{
	return false;
}

int ChdFileReader::ReadSync(void* pBuffer, uint sector, uint count)
{
	return 0;
}

void ChdFileReader::BeginRead(void* pBuffer, uint sector, uint count) {}
int ChdFileReader::FinishRead(void)
{
	return 0;
}

void ChdFileReader::Close(void) {}
void ChdFileReader::SetBlockSize(uint blocksize) {}
uint ChdFileReader::GetBlockSize() const
{
	return 0;
}
uint ChdFileReader::GetBlockCount(void) const
{
	return 0;
}

bool ChdFileReader::CanHandle(const wxString& fileName)
{
	return false;
}
