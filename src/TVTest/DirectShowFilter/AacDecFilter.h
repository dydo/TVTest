#pragma once

#include "TransFrm.h"
#include "MediaData.h"
#include "TsMedia.h"
#include "AacDecoder.h"


// �e���v���[�g��
#define AACDECFILTER_NAME	(L"AAC Decoder Filter")


// ���̃t�B���^��GUID {8D1E3E25-D92B-4849-8D38-C787DA78352C}
DEFINE_GUID(CLSID_AACDECFILTER, 0x8d1e3e25, 0xd92b, 0x4849, 0x8d, 0x38, 0xc7, 0x87, 0xda, 0x78, 0x35, 0x2c);


// �t�B���^���̊O���Q�Ɛ錾
extern const AMOVIESETUP_FILTER g_AacDecFilterInfo;


class CAacDecFilter :	public CTransformFilter,
						protected CAacDecoder::IPcmHandler
{

public:
	DECLARE_IUNKNOWN

	CAacDecFilter(LPUNKNOWN pUnk, HRESULT *phr);
	~CAacDecFilter(void);
	static IBaseFilter* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, CAacDecFilter **ppClassIf);

// CTransformFilter
	HRESULT CheckInputType(const CMediaType* mtIn);
	HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
	HRESULT DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pprop);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT StartStreaming(void);
	HRESULT StopStreaming(void);
	//HRESULT Receive(IMediaSample *pSample);

// CAacDecFilter
	const BYTE GetCurrentChannelNum();

	// Append by HDUSTest�̒��̐l
	bool SetNormalize(bool bNormalize,float Level=1.0f);
protected:
	CCritSec m_cStateLock;
	HRESULT Transform(IMediaSample *pIn, IMediaSample *pOut);

// CAacDecoder::IPcmHandler
	virtual void OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel);
	
// CAacDecFilter
	CAdtsParser m_AdtsParser;
	CAacDecoder m_AacDecoder;
	IMediaSample *m_pOutSample;
	BYTE m_byCurChannelNum;
	
private:
	static const DWORD DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples);

	// Append by HDUSTest�̒��̐l
	bool m_bNormalize;
	float m_NormalizeLevel;
	void Normalize(short *pBuffer,DWORD Samples);
};