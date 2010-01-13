#include "stdafx.h"
#include "TVTest.h"
#include "AppMain.h"
#include "PlaybackOptions.h"
#include "DirectShowFilter/DirectShowUtil.h"
#include "DialogUtil.h"
#include "resource.h"




CPlaybackOptions::CPlaybackOptions()
{
	m_szAudioDeviceName[0]='\0';
	m_szAudioFilterName[0]='\0';
	m_fDownMixSurround=true;
	m_fRestoreMute=false;
	m_fUseAudioRendererClock=true;
	m_fAdjustAudioStreamTime=false;
	m_fMinTimerResolution=true;
	m_fPacketBuffering=false;
	m_PacketBufferLength=40000;
	m_PacketBufferPoolPercentage=50;
}


CPlaybackOptions::~CPlaybackOptions()
{
}


bool CPlaybackOptions::Apply(DWORD Flags)
{
	if ((Flags&UPDATE_PACKETBUFFERING)!=0) {
		CCoreEngine *pCoreEngine=GetAppClass().GetCoreEngine();

		if (!m_fPacketBuffering)
			pCoreEngine->SetPacketBuffering(false);
		pCoreEngine->SetPacketBufferLength(m_PacketBufferLength);
		pCoreEngine->SetPacketBufferPoolPercentage(m_PacketBufferPoolPercentage);
		if (m_fPacketBuffering)
			pCoreEngine->SetPacketBuffering(true);
	}
	return true;
}


bool CPlaybackOptions::Read(CSettings *pSettings)
{
	pSettings->Read(TEXT("AudioDevice"),m_szAudioDeviceName,MAX_AUDIO_DEVICE_NAME);
	pSettings->Read(TEXT("AudioFilter"),m_szAudioFilterName,MAX_AUDIO_FILTER_NAME);
	pSettings->Read(TEXT("DownMixSurround"),&m_fDownMixSurround);
	pSettings->Read(TEXT("RestoreMute"),&m_fRestoreMute);
	pSettings->Read(TEXT("UseAudioRendererClock"),&m_fUseAudioRendererClock);
	pSettings->Read(TEXT("AdjustAudioStreamTime"),&m_fAdjustAudioStreamTime);
	pSettings->Read(TEXT("MinTimerResolution"),&m_fMinTimerResolution);
	pSettings->Read(TEXT("PacketBuffering"),&m_fPacketBuffering);
	unsigned int BufferLength;
	if (pSettings->Read(TEXT("PacketBufferLength"),&BufferLength))
		m_PacketBufferLength=BufferLength;
	pSettings->Read(TEXT("PacketBufferPoolPercentage"),&m_PacketBufferPoolPercentage);
	return true;
}


bool CPlaybackOptions::Write(CSettings *pSettings) const
{
	pSettings->Write(TEXT("AudioDevice"),m_szAudioDeviceName);
	pSettings->Write(TEXT("AudioFilter"),m_szAudioFilterName);
	pSettings->Write(TEXT("DownMixSurround"),m_fDownMixSurround);
	pSettings->Write(TEXT("RestoreMute"),m_fRestoreMute);
	pSettings->Write(TEXT("UseAudioRendererClock"),m_fUseAudioRendererClock);
	pSettings->Write(TEXT("AdjustAudioStreamTime"),m_fAdjustAudioStreamTime);
	pSettings->Write(TEXT("MinTimerResolution"),m_fMinTimerResolution);
	pSettings->Write(TEXT("PacketBuffering"),m_fPacketBuffering);
	pSettings->Write(TEXT("PacketBufferLength"),(unsigned int)m_PacketBufferLength);
	pSettings->Write(TEXT("PacketBufferPoolPercentage"),m_PacketBufferPoolPercentage);
	return true;
}


bool CPlaybackOptions::SetPacketBuffering(bool fBuffering)
{
	m_fPacketBuffering=fBuffering;
	return true;
}


CPlaybackOptions *CPlaybackOptions::GetThis(HWND hDlg)
{
	return static_cast<CPlaybackOptions*>(GetOptions(hDlg));
}


INT_PTR CALLBACK CPlaybackOptions::DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CPlaybackOptions *pThis=static_cast<CPlaybackOptions*>(OnInitDialog(hDlg,lParam));

			int Sel=0;
			DlgComboBox_AddString(hDlg,IDC_OPTIONS_AUDIODEVICE,TEXT("�f�t�H���g"));
			CDirectShowDeviceEnumerator DevEnum;
			if (DevEnum.EnumDevice(CLSID_AudioRendererCategory)) {
				for (int i=0;i<DevEnum.GetDeviceCount();i++) {
					LPCTSTR pszName=DevEnum.GetDeviceFriendlyName(i);
					DlgComboBox_AddString(hDlg,IDC_OPTIONS_AUDIODEVICE,pszName);
					if (Sel==0 && ::lstrcmpi(pszName,pThis->m_szAudioDeviceName)==0)
						Sel=i+1;
				}
			}
			DlgComboBox_SetCurSel(hDlg,IDC_OPTIONS_AUDIODEVICE,Sel);

			Sel=0;
			DlgComboBox_AddString(hDlg,IDC_OPTIONS_AUDIOFILTER,TEXT("�Ȃ�"));
			CDirectShowFilterFinder FilterFinder;
			static const GUID InputTypes[] = {
				MEDIATYPE_Audio,	MEDIASUBTYPE_PCM
			};
			static const GUID OutputTypes[] = {
				MEDIATYPE_Audio,	MEDIASUBTYPE_PCM,
				MEDIATYPE_Audio,	MEDIASUBTYPE_DOLBY_AC3,
				MEDIATYPE_Audio,	MEDIASUBTYPE_DOLBY_AC3_SPDIF,
				MEDIATYPE_Audio,	MEDIASUBTYPE_DTS,
				MEDIATYPE_Audio,	MEDIASUBTYPE_AAC,
			};
			if (FilterFinder.FindFilter(InputTypes,lengthof(InputTypes),
										OutputTypes,lengthof(OutputTypes),
										0/*MERIT_DO_NOT_USE*/)) {
				WCHAR szAudioFilter[MAX_AUDIO_FILTER_NAME];
				CLSID idAudioFilter;

				for (int i=0;i<FilterFinder.GetFilterCount();i++) {
					if (FilterFinder.GetFilterInfo(i,&idAudioFilter,szAudioFilter,MAX_AUDIO_FILTER_NAME)) {
						DlgComboBox_AddString(hDlg,IDC_OPTIONS_AUDIOFILTER,szAudioFilter);
						if (Sel==0 && ::lstrcmpi(szAudioFilter,pThis->m_szAudioFilterName)==0)
							Sel=i+1;
					}
				}
			}
			DlgComboBox_SetCurSel(hDlg,IDC_OPTIONS_AUDIOFILTER,Sel);

			DlgCheckBox_Check(hDlg,IDC_OPTIONS_RESTOREMUTE,pThis->m_fRestoreMute);
			DlgCheckBox_Check(hDlg,IDC_OPTIONS_DOWNMIXSURROUND,pThis->m_fDownMixSurround);

			DlgCheckBox_Check(hDlg,IDC_OPTIONS_MINTIMERRESOLUTION,pThis->m_fMinTimerResolution);
			DlgCheckBox_Check(hDlg,IDC_OPTIONS_ADJUSTAUDIOSTREAMTIME,pThis->m_fAdjustAudioStreamTime);
			DlgCheckBox_Check(hDlg,IDC_OPTIONS_USEDEMUXERCLOCK,!pThis->m_fUseAudioRendererClock);

			// Buffering
			DlgCheckBox_Check(hDlg,IDC_OPTIONS_ENABLEBUFFERING,pThis->m_fPacketBuffering);
			EnableDlgItems(hDlg,IDC_OPTIONS_BUFFERING_FIRST,IDC_OPTIONS_BUFFERING_LAST,
					pThis->m_fPacketBuffering);
			SetDlgItemInt(hDlg,IDC_OPTIONS_BUFFERSIZE,pThis->m_PacketBufferLength,FALSE);
			DlgUpDown_SetRange(hDlg,IDC_OPTIONS_BUFFERSIZE_UD,1,INT_MAX);
			SetDlgItemInt(hDlg,IDC_OPTIONS_BUFFERPOOLPERCENTAGE,
						  pThis->m_PacketBufferPoolPercentage,TRUE);
			DlgUpDown_SetRange(hDlg,IDC_OPTIONS_BUFFERPOOLPERCENTAGE_UD,0,100);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OPTIONS_ENABLEBUFFERING:
			EnableDlgItemsSyncCheckBox(hDlg,IDC_OPTIONS_BUFFERING_FIRST,
									   IDC_OPTIONS_BUFFERING_LAST,
									   IDC_OPTIONS_ENABLEBUFFERING);
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				CPlaybackOptions *pThis=GetThis(hDlg);

				TCHAR szAudioDevice[MAX_AUDIO_DEVICE_NAME];
				int Sel=DlgComboBox_GetCurSel(hDlg,IDC_OPTIONS_AUDIODEVICE);
				if (Sel<=0)
					szAudioDevice[0]='\0';
				else
					DlgComboBox_GetLBString(hDlg,IDC_OPTIONS_AUDIODEVICE,Sel,szAudioDevice);
				if (::lstrcmpi(pThis->m_szAudioDeviceName,szAudioDevice)!=0) {
					::lstrcpy(pThis->m_szAudioDeviceName,szAudioDevice);
					SetGeneralUpdateFlag(UPDATE_GENERAL_BUILDMEDIAVIEWER);
				}

				TCHAR szAudioFilter[MAX_AUDIO_FILTER_NAME];
				Sel=DlgComboBox_GetCurSel(hDlg,IDC_OPTIONS_AUDIOFILTER);
				if (Sel<=0)
					szAudioFilter[0]='\0';
				else
					DlgComboBox_GetLBString(hDlg,IDC_OPTIONS_AUDIOFILTER,Sel,szAudioFilter);
				if (::lstrcmpi(pThis->m_szAudioFilterName,szAudioFilter)!=0) {
					::lstrcpy(pThis->m_szAudioFilterName,szAudioFilter);
					SetGeneralUpdateFlag(UPDATE_GENERAL_BUILDMEDIAVIEWER);
				}

				pThis->m_fDownMixSurround=DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_DOWNMIXSURROUND);
				GetAppClass().GetCoreEngine()->SetDownMixSurround(pThis->m_fDownMixSurround);
				pThis->m_fRestoreMute=DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_RESTOREMUTE);

				bool f=DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_MINTIMERRESOLUTION);
				if (f!=pThis->m_fMinTimerResolution) {
					pThis->m_fMinTimerResolution=f;
					if (GetAppClass().GetMainWindow()->IsPreview())
						GetAppClass().GetCoreEngine()->SetMinTimerResolution(f);
				}

				f=DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_ADJUSTAUDIOSTREAMTIME);
				if (f!=pThis->m_fAdjustAudioStreamTime) {
					pThis->m_fAdjustAudioStreamTime=f;
					GetAppClass().GetCoreEngine()->m_DtvEngine.m_MediaViewer.SetAdjustAudioStreamTime(f);
				}

				f=!DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_USEDEMUXERCLOCK);
				if (f!=pThis->m_fUseAudioRendererClock) {
					pThis->m_fUseAudioRendererClock=f;
					GetAppClass().GetCoreEngine()->m_DtvEngine.m_MediaViewer.SetUseAudioRendererClock(f);
					SetGeneralUpdateFlag(UPDATE_GENERAL_BUILDMEDIAVIEWER);
				}

				bool fBuffering=DlgCheckBox_IsChecked(hDlg,IDC_OPTIONS_ENABLEBUFFERING);
				DWORD BufferLength=::GetDlgItemInt(hDlg,IDC_OPTIONS_BUFFERSIZE,NULL,FALSE);
				int PoolPercentage=::GetDlgItemInt(hDlg,IDC_OPTIONS_BUFFERPOOLPERCENTAGE,NULL,TRUE);
				if (fBuffering!=pThis->m_fPacketBuffering
					|| (fBuffering
						&& (BufferLength!=pThis->m_PacketBufferLength
							|| PoolPercentage!=pThis->m_PacketBufferPoolPercentage)))
					pThis->SetUpdateFlag(UPDATE_PACKETBUFFERING);
				pThis->m_fPacketBuffering=fBuffering;
				pThis->m_PacketBufferLength=BufferLength;
				pThis->m_PacketBufferPoolPercentage=PoolPercentage;
			}
			break;
		}
		break;

	case WM_DESTROY:
		{
			CPlaybackOptions *pThis=GetThis(hDlg);

			pThis->OnDestroy();
		}
		return TRUE;
	}
	return FALSE;
}