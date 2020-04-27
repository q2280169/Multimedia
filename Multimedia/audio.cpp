
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

// 创建媒体会话的专用函数
void Audio::CreateMediaSession(HWND hWnd, LPCWSTR url)
{
	HRESULT hr;
	// 初始化 Media Foundation 平台
	hr = MFStartup(MF_VERSION);
	// 创建媒体会话
	hr = MFCreateMediaSession(NULL, &m_pSession);
	// 创建源解析器
	hr = MFCreateSourceResolver(&m_pSourceResolver);
	MF_OBJECT_TYPE  ObjectType = MF_OBJECT_INVALID;
	// 利用源解析器创建媒体源
	hr = m_pSourceResolver->CreateObjectFromURL(
		url,  // URL of the source.
		MF_RESOLUTION_MEDIASOURCE,			 // Create a source object.
		NULL,								 // Optional property store.
		&ObjectType,						 // Receives the created object type. 
		&pSource							 // Receives a pointer to the media source.
	);
	hr = pSource->QueryInterface(IID_PPV_ARGS(&m_pSource));
	// 创建媒体源呈现描述器
	hr = m_pSource->CreatePresentationDescriptor(&m_pSourcePD);
	// 创建新拓扑
	hr = MFCreateTopology(&m_pTopology);
	// 获取媒体源的流的数目
	hr = m_pSourcePD->GetStreamDescriptorCount(&cSourceStreams);
	// 为每一个流创建拓扑节点并添加到拓扑中
	for (DWORD i = 0; i < cSourceStreams; i++)
	{
		BOOL fSelected = FALSE;
		hr = m_pSourcePD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
		hr = pSD->GetMediaTypeHandler(&pHandler);
		GUID guidMajorType;
		hr = pHandler->GetMajorType(&guidMajorType);
		if (MFMediaType_Audio == guidMajorType)
		{
			// 创建音频渲染器
			hr = MFCreateAudioRendererActivate(&pActivate);
		}
		else if (MFMediaType_Video == guidMajorType)
		{
			// 创建视频渲染器
			hr = MFCreateVideoRendererActivate(hWnd, &pActivate);
		}
		// 创建拓扑的输入节点（即源节点）
		hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pSourceNode);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, m_pSource);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, m_pSourcePD);
		hr = pSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
		hr = m_pTopology->AddNode(pSourceNode);
		// 创建拓扑的输出节点
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);
		hr = pOutputNode->SetObject(pActivate);
		hr = pOutputNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
		hr = pOutputNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
		hr = m_pTopology->AddNode(pOutputNode);
		// 连接源节点与输出节点
		hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
	}
	// 将拓扑设置给 Media Session
	m_pSession->SetTopology(0, m_pTopology);
}

void Audio::TransCode()
{
	HRESULT hr = S_OK;
	HRESULT hr2 = S_OK;

	const LONG MAX_AUDIO_DURATION_MSEC = 246000; // 90秒
	
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
	IMFSourceReader* pReader,         // 源阅读器指针
	IMFSourceReader* pReaderAnother,
	HANDLE hFile,                     // 输出文件句柄
	HANDLE hFile2,
	LONG msecAudioData                // 写入文件的最大音频数据量，以毫秒计算
)
{
	HRESULT hr = S_OK;
	DWORD cbHeader = 0;					// WAVE 文件头部的大小，单位是字节
	DWORD cbAudioData = 0;				// 被写入文件的 PCM 音频数据的总字节数
	DWORD cbMaxAudioData = 0;
	IMFMediaType* pAudioType = NULL;    // 表示 PCM 音频格式

	HRESULT hr2 = S_OK;
	DWORD cbHeaderAnother = 0;
	DWORD cbAudioDataAnother = 0;
	DWORD cbMaxAudioDataAnother = 0;
	IMFMediaType* pAudioTypeAnother = NULL;

	// 配置源阅读器，使其从文件中获取压缩的 PCM 音频
	hr = ConfigureAudioStream(pReader, &pAudioType);
	hr2 = ConfigureAudioStream(pReaderAnother, &pAudioTypeAnother);

	// 写 WAVE 文件头部
	if (SUCCEEDED(hr))
	{
		hr = WriteWaveHeader(hFile, pAudioType, &cbHeader);
	}

	if (SUCCEEDED(hr2))
	{
		hr2 = WriteWaveHeader(hFile2, pAudioTypeAnother, &cbHeaderAnother);
	}

	// 计算最大解码音频数据量，以字节为单位
	if (SUCCEEDED(hr) && SUCCEEDED(hr2))
	{
		cbMaxAudioData = CalculateMaxAudioDataSize(pAudioType, cbHeader, msecAudioData);
		cbMaxAudioDataAnother = CalculateMaxAudioDataSize(pAudioTypeAnother, cbHeaderAnother, msecAudioData);
		// 解码音频数据
		hr = WriteWaveData(hFile, hFile2, pReader, pReaderAnother, cbMaxAudioData, cbMaxAudioDataAnother, &cbAudioData, &cbAudioDataAnother);
	}

	// 用实际数据填充 RIFF 头部
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
	IMFSourceReader* pReader,   // 源阅读器指针
	IMFMediaType** ppPCMAudio   // 用于接收音频格式信息
)
{
	HRESULT hr = S_OK;

	IMFMediaType* pUncompressedAudioType = NULL;
	IMFMediaType* pPartialType = NULL;

	// 创建媒体类型
	hr = MFCreateMediaType(&pPartialType);

	if (SUCCEEDED(hr))
	{
		hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);  //设置主媒体类型为音频
	}

	if (SUCCEEDED(hr))
	{
		hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);	  //设置子媒体类型为未压缩 PCM
	}

	// 将所确定的子类型设置到源阅读器，以便源阅读器加载对应的解码器
	if (SUCCEEDED(hr))
	{
		hr = pReader->SetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			NULL,
			pPartialType
		);
	}

	// 查询并获取源阅读器指定流的媒体类型
	if (SUCCEEDED(hr))
	{
		hr = pReader->GetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			&pUncompressedAudioType
		);
	}

	// 确保音频流被选择.
	if (SUCCEEDED(hr))
	{
		hr = pReader->SetStreamSelection(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			TRUE
		);
	}

	// 返回所完成的媒体类型设置
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
	HANDLE hFile,               // 输出文件句柄
	IMFMediaType* pMediaType,   // PCM 音频格式
	DWORD* pcbWritten           // 接收头部大小
)
{
	HRESULT hr = S_OK;
	UINT32 cbFormat = 0;
	WAVEFORMATEX* pWav = NULL;
	*pcbWritten = 0;

	// 将 PCM 音频格式传入 WAVEFORMATEX 结构体
	hr = MFCreateWaveFormatExFromMFMediaType(
		pMediaType,
		&pWav,
		&cbFormat
	);

	// 写 RIFF 头部以及 fmt 块的开始部分
	if (SUCCEEDED(hr))
	{
		DWORD header[] = {
			// RIFF 头部
			FCC('RIFF'),
			0,
			FCC('WAVE'),
			// fmt块的开始
			FCC('fmt '),
			cbFormat
		};

		DWORD dataHeader[] = { FCC('data'), 0 };

		hr = WriteToFile(hFile, header, sizeof(header));
		// 写 WAVEFORMATEX 结构体
		if (SUCCEEDED(hr))
		{
			hr = WriteToFile(hFile, pWav, cbFormat);

		}
		// 写 data 块的开始
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
	HANDLE hFile,						 // 输出文件1
	HANDLE hFile2,						 // 输出文件2
	IMFSourceReader* pReader,			 // 源阅读器
	IMFSourceReader* pReaderAnother,     // 第二个源阅读器
	DWORD cbMaxAudioData,				 // 最大音频数据量 (bytes).
	DWORD cbMaxAudioDataAnother,         // 最大音频数据量 (bytes).
	DWORD* pcbDataWritten,				 // 用于接收写入的音频数据量
	DWORD* pcbDataWrittenAnother         // 用于接收写入的音频数据量.
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

	// 从阅读器读取音频样本
	while (true)
	{
		DWORD dwFlags = 0;
		DWORD dwFlags2 = 0;

		// 读取下一个样本
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

		// 获取指向样本中音频数据的指针
		hr = pSample->ConvertToContiguousBuffer(&pBuffer);
		if (FAILED(hr)) { break; }
		hr2 = pSampleAnother->ConvertToContiguousBuffer(&pBufferAnother);
		if (FAILED(hr2)) { break; }

		// 锁定内存，得到缓冲地址指针
		hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);
		if (FAILED(hr)) { break; }
		hr2 = pBufferAnother->Lock(&pAudioDataAnother, NULL, &cbBufferAnother);
		if (FAILED(hr2)) { break; }

		// 确定没有超过所确定的最大数据量
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
			// 将左声道静音
			//if ((i - 0) % 4 == 0) *(pAudioData + i) = 0;  // *(pAudioDataAnother + i);
			//if ((i - 1) % 4 == 0) *(pAudioData + i) = 0;  // *(pAudioDataAnother + i);;
			// 将两首音频左右声道混合
			if ((i - 0) % 4 == 0)  *(pAudioData + i) = (*(pAudioDataAnother + i));
			if ((i - 1) % 4 == 0)  *(pAudioData + i) = (*(pAudioDataAnother + i));
			// 静音
			*(pAudioDataAnother + i) = 100;
		}

		// 向输出文件写数据
		hr = WriteToFile(hFile, pAudioData, cbBuffer);
		hr2 = WriteToFile(hFile2, pAudioDataAnother, cbBufferAnother);
		if (FAILED(hr)) { break; }
		if (FAILED(hr2)) { break; }

		// 解锁缓冲区
		hr = pBuffer->Unlock();
		pAudioData = NULL;
		hr2 = pBufferAnother->Unlock();
		pAudioDataAnother = NULL;

		if (FAILED(hr)) { break; }
		if (FAILED(hr2)) { break; }

		// 更新累加的音频数据总量
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
	IMFMediaType* pAudioType,    // PCM 音频格式
	DWORD cbHeader,              // WAVE 文件头部大小
	DWORD msecAudioData          // 最大音频数据持续时间，单位是毫秒
)
{
	UINT32 cbBlockSize = 0;         // 音频块大小，单位是字节
	UINT32 cbBytesPerSecond = 0;    // 每秒字节数

	// 从音频格式中获取音频（帧）大小、音频数据率（每秒字节数）
	cbBlockSize = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
	cbBytesPerSecond = MFGetAttributeUINT32(pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);

	// 计算最大输出音频数据量，该值等于音频持续时间乘以音频数据率。但不能超过 WAVE 文件所能容纳的最大音频数据量
	// 所需要的的音频片段数据量（字节）
	DWORD cbAudioClipSize = (DWORD)MulDiv(cbBytesPerSecond, msecAudioData, 1000);

	// 音频文件的最大可能数据量
	DWORD cbMaxSize = MAXDWORD - cbHeader;

	// 实际的最大数据量
	cbAudioClipSize = min(cbAudioClipSize, cbMaxSize);

	// 对音频块大小进行取整运算，避免写入不完整的音频块
	cbAudioClipSize = (cbAudioClipSize / cbBlockSize) * cbBlockSize;

	return cbAudioClipSize;
}

HRESULT Audio::FixUpChunkSizes(
	HANDLE hFile,           // 输出文件
	DWORD cbHeader,         // fmt 块大小
	DWORD cbAudioData       // data 块大小
)
{
	HRESULT hr = S_OK;
	LARGE_INTEGER ll;
	ll.QuadPart = cbHeader - sizeof(DWORD);

	if (0 == SetFilePointerEx(hFile, ll, NULL, FILE_BEGIN))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	// 写数据大小
	if (SUCCEEDED(hr))
	{
		hr = WriteToFile(hFile, &cbAudioData, sizeof(cbAudioData));
	}

	if (SUCCEEDED(hr))
	{
		// 写文件大小
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