
#include "audio.h"

template <class T> void Audio::SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

Audio::Audio()
{
	
};

void Audio::play()
{
	m_pSession->Start(NULL, &var);
}

void Audio::pause()
{
	m_pSession->Pause();
}

void Audio::stop()
{
	m_pSession->Stop();
}

// ����ý��Ự��ר�ú���
void Audio::CreateMediaSession(HWND hWnd, LPCWSTR url)
{
	HRESULT hr;
	// ��ʼ�� Media Foundation ƽ̨
	hr = MFStartup(MF_VERSION);
	// ����ý��Ự
	hr = MFCreateMediaSession(NULL, &m_pSession);
	// ����Դ������
	hr = MFCreateSourceResolver(&m_pSourceResolver);
	MF_OBJECT_TYPE  ObjectType = MF_OBJECT_INVALID;
	// ����Դ����������ý��Դ
	hr = m_pSourceResolver->CreateObjectFromURL(
		url,  // URL of the source.
		MF_RESOLUTION_MEDIASOURCE,			 // Create a source object.
		NULL,								 // Optional property store.
		&ObjectType,						 // Receives the created object type. 
		&pSource							 // Receives a pointer to the media source.
	);
	hr = pSource->QueryInterface(IID_PPV_ARGS(&m_pSource));
	// ����ý��Դ����������
	hr = m_pSource->CreatePresentationDescriptor(&m_pSourcePD);
	// ����������
	hr = MFCreateTopology(&m_pTopology);
	// ��ȡý��Դ��������Ŀ
	hr = m_pSourcePD->GetStreamDescriptorCount(&cSourceStreams);
	// Ϊÿһ�����������˽ڵ㲢��ӵ�������
	for (DWORD i = 0; i < cSourceStreams; i++)
	{
		BOOL fSelected = FALSE;
		hr = m_pSourcePD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
		hr = pSD->GetMediaTypeHandler(&pHandler);
		GUID guidMajorType;
		hr = pHandler->GetMajorType(&guidMajorType);
		if (MFMediaType_Audio == guidMajorType)
		{
			// ������Ƶ��Ⱦ��
			hr = MFCreateAudioRendererActivate(&pActivate);
		}
		else if (MFMediaType_Video == guidMajorType)
		{
			// ������Ƶ��Ⱦ��
			hr = MFCreateVideoRendererActivate(hWnd, &pActivate);
		}
		// �������˵�����ڵ㣨��Դ�ڵ㣩
		hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pSourceNode);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_pSource);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_pSourcePD);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
		hr = m_pTopology->AddNode(pSourceNode);
		// �������˵�����ڵ�
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);
		hr = pOutputNode->SetObject(pActivate);
		hr = pOutputNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
		hr = pOutputNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
		hr = m_pTopology->AddNode(pOutputNode);
		// ����Դ�ڵ�������ڵ�
		hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
	}
	// ���������ø� Media Session
	m_pSession->SetTopology(0, m_pTopology);
}

void Audio::TransCode()
{
	HRESULT hr = S_OK;
	HRESULT hr2 = S_OK;

	const LONG MAX_AUDIO_DURATION_MSEC = 246000; // 90��
	
	const WCHAR* wszTargetFile = L"out1.wav";
	IMFSourceReader* pReader = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	const WCHAR* wszTargetFile2 = L"out2.wav";
	IMFSourceReader* pReaderAnother = NULL;
	HANDLE hFile2 = INVALID_HANDLE_VALUE;

	// Create the source reader to read the input file.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateSourceReaderFromURL(
			L"HuoHua.wav",
			NULL,
			&pReader
		);

		if (FAILED(hr))
		{
			return;
		}
	}

	// Create another source reader to read another input file which will be the background sound.
	if (SUCCEEDED(hr2))
	{
		hr2 = MFCreateSourceReaderFromURL(
			L"TheClassic.wav",
			NULL,
			&pReaderAnother
		);

		if (FAILED(hr2))
		{
			return;
		}
	}

	// open the output wave file to which we write decoded data
	if (SUCCEEDED(hr))
	{
		hFile = CreateFile(
			wszTargetFile,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
		);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			return;
		}
	}

	// open another output wave file to which we write decoded data
	if (SUCCEEDED(hr2))
	{
		hFile2 = CreateFile(
			wszTargetFile2,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
		);

		if (hFile2 == INVALID_HANDLE_VALUE)
		{
			hr2 = HRESULT_FROM_WIN32(GetLastError());
			return;
		}
	}

	// write wave file
	if (SUCCEEDED(hr))
	{
		hr = WriteWaveFile(pReader, pReaderAnother, hFile, hFile2, MAX_AUDIO_DURATION_MSEC);
	}

	if (FAILED(hr))
	{
		return;
	}
	// close file1
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}
	// close file2
	if (hFile2 != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile2);
	}
	SafeRelease(&pReader);
	SafeRelease(&pReaderAnother);
}


HRESULT Audio::WriteWaveFile(
	IMFSourceReader* pReader,         // Դ�Ķ���ָ��
	IMFSourceReader* pReaderAnother,
	HANDLE hFile,                     // ����ļ����
	HANDLE hFile2,
	LONG msecAudioData                // д���ļ��������Ƶ���������Ժ������
)
{
	HRESULT hr = S_OK;
	DWORD cbHeader = 0;					// WAVE �ļ�ͷ���Ĵ�С����λ���ֽ�
	DWORD cbAudioData = 0;				// ��д���ļ��� PCM ��Ƶ���ݵ����ֽ���
	DWORD cbMaxAudioData = 0;
	IMFMediaType* pAudioType = NULL;    // ��ʾ PCM ��Ƶ��ʽ

	HRESULT hr2 = S_OK;
	DWORD cbHeaderAnother = 0;
	DWORD cbAudioDataAnother = 0;
	DWORD cbMaxAudioDataAnother = 0;
	IMFMediaType* pAudioTypeAnother = NULL;

	// ����Դ�Ķ�����ʹ����ļ��л�ȡѹ���� PCM ��Ƶ
	hr = ConfigureAudioStream(pReader, &pAudioType);
	hr2 = ConfigureAudioStream(pReaderAnother, &pAudioTypeAnother);

	// д WAVE �ļ�ͷ��
	if (SUCCEEDED(hr))
	{
		hr = WriteWaveHeader(hFile, pAudioType, &cbHeader);
	}

	if (SUCCEEDED(hr2))
	{
		hr2 = WriteWaveHeader(hFile2, pAudioTypeAnother, &cbHeaderAnother);
	}

	// ������������Ƶ�����������ֽ�Ϊ��λ
	if (SUCCEEDED(hr) && SUCCEEDED(hr2))
	{
		cbMaxAudioData = CalculateMaxAudioDataSize(pAudioType, cbHeader, msecAudioData);
		cbMaxAudioDataAnother = CalculateMaxAudioDataSize(pAudioTypeAnother, cbHeaderAnother, msecAudioData);
		// ������Ƶ����
		hr = WriteWaveData(hFile, hFile2, pReader, pReaderAnother, cbMaxAudioData, cbMaxAudioDataAnother, &cbAudioData, &cbAudioDataAnother);
	}

	// ��ʵ��������� RIFF ͷ��
	if (SUCCEEDED(hr))
	{
		hr = FixUpChunkSizes(hFile, cbHeader, cbAudioData);
	}

	if (SUCCEEDED(hr2))
	{
		hr2 = FixUpChunkSizes(hFile2, cbHeaderAnother, cbAudioDataAnother);
	}

	SafeRelease(&pAudioType);
	SafeRelease(&pAudioTypeAnother);
	return hr;
}

HRESULT Audio::ConfigureAudioStream(
	IMFSourceReader* pReader,   // Դ�Ķ���ָ��
	IMFMediaType** ppPCMAudio   // ���ڽ�����Ƶ��ʽ��Ϣ
)
{
	HRESULT hr = S_OK;

	IMFMediaType* pUncompressedAudioType = NULL;
	IMFMediaType* pPartialType = NULL;

	// ����ý������
	hr = MFCreateMediaType(&pPartialType);

	if (SUCCEEDED(hr))
	{
		hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);  //������ý������Ϊ��Ƶ
	}

	if (SUCCEEDED(hr))
	{
		hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);	  //������ý������Ϊδѹ�� PCM
	}

	// ����ȷ�������������õ�Դ�Ķ������Ա�Դ�Ķ������ض�Ӧ�Ľ�����
	if (SUCCEEDED(hr))
	{
		hr = pReader->SetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			NULL,
			pPartialType
		);
	}

	// ��ѯ����ȡԴ�Ķ���ָ������ý������
	if (SUCCEEDED(hr))
	{
		hr = pReader->GetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			&pUncompressedAudioType
		);
	}

	// ȷ����Ƶ����ѡ��.
	if (SUCCEEDED(hr))
	{
		hr = pReader->SetStreamSelection(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			TRUE
		);
	}

	// ��������ɵ�ý����������
	if (SUCCEEDED(hr))
	{
		*ppPCMAudio = pUncompressedAudioType;
		(*ppPCMAudio)->AddRef();
	}

	SafeRelease(&pUncompressedAudioType);
	SafeRelease(&pPartialType);
	return hr;
}

HRESULT Audio:: WriteWaveHeader(
	HANDLE hFile,               // ����ļ����
	IMFMediaType* pMediaType,   // PCM ��Ƶ��ʽ
	DWORD* pcbWritten           // ����ͷ����С
)
{
	HRESULT hr = S_OK;
	UINT32 cbFormat = 0;
	WAVEFORMATEX* pWav = NULL;
	*pcbWritten = 0;

	// �� PCM ��Ƶ��ʽ���� WAVEFORMATEX �ṹ��
	hr = MFCreateWaveFormatExFromMFMediaType(
		pMediaType,
		&pWav,
		&cbFormat
	);

	// д RIFF ͷ���Լ� fmt ��Ŀ�ʼ����
	if (SUCCEEDED(hr))
	{
		DWORD header[] = {
			// RIFF ͷ��
			FCC('RIFF'),
			0,
			FCC('WAVE'),
			// fmt��Ŀ�ʼ
			FCC('fmt '),
			cbFormat
		};

		DWORD dataHeader[] = { FCC('data'), 0 };

		hr = WriteToFile(hFile, header, sizeof(header));
		// д WAVEFORMATEX �ṹ��
		if (SUCCEEDED(hr))
		{
			hr = WriteToFile(hFile, pWav, cbFormat);

		}
		// д data ��Ŀ�ʼ
		if (SUCCEEDED(hr))
		{
			hr = WriteToFile(hFile, dataHeader, sizeof(dataHeader));
		}

		if (SUCCEEDED(hr))
		{
			*pcbWritten = sizeof(header) + cbFormat + sizeof(dataHeader);
		}
	}
	CoTaskMemFree(pWav);
	return hr;
}

HRESULT Audio::WriteWaveData(
	HANDLE hFile,						 // ����ļ�1
	HANDLE hFile2,						 // ����ļ�2
	IMFSourceReader* pReader,			 // Դ�Ķ���
	IMFSourceReader* pReaderAnother,     // �ڶ���Դ�Ķ���
	DWORD cbMaxAudioData,				 // �����Ƶ������ (bytes).
	DWORD cbMaxAudioDataAnother,         // �����Ƶ������ (bytes).
	DWORD* pcbDataWritten,				 // ���ڽ���д�����Ƶ������
	DWORD* pcbDataWrittenAnother         // ���ڽ���д�����Ƶ������.
)
{
	int i;
	HRESULT hr = S_OK;
	DWORD cbAudioData = 0;
	DWORD cbBuffer = 0;
	BYTE* pAudioData = NULL;
	IMFSample* pSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;

	HRESULT hr2 = S_OK;
	DWORD cbAudioDataAnother = 0;
	DWORD cbBufferAnother = 0;
	BYTE* pAudioDataAnother = NULL;
	IMFSample* pSampleAnother = NULL;
	IMFMediaBuffer* pBufferAnother = NULL;

	// ���Ķ�����ȡ��Ƶ����
	while (true)
	{
		DWORD dwFlags = 0;
		DWORD dwFlags2 = 0;

		// ��ȡ��һ������
		hr = pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			NULL,
			&dwFlags,
			NULL,
			&pSample
		);

		hr2 = pReaderAnother->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			NULL,
			&dwFlags2,
			NULL,
			&pSampleAnother
		);

		if (FAILED(hr)) { break; }
		if (FAILED(hr2)) { break; }

		if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
		{
			printf("Type change - not supported by WAVE file format.\n");
			break;
		}
		if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			MessageBox(NULL, L"End of input file", L"message", NULL);//printf("End of input file.\n");
			break;
		}

		if (pSample == NULL)
		{
			printf("No sample\n");
			continue;
		}

		if (pSampleAnother == NULL)
		{
			printf("No sample\n");
			continue;
		}

		// ��ȡָ����������Ƶ���ݵ�ָ��
		hr = pSample->ConvertToContiguousBuffer(&pBuffer);
		if (FAILED(hr)) { break; }
		hr2 = pSampleAnother->ConvertToContiguousBuffer(&pBufferAnother);
		if (FAILED(hr2)) { break; }

		// �����ڴ棬�õ������ַָ��
		hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);
		if (FAILED(hr)) { break; }
		hr2 = pBufferAnother->Lock(&pAudioDataAnother, NULL, &cbBufferAnother);
		if (FAILED(hr2)) { break; }

		// ȷ��û�г�����ȷ�������������
		if (cbMaxAudioData - cbAudioData < cbBuffer)
		{
			cbBuffer = cbMaxAudioData - cbAudioData;
		}
		if (cbMaxAudioDataAnother - cbAudioDataAnother < cbBufferAnother)
		{
			cbBufferAnother = cbMaxAudioDataAnother - cbAudioDataAnother;
		}

		for (int i = 0; i < cbBuffer; i++)
		{
			// ������������
			//if ((i - 0) % 4 == 0) *(pAudioData + i) = 0;  // *(pAudioDataAnother + i);
			//if ((i - 1) % 4 == 0) *(pAudioData + i) = 0;  // *(pAudioDataAnother + i);;
			// ��������Ƶ�����������
			if ((i - 0) % 4 == 0)  *(pAudioData + i) = (*(pAudioDataAnother + i));
			if ((i - 1) % 4 == 0)  *(pAudioData + i) = (*(pAudioDataAnother + i));
			// ����
			*(pAudioDataAnother + i) = 100;
		}

		// ������ļ�д����
		hr = WriteToFile(hFile, pAudioData, cbBuffer);
		hr2 = WriteToFile(hFile2, pAudioDataAnother, cbBufferAnother);
		if (FAILED(hr)) { break; }
		if (FAILED(hr2)) { break; }

		// ����������
		hr = pBuffer->Unlock();
		pAudioData = NULL;
		hr2 = pBufferAnother->Unlock();
		pAudioDataAnother = NULL;

		if (FAILED(hr)) { break; }
		if (FAILED(hr2)) { break; }

		// �����ۼӵ���Ƶ��������
		cbAudioData += cbBuffer;
		cbAudioDataAnother += cbBufferAnother;

		if (cbAudioData >= cbMaxAudioData)
		{
			break;
		}
		if (cbAudioDataAnother >= cbMaxAudioDataAnother)
		{
			break;
		}

	}

	SafeRelease(&pSample);
	SafeRelease(&pBuffer);
	SafeRelease(&pSampleAnother);
	SafeRelease(&pBufferAnother);

	if (SUCCEEDED(hr))
	{
		MessageBox(NULL, L"success to write file1", L"message", NULL);

		*pcbDataWritten = cbAudioData;
	}
	if (SUCCEEDED(hr2))
	{
		MessageBox(NULL, L"success to write file2", L"message", NULL);

		*pcbDataWrittenAnother = cbAudioDataAnother;
	}

	if (pAudioData)
	{
		pBuffer->Unlock();
	}
	if (pAudioDataAnother)
	{
		pBufferAnother->Unlock();
	}
	SafeRelease(&pBuffer);
	SafeRelease(&pSample);
	SafeRelease(&pBufferAnother);
	SafeRelease(&pSampleAnother);
	return hr;
}

DWORD Audio::CalculateMaxAudioDataSize(
	IMFMediaType* pAudioType,    // PCM ��Ƶ��ʽ
	DWORD cbHeader,              // WAVE �ļ�ͷ����С
	DWORD msecAudioData          // �����Ƶ���ݳ���ʱ�䣬��λ�Ǻ���
)
{
	UINT32 cbBlockSize = 0;         // ��Ƶ���С����λ���ֽ�
	UINT32 cbBytesPerSecond = 0;    // ÿ���ֽ���

	// ����Ƶ��ʽ�л�ȡ��Ƶ��֡����С����Ƶ�����ʣ�ÿ���ֽ�����
	cbBlockSize = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
	cbBytesPerSecond = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);

	// ������������Ƶ����������ֵ������Ƶ����ʱ�������Ƶ�����ʡ������ܳ��� WAVE �ļ��������ɵ������Ƶ������
	// ����Ҫ�ĵ���ƵƬ�����������ֽڣ�
	DWORD cbAudioClipSize = (DWORD)MulDiv(cbBytesPerSecond, msecAudioData, 1000);

	// ��Ƶ�ļ���������������
	DWORD cbMaxSize = MAXDWORD - cbHeader;

	// ʵ�ʵ����������
	cbAudioClipSize = min(cbAudioClipSize, cbMaxSize);

	// ����Ƶ���С����ȡ�����㣬����д�벻��������Ƶ��
	cbAudioClipSize = (cbAudioClipSize / cbBlockSize) * cbBlockSize;

	return cbAudioClipSize;
}

HRESULT Audio::FixUpChunkSizes(
	HANDLE hFile,           // ����ļ�
	DWORD cbHeader,         // fmt ���С
	DWORD cbAudioData       // data ���С
)
{
	HRESULT hr = S_OK;
	LARGE_INTEGER ll;
	ll.QuadPart = cbHeader - sizeof(DWORD);

	if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	// д���ݴ�С
	if (SUCCEEDED(hr))
	{
		hr = WriteToFile(hFile, &cbAudioData, sizeof(cbAudioData));
	}

	if (SUCCEEDED(hr))
	{
		// д�ļ���С
		ll.QuadPart = sizeof(DWORD);

		if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	if (SUCCEEDED(hr))
	{
		DWORD cbRiffFileSize = cbHeader + cbAudioData - 8;

		// NOTE: The "size" field in the RIFF header does not include
		// the first 8 bytes of the file. i.e., it is the size of the
		// data that appears _after_ the size field.

		hr = WriteToFile(hFile, &cbRiffFileSize, sizeof(cbRiffFileSize));
	}

	return hr;
}

HRESULT Audio::WriteToFile(HANDLE hFile, void* p, DWORD cb)
{
	DWORD cbWritten = 0;
	HRESULT hr = S_OK;

	BOOL bResult = WriteFile(hFile, p, cb, &cbWritten, NULL);
	if (!bResult)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}