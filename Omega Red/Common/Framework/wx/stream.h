#pragma once

#include "wfstream.h"

// ============================================================================
// base stream classes: wxInputStream and wxOutputStream
// ============================================================================

// ---------------------------------------------------------------------------
// wxStreamBase: common (but non virtual!) base for all stream classes
// ---------------------------------------------------------------------------

class wxStreamBase
{
public:
	bool IsOk() const;
};

// ----------------------------------------------------------------------------
// wxInputStream: base class for the input streams
// ----------------------------------------------------------------------------

class wxInputStream : public wxStreamBase
{
};


// ----------------------------------------------------------------------------
// wxOutputStream: base for the output streams
// ----------------------------------------------------------------------------

class wxOutputStream : public wxStreamBase
{
};