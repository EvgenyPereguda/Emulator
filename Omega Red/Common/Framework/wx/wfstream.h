
#pragma once

typedef wxLongLong_t wxFileOffset;

class wxFileOutputStream
{
};


class wxFileInputStream
{
	wxString m_file_path;

public:

	wxFileInputStream(const wxString& a_file_path);

	void SeekI(const int& pos);

	int Read(void* a_data, int a_length);

	wxFileOffset GetLength() const;

	wxFileOffset LastRead();
};
