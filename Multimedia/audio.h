#pragma once
#include "framework.h"
#include <mfidl.h>	
#include <mfapi.h>
#include <mfreadwrite.h>
#include <stdio.h>

class Audio
{
	private:
		IMFMediaSession* m_pSession;
		IMFSourceResolver* m_pSourceResolver;
		IUnknown* pSource;
		IMFMediaSource* m_pSource;
		IMFTopology* m_pTopology;
		IMFPresentationDescriptor* m_pSourcePD;
		DWORD cSourceStreams = 0;
		IMFTopologyNode* pSourceNode;
		IMFTopologyNode* pOutputNode;
		IMFActivate* pSinkActivate;
		IMFStreamDescriptor* pSD;
		IMFMediaTypeHandler* pHandler;
		IMFActivate* pActivate;
		PROPVARIANT	var;

		HRESULT WriteWaveFile(IMFSourceReader*, IMFSourceReader*, HANDLE, HANDLE, LONG);
		HRESULT ConfigureAudioStream(IMFSourceReader*, IMFMediaType**);
		HRESULT WriteWaveHeader(HANDLE, IMFMediaType*, DWORD*);
		DWORD   CalculateMaxAudioDataSize(IMFMediaType*, DWORD, DWORD);
		HRESULT WriteWaveData(HANDLE, HANDLE, IMFSourceReader*, IMFSourceReader*, DWORD, DWORD, DWORD*, DWORD*);
		HRESULT FixUpChunkSizes(HANDLE, DWORD, DWORD);
		HRESULT WriteToFile(HANDLE, void*, DWORD);
		template <class T> void SafeRelease(T** ppT);

	public:
		Audio();
		void CreateMediaSession(HWND hWnd, LPCWSTR url);
		void play();
		void pause();
		void stop();
		void TransCode();
};

