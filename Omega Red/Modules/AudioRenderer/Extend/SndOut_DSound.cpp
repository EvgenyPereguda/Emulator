/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 *
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * SPU2-X is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * SPU2-X is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SPU2-X.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "..\src\Global.h"
#include "..\WinConfig.h"
#include <dsound.h>

extern SetDataCallback g_setAudioData;

#define _WIN32_DCOM

#define DIRECTSOUND_VERSION 0x1000
#include <dsound.h>

struct ds_device_data
{
    std::wstring name;
    GUID guid;
    bool hasGuid;
};

static void Verifyc(HRESULT hr, const char *fn)
{
    if (FAILED(hr)) {
        assert(0);
        throw std::runtime_error("DirectSound returned an error from %s");
    }
}

// Items Specific to DirectSound
#define STRFY(x) #x
#define verifyc(x) Verifyc(x, STRFY(x))

class DSound : public SndOutModule
{
private:
    static const uint MAX_BUFFER_COUNT = 8;
    static const int PacketsPerBuffer = 8;
    static const int BufferSize = SndOutPacketSize * PacketsPerBuffer;

    LONG CurrentVolume = DSBVOLUME_MAX;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Configuration Vars

    std::wstring m_Device;
    u8 m_NumBuffers;
    bool m_DisableGlobalFocus;
    bool m_UseHardware;

    ds_device_data m_devices[32];
    int ndevs;
    GUID DevGuid; // currently employed GUID.
    bool haveGuid;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Instance vars

    int channel;
    int myLastWrite; // last write position, in bytes

    bool dsound_running;
    HANDLE thread;
    DWORD tid;

    IDirectSound8 *dsound;
    IDirectSoundBuffer8 *buffer;
    IDirectSoundNotify8 *buffer_notify;
    HANDLE buffer_events[MAX_BUFFER_COUNT];

    WAVEFORMATEX wfx;

    HANDLE waitEvent;

    template <typename T>
    static DWORD CALLBACK RThread(DSound *obj)
    {
        return obj->Thread<T>();
    }

    template <typename T>
    DWORD CALLBACK Thread()
    {
        static const int BufferSizeBytes = BufferSize * sizeof(T);

        while (dsound_running) {
            u32 rv = WaitForMultipleObjects(m_NumBuffers, buffer_events, FALSE, 200);

            T *p1, *oldp1;
            LPVOID p2;
            DWORD s1, s2;

            u32 poffset = BufferSizeBytes * rv;

            if (FAILED(buffer->Lock(poffset, BufferSizeBytes, (LPVOID *)&p1, &s1, &p2, &s2, 0))) {
                assert(0);
                fputs("* SPU2-X: Directsound Warning > Buffer lock failure.  You may need to increase\n\tyour configured DSound buffer count.\n", stderr);
                continue;
            }
            oldp1 = p1;

            for (int p = 0; p < PacketsPerBuffer; p++, p1 += SndOutPacketSize)
                SndBuffer::ReadSamples(p1);

            if (g_setAudioData != nullptr)
                g_setAudioData(oldp1, BufferSizeBytes);

            buffer->Unlock(oldp1, s1, p2, s2);

            // Set the write pointer to the beginning of the next block.
            myLastWrite = (poffset + BufferSizeBytes) & ~BufferSizeBytes;
        }
        return 0;
    }

public:
    s32 Init()
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);

		m_NumBuffers = 8;

        //
        // Initialize DSound
        //
        GUID cGuid;

        try {
            if (m_Device.empty())
                throw std::runtime_error("screw it");

            if ((FAILED(IIDFromString(m_Device.c_str(), &cGuid))) ||
                FAILED(DirectSoundCreate8(&cGuid, &dsound, NULL)))
                throw std::runtime_error("try again?");
        } catch (std::runtime_error &) {
            // if the GUID failed, just open up the default dsound driver:
            if (FAILED(DirectSoundCreate8(NULL, &dsound, NULL)))
                throw std::runtime_error("DirectSound failed to initialize!");
        }

        if (FAILED(dsound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY)))
            throw std::runtime_error("DirectSound Error: Cooperative level could not be set.");

        // Determine the user's speaker configuration, and select an expansion option as needed.
        // FAIL : Directsound doesn't appear to support audio expansion >_<

        DWORD speakerConfig = 2;
        //dsound->GetSpeakerConfig( &speakerConfig );

        IDirectSoundBuffer *buffer_;
        DSBUFFERDESC desc;

        // Set up WAV format structure.

        memset(&wfx, 0, sizeof(WAVEFORMATEX));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nSamplesPerSec = SampleRate;
        wfx.nChannels = (WORD)speakerConfig;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = 2 * (WORD)speakerConfig;
        wfx.nAvgBytesPerSec = SampleRate * wfx.nBlockAlign;
        wfx.cbSize = 0;

        uint BufferSizeBytes = BufferSize * wfx.nBlockAlign;

        // Set up DSBUFFERDESC structure.

        memset(&desc, 0, sizeof(DSBUFFERDESC));
        desc.dwSize = sizeof(DSBUFFERDESC);
        desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;
        desc.dwBufferBytes = BufferSizeBytes * m_NumBuffers;
        desc.lpwfxFormat = &wfx;

        // Try a hardware buffer first, and then fall back on a software buffer if
        // that one fails.

        desc.dwFlags |= m_UseHardware ? DSBCAPS_LOCHARDWARE : DSBCAPS_LOCSOFTWARE;
        desc.dwFlags |= m_DisableGlobalFocus ? DSBCAPS_STICKYFOCUS : DSBCAPS_GLOBALFOCUS;

        if (FAILED(dsound->CreateSoundBuffer(&desc, &buffer_, 0))) {
            if (m_UseHardware) {
                desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
                desc.dwFlags |= m_DisableGlobalFocus ? DSBCAPS_STICKYFOCUS : DSBCAPS_GLOBALFOCUS;

                if (FAILED(dsound->CreateSoundBuffer(&desc, &buffer_, 0)))
                    throw std::runtime_error("DirectSound Error: Buffer could not be created.");
            }

            throw std::runtime_error("DirectSound Error: Buffer could not be created.");
        }
        if (FAILED(buffer_->QueryInterface(IID_IDirectSoundBuffer8, (void **)&buffer)) || buffer == NULL)
            throw std::runtime_error("DirectSound Error: Interface could not be queried.");

        buffer_->Release();
        verifyc(buffer->QueryInterface(IID_IDirectSoundNotify8, (void **)&buffer_notify));

        DSBPOSITIONNOTIFY lnot[MAX_BUFFER_COUNT];

        for (uint i = 0; i < m_NumBuffers; i++) {
            buffer_events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            lnot[i].dwOffset = (wfx.nBlockAlign + BufferSizeBytes * (i + 1)) % desc.dwBufferBytes;
            lnot[i].hEventNotify = buffer_events[i];
        }

        buffer_notify->SetNotificationPositions(m_NumBuffers, lnot);

        LPVOID p1 = 0, p2 = 0;
        DWORD s1 = 0, s2 = 0;

        verifyc(buffer->Lock(0, desc.dwBufferBytes, &p1, &s1, &p2, &s2, 0));
        assert(p2 == 0);
        memset(p1, 0, s1);
        verifyc(buffer->Unlock(p1, s1, p2, s2));

        //Play the buffer !
        verifyc(buffer->Play(0, 0, DSBPLAY_LOOPING));

        // Start Thread
        myLastWrite = 0;
        dsound_running = true;
        thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RThread<StereoOut16>, this, 0, &tid);
        SetThreadPriority(thread, THREAD_PRIORITY_ABOVE_NORMAL);
				
		setVolume(CurrentVolume);

        return 0;
    }

    void Close()
    {
        // Stop Thread
        fprintf(stderr, "* SPU2-X: Waiting for DSound thread to finish...");
        dsound_running = false;

        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);

        fprintf(stderr, " Done.\n");

        //
        // Clean up
        //
        if (buffer != NULL) {
            buffer->Stop();

            for (u32 i = 0; i < m_NumBuffers; i++) {
                if (buffer_events[i] != NULL)
                    CloseHandle(buffer_events[i]);
                buffer_events[i] = NULL;
            }

            safe_release(buffer_notify);
            safe_release(buffer);
        }

        safe_release(dsound);
        CoUninitialize();
    }

private:
    bool _DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext)
    {
        m_devices[ndevs].name = lpcstrDescription;

        if (lpGuid) {
            m_devices[ndevs].guid = *lpGuid;
            m_devices[ndevs].hasGuid = true;
        } else {
            m_devices[ndevs].hasGuid = false;
        }
        ndevs++;

        if (ndevs < 32)
            return TRUE;
        return FALSE;
    }

    bool _ConfigProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return TRUE;
    }

    static BOOL CALLBACK ConfigProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext);

public:
    virtual void Configure(uptr parent)
    {

    }

    s32 Test() const
    {
        return 0;
    }

    int GetEmptySampleCount()
    {
        DWORD play, write;
        buffer->GetCurrentPosition(&play, &write);

        // Note: Dsound's write cursor is bogus.  Use our own instead:

        int empty = play - myLastWrite;
        if (empty < 0)
            empty = -empty;

        return empty / 2;
    }

    const wchar_t *GetIdent() const
    {
        return L"dsound";
    }

    const wchar_t *GetLongName() const
    {
        return L"DirectSound (Nice)";
    }

    void ReadSettings()
    {
        m_Device = L"default";
        m_NumBuffers = 5;
        m_DisableGlobalFocus = false;
        m_UseHardware = false;

        Clampify(m_NumBuffers, (u8)3, (u8)8);
    }

    void SetApiSettings(wxString)
    {
    }

    void WriteSettings() const
    {
    }

    void setVolume(LONG aVolume)
    {
        HRESULT lres = DS_OK;

        CurrentVolume = aVolume;

        if (buffer != NULL)
            lres = IDirectSoundBuffer_SetVolume(buffer, aVolume);
    }

} static DS;

BOOL CALLBACK DSound::ConfigProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DS._ConfigProc(hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK DSound::DSEnumCallback(LPGUID lpGuid, LPCTSTR lpcstrDescription, LPCTSTR lpcstrModule, LPVOID lpContext)
{
    //pxAssume(DSoundOut != NULL);
    return DS._DSEnumCallback(lpGuid, lpcstrDescription, lpcstrModule, lpContext);
}

void DSSetVolume(LONG aVolume)
{
    DS.setVolume(aVolume);
}

SndOutModule *DSoundOut = &DS;
