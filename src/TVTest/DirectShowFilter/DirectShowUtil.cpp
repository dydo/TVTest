#include "stdafx.h"
#include "DirectShowUtil.h"

#ifdef USE_MEDIA_FOUNDATION
#pragma comment(lib,"mf.lib")
#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfuuid.lib")
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




CDirectShowFilterFinder::CDirectShowFilterFinder()
{
	::CoInitialize(NULL);
}


CDirectShowFilterFinder::~CDirectShowFilterFinder()
{
	::CoUninitialize();
}


void CDirectShowFilterFinder::Clear()
{
	m_FilterList.clear();
}


int CDirectShowFilterFinder::GetFilterCount()
{
	return m_FilterList.size();
}


bool CDirectShowFilterFinder::GetFilterInfo(const int iIndex,CLSID *pidClass,LPWSTR pwszFriendryName,int iBufLen)
{
	if (iIndex<0 || iIndex>=GetFilterCount())
		return false;
	CFilterInfo &Info = m_FilterList[iIndex];
	if (pidClass)
		*pidClass = Info.m_clsid;
	if (pwszFriendryName) {
		if (Info.m_pwszFriendryName) {
			::lstrcpynW(pwszFriendryName,Info.m_pwszFriendryName,iBufLen);
		} else if (iBufLen>0) {
			pwszFriendryName[0]='\0';
		}
	}
	return true;
}


bool CDirectShowFilterFinder::FindFilter(const CLSID *pidInType,const CLSID *pidInSubType,const CLSID *pidOutType,const CLSID *pidOutSubType)
{
	// idType idSubType �̃t�B���^����������(�ϊ��t�B���^)
	bool bRet = false;
	IFilterMapper2 *pMapper=NULL;
	HRESULT hr=CoCreateInstance(CLSID_FilterMapper2,NULL, CLSCTX_INPROC, IID_IFilterMapper2,(void **) &pMapper);

	if(SUCCEEDED(hr)) {
		IEnumMoniker *pEnum=NULL;
		GUID arInType[2],arOutType[2];
		GUID *pInTypes,*pOutTypes;
		if(pidInType)    arInType[0] = *pidInType;
		if(pidInSubType) arInType[1] = *pidInSubType;
		pInTypes = (pidInType && pidInSubType) ? arInType : NULL;
		if(pidOutType)   arOutType[0] = *pidOutType;
		if(pidOutSubType)arOutType[1] = *pidOutSubType;
		pOutTypes = (pidOutType && pidOutSubType) ? arOutType : NULL;

		hr = pMapper->EnumMatchingFilters(
			&pEnum,
			0,					// �\��ς�
			TRUE,				// ���S��v���g�p���邩
			MERIT_DO_NOT_USE+1,	// �ŏ��̃����b�g
			TRUE,				// 1 �ȏ�̓��̓s����?
			pInTypes?1:0,		// ���͂̃��W���[�^�C�v/�T�u�^�C�v�̑΂̐�
			pInTypes,			// ���͂̃��W���[�^�C�v/�T�u�^�C�v�̑΂̔z��
			NULL,				// ���̓��f�B�A
			NULL,				// ���̓s���̃J�e�S��
			FALSE,				// �����_���łȂ���΂Ȃ�Ȃ���
			TRUE,				// 1 �ȏ�̏o�̓s����
			pOutTypes?1:0,		// �o�͂̃��W���[�^�C�v/�T�u�^�C�v�̑΂̐�
			pOutTypes,			// �o�͂̃��W���[�^�C�v/�T�u�^�C�v�̑΂̔z��
			NULL,				// �o�̓��f�B�A
			NULL);				// �o�̓s���̃J�e�S��
		if(SUCCEEDED(hr)){
			IMoniker *pMoniker;
			ULONG cFetched;
			while(pEnum->Next(1, &pMoniker, &cFetched) == S_OK){
				IPropertyBag *pPropBag;
				hr=pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
				if(SUCCEEDED(hr)){
					VARIANT varName,varID;
					VariantInit(&varName);
					VariantInit(&varID);
					hr=pPropBag->Read(L"FriendlyName", &varName, 0);
					if(SUCCEEDED(hr)){
						hr=pPropBag->Read(L"CLSID", &varID, 0);
						if(SUCCEEDED(hr)){
							bRet = true;
							CFilterInfo FilterInfo;
							FilterInfo.SetFriendryName(varName.bstrVal);
							CLSIDFromString(varID.bstrVal,&FilterInfo.m_clsid);
							m_FilterList.push_back(FilterInfo);
							SysFreeString(varID.bstrVal);
							}
							SysFreeString(varName.bstrVal);
						}
						pPropBag->Release();
					}
				}
				pEnum->Release();
			}
			pMapper->Release();
		}
	return bRet;
}


// �D�悷��t�B���^�����X�g��[�Ɏ����Ă���
bool CDirectShowFilterFinder::PriorityFilterGoToHead(const CLSID idPriorityClass)
{
	std::vector<CFilterInfo> TmpList;
	size_t i;

	for (i=0;i<m_FilterList.size();i++) {
		if (m_FilterList[i].m_clsid == idPriorityClass) {
			// �D�悷����̂𔭌�
			TmpList.push_back(m_FilterList[i]);
		}
	}
	if (!TmpList.empty()) {
		for (i=0;i<m_FilterList.size();i++) {
			if(m_FilterList[i].m_clsid != idPriorityClass) {
				// �D�悷����̈ȊO
				TmpList.push_back(m_FilterList[i]);
			}
		}
	}
	m_FilterList=TmpList;
	return true;
}


// ��������t�B���^�����X�g�I�[�Ɏ����Ă���
bool CDirectShowFilterFinder::IgnoreFilterGoToTail(const CLSID idIgnoreClass,bool bRemoveIt)
{
	std::vector<CFilterInfo> TmpList;
	size_t i;

	for (i=0;i<m_FilterList.size();i++) {
		if (m_FilterList[i].m_clsid != idIgnoreClass) {
			// ����������̈ȊO
			TmpList.push_back(m_FilterList[i]);
		}
	}
	if (!bRemoveIt) {
		for (i=0;i<m_FilterList.size();i++) {
			if (m_FilterList[i].m_clsid == idIgnoreClass) {
				TmpList.push_back(m_FilterList[i]);
			}
		}
	}
	m_FilterList=TmpList;
	return true;
}




CDirectShowFilterFinder::CFilterInfo::CFilterInfo()
	: m_pwszFriendryName(NULL)
{
}


CDirectShowFilterFinder::CFilterInfo::CFilterInfo(const CFilterInfo &Info)
	: m_pwszFriendryName(NULL)
{
	*this=Info;
}


CDirectShowFilterFinder::CFilterInfo::~CFilterInfo()
{
	delete m_pwszFriendryName;
}


CDirectShowFilterFinder::CFilterInfo &CDirectShowFilterFinder::CFilterInfo::operator=(const CFilterInfo &Info)
{
	SetFriendryName(Info.m_pwszFriendryName);
	m_clsid=Info.m_clsid;
	return *this;
}


void CDirectShowFilterFinder::CFilterInfo::SetFriendryName(LPCWSTR pwszFriendryName)
{
	if (m_pwszFriendryName) {
		delete [] m_pwszFriendryName;
		m_pwszFriendryName=NULL;
	}
	if (pwszFriendryName) {
		m_pwszFriendryName=new WCHAR[::lstrlenW(pwszFriendryName)+1];
		::lstrcpyW(m_pwszFriendryName,pwszFriendryName);
	}
}




HRESULT DirectShowUtil::AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;

	if (FAILED(::GetRunningObjectTable(0, &pROT)))
		return E_FAIL;

	wchar_t wsz[256];
	swprintf_s(wsz,256, L"FilterGraph %08p pid %08x", (DWORD_PTR)pUnkGraph, ::GetCurrentProcessId());

	HRESULT hr = ::CreateItemMoniker(L"!", wsz, &pMoniker);

	if (SUCCEEDED(hr)) {
		hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
		pMoniker->Release();
	}
	pROT->Release();
	return hr;
}


void DirectShowUtil::RemoveFromRot(const DWORD dwRegister)
{
	IRunningObjectTable *pROT;

	if (SUCCEEDED(::GetRunningObjectTable(0, &pROT))) {
		pROT->Revoke(dwRegister);
		pROT->Release();
	}
}


// �t�B���^����w�胁�f�B�A�̃s������������
IPin* DirectShowUtil::GetFilterPin(IBaseFilter *pFilter, const PIN_DIRECTION dir, const AM_MEDIA_TYPE *pMediaType)
{
	IEnumPins *pEnumPins=NULL;
	IPin *pPin;
	IPin *pRetPin=NULL;

	if(pFilter->EnumPins(&pEnumPins)==S_OK ){
		ULONG cFetched;
		while(!pRetPin&&pEnumPins->Next(1,&pPin,&cFetched)==S_OK){
			PIN_INFO stPin;
			if(pPin->QueryPinInfo(&stPin)==S_OK){
				if(stPin.dir==dir){
					if(!pMediaType){
						// �������������Ă����OK
						pRetPin=pPin;
						pRetPin->AddRef();
						} else {
						// DirectShow�ɂ܂����ăs��������
						if(pPin->QueryAccept(pMediaType)==S_OK){
							pRetPin=pPin;
							pRetPin->AddRef();
							}
						}
					}
					if(stPin.pFilter) stPin.pFilter->Release();
				}
				pPin->Release();
			}
		pEnumPins->Release();
		}
	return pRetPin;
}

// �t�B���^�̃v���p�e�B�y�[�W���J��
bool DirectShowUtil::ShowPropertyPage(IBaseFilter *pFilter, HWND hWndParent)
{
	if (!pFilter)
		return false;

	bool bRet = false;
	ISpecifyPropertyPages *pProp = NULL;

	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if(SUCCEEDED(hr)){
		CAUUID caGUID;
		FILTER_INFO stFilterInfo;

		pProp->GetPages(&caGUID);
		SAFE_RELEASE(pProp);
		hr = pFilter->QueryFilterInfo(&stFilterInfo);
		if(SUCCEEDED(hr)){
			IUnknown *pFilterUnk=NULL;

			hr = pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);
			if(SUCCEEDED(hr)){
				OleCreatePropertyFrame(
					hWndParent,             // �e�E�B���h�E�B
					0, 0,                   // �\��ς݁B
					stFilterInfo.achName,   // �_�C�A���O �{�b�N�X�̃L���v�V�����B
					1,                      // �I�u�W�F�N�g�� (�t�B���^�̂�)�B
					&pFilterUnk,            // �I�u�W�F�N�g �|�C���^�̔z��B
					caGUID.cElems,          // �v���p�e�B �y�[�W���B
					caGUID.pElems,          // �v���p�e�B �y�[�W CLSID �̔z��B
					0,                      // ���P�[�����ʎq�B
					0, NULL                 // �\��ς݁B
				);
				CoTaskMemFree(caGUID.pElems);
				SAFE_RELEASE(pFilterUnk);
				bRet = true;
			}
			SAFE_RELEASE(stFilterInfo.pGraph);
		}
	}
	return bRet;
}


#if 0
// Mpeg�f�R�[�_��ǉ����ăs���ڑ����s���B
// pUtil�ɐ���ς݃f�R�[�_�񋓂����Ă����Ƃ��̐��񏇂ɐڑ����s����BNULL�Ȃ�f�t�H���g���B
//   �킴�킴�w�肷�闝�R : ���̃N���X�Ɍ������ꂽ�t�B���^��Mpeg2�̂��̂Ƃ͌���Ȃ��ׁB
//
// ppMpeg2DecoderFilter �͐ڑ��Ɏg��ꂽMpeg2�C���^�[�t�F�[�X���󂯂Ƃ�B
// ppCurrentOutputPin ���㗬�ɐڑ����ꂽ�L���ȃs���ł���̂��O��B
// ppCurrentOutputPin �͐���I���Ȃ�������A�f�R�[�_�t�B���^�̏o�̓s����*ppNewOutputPin�ɂȂ�B
// ppNewOutputPin==NULL �̏ꍇ�A�f�R�[�_�t�B���^�̏o�̓s����*ppCurrentOutputPin�ɂȂ�B����*ppCurrentOutputPin�͉�������
//
// ���s�����ꍇ�ł��t�B���^����͍s���Ȃ��p�X������(���ɐڑ��ς݂̏ꍇ)�B�߂�t�B���^�͊m�F���ĉ�����邱��
bool DirectShowUtil::AppendMpeg2Decoder_and_Connect(IGraphBuilder *pFilterGraph, CDirectShowFilterFinder *pUtil, IBaseFilter **ppMpeg2DecoderFilter,wchar_t *lpszDecoderName,int iDecNameBufLen, IPin **ppCurrentOutputPin, IPin **ppNewOutputPin)
{
	HRESULT hr;
	IPin *pInput = NULL;
	CDirectShowFilterFinder cUtil;	
	CLSID guidFilter = CLSID_NULL;
	bool bRet;
	wchar_t tmp[128];
	if(!lpszDecoderName){
		lpszDecoderName = tmp;
		iDecNameBufLen = 128;
		}

	// �|�C���^�`�F�b�N
	if(!pFilterGraph || !ppMpeg2DecoderFilter || !ppCurrentOutputPin) return false;
	// �����s���A�h���X�Ȃ� New==NULL �œ��͂��ꂽ�̂Ɠ��`
	if(ppCurrentOutputPin==ppNewOutputPin) ppNewOutputPin = NULL;
	// �w�肪�Ȃ��ꍇ�̃t�B���^����
	if(!pUtil){
		pUtil = &cUtil;
		if(!pUtil->FindFilter(&MEDIATYPE_Video,&MEDIASUBTYPE_MPEG2_VIDEO)) return false;
		}
	// �߂�l���N���A
	*ppMpeg2DecoderFilter = NULL;

	bRet=false;
	for(int i=0;!bRet&&i<pUtil->GetFilterCount();i++){
		if(!pUtil->GetFilterInfo(i,&guidFilter,lpszDecoderName,iDecNameBufLen)) continue;
		hr = ::CoCreateInstance(guidFilter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)ppMpeg2DecoderFilter);
		if(FAILED(hr)) continue;
		hr = pFilterGraph->AddFilter(*ppMpeg2DecoderFilter,lpszDecoderName);
		if(FAILED(hr)){
			SAFE_RELEASE(*ppMpeg2DecoderFilter);
			continue;
			}
		pInput = CDirectShowFilterFinder::GetFilterPin(*ppMpeg2DecoderFilter,PINDIR_INPUT);
		if(!pInput){
			SAFE_RELEASE(*ppMpeg2DecoderFilter);
			continue;
		}
		hr = pFilterGraph->Connect(*ppCurrentOutputPin,pInput);
		if(FAILED(hr)){
			SAFE_RELEASE(pInput);
			pFilterGraph->RemoveFilter(*ppMpeg2DecoderFilter);			
			SAFE_RELEASE(*ppMpeg2DecoderFilter);
			continue;
		} else {
			bRet=true;
		}
		}
	if(!bRet) {
		// �S�g�ݍ��킹�œK���f�R�[�_����������
		return false;
	}
	// �ڑ��Ɏg�����s�����
	SAFE_RELEASE(*ppCurrentOutputPin);
	SAFE_RELEASE(pInput);
	// ���t�B���^�ւ̏o�̓s����T��
	IPin *pNewOutput = GetFilterPin(*ppMpeg2DecoderFilter, PINDIR_OUTPUT);
	if(!pNewOutput){
		// �o�̓s����������Ȃ�
		return false;
		}
	if(ppNewOutputPin){
		// �V�o�̓s���ɏo��
		// ���̏o�̓s���͊��ɉ���ς�
		*ppNewOutputPin = pNewOutput;
		}else{
		// �o�̓s�����X�V(ppCurrentOutputPin==ppNewOutputPin�̏ꍇ�ł����̂��㏑��������X�V�ƂȂ�)
		// ���̏o�̓s���͊��ɉ���ς�
		*ppCurrentOutputPin = pNewOutput;
		}
	return true;
}
#endif

// �w��t�B���^�o�R���ăs���ڑ����s��(��ɓ���=1/�o��=1�̌o�R�^�t�B���^�ڑ��p)
// Filter�w���
//
// lpwszFilterName �� NULL �ł��ǂ��B
// ppCurrentOutputPin ���㗬�ɐڑ����ꂽ�L���ȃs���ł���̂��O��B
// ppCurrentOutputPin �͐���I���Ȃ�������A�t�B���^�̏o�̓s����*ppNewOutputPin�ɂȂ�B
// ppNewOutputPin==NULL �̏ꍇ�A�t�B���^�̏o�̓s����*ppCurrentOutputPin�ɂȂ�B����*ppCurrentOutputPin�͉�������
//
bool DirectShowUtil::AppendFilterAndConnect(IGraphBuilder *pFilterGraph,
				IBaseFilter *pFilter,LPCWSTR lpwszFilterName,
				IPin **ppCurrentOutputPin,IPin **ppNewOutputPin,bool fDirect)
{
	HRESULT hr;

	// �|�C���^�`�F�b�N
	if (!pFilterGraph || !pFilter || !ppCurrentOutputPin)
		return false;
	// �����s���A�h���X�Ȃ� New==NULL �œ��͂��ꂽ�̂Ɠ��`
	if (ppCurrentOutputPin==ppNewOutputPin)
		ppNewOutputPin = NULL;
	if (!lpwszFilterName)
		lpwszFilterName = L"No Name";
	hr = pFilterGraph->AddFilter(pFilter, lpwszFilterName);
	if (FAILED(hr)) {
		// �t�B���^�ɒǉ����s
		return false;
	}
	IPin *pInput = GetFilterPin(pFilter, PINDIR_INPUT);
	if (!pInput) {
		// ���̓s����������Ȃ�
		pFilterGraph->RemoveFilter(pFilter);
		return false;
	}
	// �ڑ�
	if (fDirect)
		hr = pFilterGraph->ConnectDirect(*ppCurrentOutputPin,pInput,NULL);
	else
		hr = pFilterGraph->Connect(*ppCurrentOutputPin,pInput);
	if (FAILED(hr)) {
		// �ڑ��ł��Ȃ�
		SAFE_RELEASE(pInput);
		pFilterGraph->RemoveFilter(pFilter);
		return false;
	}
	// �ڑ��Ɏg�����s�����
	SAFE_RELEASE(*ppCurrentOutputPin);
	SAFE_RELEASE(pInput);
	// ���t�B���^�ւ̏o�̓s����T��(�o�̓s���������Ă������͑��s)
	IPin *pNewOutput = GetFilterPin(pFilter, PINDIR_OUTPUT);
	if (ppNewOutputPin) {
		// �V�o�̓s���ɏo��
		// ���̏o�̓s���͊��ɉ���ς�
		*ppNewOutputPin = pNewOutput;
	} else {
		// �o�̓s�����X�V(ppCurrentOutputPin==ppNewOutputPin�̏ꍇ�ł����̂��㏑��������X�V�ƂȂ�)
		// ���̏o�̓s���͊��ɉ���ς�
		*ppCurrentOutputPin = pNewOutput;
	}
#ifdef _DEBUG
	LONG refCount = GetRefCount(pFilter);
#endif
	return true;
}

// �w��t�B���^�o�R���ăs���ڑ����s��(��ɓ���=1/�o��=1�̌o�R�^�t�B���^�ڑ��p)
// CLSID�w���
//
// AppendFilterAndConnect(Filter�w���)�̐������Q�ƁB
// guidFilter�͗L����DirectShow�t�B���^��GUID���w�肷��B
// ppAppendedFilter �͒ǉ������t�B���^���󂯎��B
bool DirectShowUtil::AppendFilterAndConnect(IGraphBuilder *pFilterGraph,
	const CLSID guidFilter,LPCWSTR lpwszFilterName,IBaseFilter **ppAppendedFilter,
	IPin **ppCurrentOutputPin,IPin **ppNewOutputPin,bool fDirect)
{
	// �t�B���^�C���X�^���X�쐬
	HRESULT hr = ::CoCreateInstance(guidFilter, NULL, CLSCTX_INPROC_SERVER,
				IID_IBaseFilter, reinterpret_cast<LPVOID*>(ppAppendedFilter));
	if (FAILED(hr)) {
		// �C���X�^���X�쐬���s
		return false;
	}
	bool bRet = AppendFilterAndConnect(pFilterGraph,*ppAppendedFilter,lpwszFilterName,
									ppCurrentOutputPin,ppNewOutputPin,fDirect);
	if (!bRet) {
		SAFE_RELEASE(*ppAppendedFilter);
		return false;
	}
	return true;
}

// �F��ԕϊ��t�B���^���o�R���ăs���ڑ����s��(����G�t�F�N�g�t�B���^�ւ̑Ή��̂��߂̐F��ԕϊ����K�v�ȏꍇ�̂���)
//
// AppendFilterAndConnect(Filter�w���)�̐������Q��
// ppColorSpaceConverterFilter �͒ǉ������t�B���^���󂯎��ׂ̃|�C���^
bool DirectShowUtil::AppendColorSpaceConverterFilter_and_Connect(IGraphBuilder *pFilterGraph, IBaseFilter **ppColorSpaceConverterFilter, IPin **ppCurrentOutputPin, IPin **ppNewOutputPin)
{
	return AppendFilterAndConnect(pFilterGraph,CLSID_ColorSpaceConverter,L"Color Space Converter",ppColorSpaceConverterFilter,ppCurrentOutputPin,ppNewOutputPin);
}

// �����_���̏o�͐�r�f�I�E�B���h�E�̃C���^�[�t�F�[�X�擾
// EVR�g�p����MF_GetVideoDisplayControl()���g��
IVideoWindow* DirectShowUtil::GetVideoWindow(IGraphBuilder *pGraph)
{
	IVideoWindow *pVideoWindow;
	HRESULT hr = pGraph->QueryInterface(IID_IVideoWindow,(void**)&pVideoWindow);
	if(SUCCEEDED(hr)){
		return pVideoWindow;
		}
	return NULL;
}

IBasicVideo2* DirectShowUtil::GetBasicVideo2(IGraphBuilder *pGraph)
{
	IBasicVideo2 *pBasicVideo;
	HRESULT hr = pGraph->QueryInterface(IID_IBasicVideo,(void**)&pBasicVideo);
	if(SUCCEEDED(hr)){
		return pBasicVideo;
		}
	return NULL;
}

IMediaControl* DirectShowUtil::GetMediaControl(IGraphBuilder *pGraph)
{
	IMediaControl *pMediaControl;
	HRESULT hr = pGraph->QueryInterface(IID_IMediaControl,(void**)&pMediaControl);
	if(SUCCEEDED(hr)){
		return pMediaControl;
		}
	return NULL;
}

bool DirectShowUtil::FilterGrapph_Play(IGraphBuilder *pFilterGraph)
{
	bool bRet=false;

	if(pFilterGraph){
		IMediaControl *pControl = GetMediaControl(pFilterGraph);
		if(pControl){
			HRESULT hr = pControl->Run();
			SAFE_RELEASE(pControl);
			bRet = true;
		}
	}
	return bRet;
}

bool DirectShowUtil::FilterGrapph_Stop(IGraphBuilder *pFilterGraph)
{
	bool bRet=false;

	if(pFilterGraph){
		IMediaControl *pControl = GetMediaControl(pFilterGraph);
		if(pControl){
			HRESULT hr = pControl->Stop();
			SAFE_RELEASE(pControl);
			bRet = true;
		}
	}
	return bRet;
}

bool DirectShowUtil::FilterGrapph_Pause(IGraphBuilder *pFilterGraph)
{
	bool bRet=false;

	if(pFilterGraph){
		IMediaControl *pControl = GetMediaControl(pFilterGraph);
		if(pControl){
			HRESULT hr = pControl->Pause();
			SAFE_RELEASE(pControl);
			bRet = true;
		}
	}
	return bRet;
}


//////////////////////////////////////////////////////////////////////
// �ȉ� EVR��p���[�e�B���e�B

#ifdef USE_MEDIA_FOUNDATION

// EVR��p : ����������
void DirectShowUtil::MF_Init()
{
	::MFStartup(MF_VERSION);
};

// EVR��p : �I������
void DirectShowUtil::MF_Term() {
	::MFShutdown();
};

// EVR��p : EVR�ݒ�p�C���^�[�t�F�[�X�̎擾
IEVRFilterConfig* DirectShowUtil::MF_GetEVRFilterConfig(IBaseFilter *pEvr)
{
	IEVRFilterConfig *pEvrFilterConfig;
	HRESULT hr = pEvr->QueryInterface(IID_IEVRFilterConfig,(void**)&pEvrFilterConfig);
	if(SUCCEEDED(hr)) {
		return pEvrFilterConfig;
		}
	return NULL;
}

// EVR��p : EVR����MF�̃T�[�r�X�擾�p�C���^�[�t�F�[�X�̎擾
IMFGetService* DirectShowUtil::MF_GetService(IBaseFilter *pEvr)
{
	IMFGetService *pMFGetService;
	HRESULT hr = pEvr->QueryInterface(IID_IMFGetService,(void**)&pMFGetService);
	if(SUCCEEDED(hr)) {
		return pMFGetService;
		}
	return NULL;
}

// EVR��p : ���̓X�g���[������ݒ肷��i�ʏ�͂P�j
bool DirectShowUtil::MF_SetNumberOfStreams(IBaseFilter *pEvr,int iStreamNumber)
{
	IEVRFilterConfig *pFilterConfig = MF_GetEVRFilterConfig(pEvr);
	if(pFilterConfig) {
		pFilterConfig->SetNumberOfStreams(iStreamNumber);
		SAFE_RELEASE(pFilterConfig);
		return true;
		}
	return false;
}

// EVR��p : �f�B�X�v���C����p�C���^�[�t�F�[�X�̎擾
IMFVideoDisplayControl* DirectShowUtil::MF_GetVideoDisplayControl(IBaseFilter *pEvr)
{
	IMFGetService *pService = MF_GetService(pEvr);
	if(pService) {
		IMFVideoDisplayControl *pDisplayControl;
		HRESULT hr = pService->GetService(MR_VIDEO_RENDER_SERVICE,IID_IMFVideoDisplayControl,(void**)&pDisplayControl);
		SAFE_RELEASE(pService);
		if(SUCCEEDED(hr)){
			return pDisplayControl;
			}
		}
	return NULL;
}

// EVR��p : �r�f�I�~�L�T����p�C���^�[�t�F�[�X�̎擾
IMFVideoMixerControl* DirectShowUtil::MF_GetVideoMixerControl(IBaseFilter *pEvr)
{
	IMFGetService *pService = MF_GetService(pEvr);
	if(pService) {
		IMFVideoMixerControl *pVideoMixerControl;
		HRESULT hr = pService->GetService(MR_VIDEO_MIXER_SERVICE,IID_IMFVideoMixerControl,(void**)&pVideoMixerControl);
		SAFE_RELEASE(pService);
		if(SUCCEEDED(hr)) {
			return pVideoMixerControl;
			}
		}
	return NULL;
}

// EVR��p : �r�f�I�v���Z�b�T����p�C���^�[�t�F�[�X�̎擾
IMFVideoProcessor* DirectShowUtil::MF_GetVideoProcessor(IBaseFilter *pEvr)
{
	IMFGetService *pService = MF_GetService(pEvr);
	if(pService) {
		IMFVideoProcessor *pVideoProcessor;
		HRESULT hr = pService->GetService(MR_VIDEO_MIXER_SERVICE,IID_IMFVideoProcessor,(void**)&pVideoProcessor);
		SAFE_RELEASE(pService);
		if(SUCCEEDED(hr)) {
			return pVideoProcessor;
			}
		}
	return NULL;
}

#endif