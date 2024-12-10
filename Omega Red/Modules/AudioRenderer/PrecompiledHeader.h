
#pragma once

#include "../../../pcsx2/PrecompiledHeader.h"

typedef void(*_SPU2callback)();

typedef void(*_DMA4callback)();

typedef void(*_DMA7callback)();

extern _SPU2callback g_SPU2callback;

extern _DMA4callback g_DMA4callback;

extern _DMA7callback g_DMA7callback;


typedef void(*SPU2writeCallback)(s32 counter);

typedef void(*SPU2readCallback)(s32 counter);

extern SPU2writeCallback g_SPU2writeCallback;

extern SPU2readCallback g_SPU2readCallback;


typedef void(*SPU2TimeUpdateCallback)(s32 counter);

extern SPU2TimeUpdateCallback g_SPU2TimeUpdateCallback;
