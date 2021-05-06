//! \example ChromaBroadcastSampleApplicationDlg.cpp
// ChromaBroadcastSampleApplicationDlg.cpp : implementation file
//
/*
 *Copyright (c) 2021, Yeelight
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 */


#include "stdafx.h"
#include "ChromaYeelightApp.h"
#include "ChromaBroadcastSampleApplicationDlg.h"
#include "afxdialogex.h"


#include "PngButton.h"
#include "PngComboBox.h"
#include "CGuidanceDlg.h"
#include "CPng.h"
#include "CPngCheckBox.h"
#include "CCustomSliderCtrl2.h"
#include "CGraphicsRoundRectPath.h"
#include "IniFile.h"

#include "shlobj.h"//file directory

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#include <exception>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define AUTO_ADAPT_DPI //Adaptive to different dpi resolution screens

#define PNP_PORT         1982 //Multicast port

#define WM_USER_NOTIFYICON WM_USER+1 //Custom message, tray

#define PERSIST_LIGHT_CFG_PERIOD 5000 //5S Lamp configuration persistent period

#define LIGHT_SESSION_CHECK_PERIOD 3*1000//3S Lamp session detection period

#define BROADCAST_DISVOVER_PERIOD 5*1000//5S Multicast discovery device period

#define PERSIST_GLOBAL_CFG_PERIOD 5*1000//5S System configuration persistent period

#define PREVIEW_WAIT_COOLING_PERIOD 6*1000//6S Waiting interval after clicking the preview button

IP_ADAPTER_INFO* g_pAdapter;//all network card information
IP_ADAPTER_INFO* g_pCurrentAdapter;

typedef enum
{
	PERSIST_LIGHT_CFG_TIMER = 1,//Lamp configuration persistent timer
	LIGHT_SESSION_CHECK_TIMER,//Lamp session detection timer
	BROADCAST_DISVOVER_TIMER,//Multicast discovery device timer
	PERSIST_GLOBAL_CFG_TIMER,//System configuration persistent timer
	PREVIEW_WAIT_COOLING_TIMER,//Waiting timer after clicking the preview button
}chroma_yeelight_timer_t;


CChromaBroadcastSampleApplicationDlg* pInstance;

CDC* pDC = nullptr;

CRect dialogRect;

BOOL BroadcastEnabled = FALSE;

COLORREF LedColorLight[5];


unsigned char CalculateBrightByRGB(COLORREF LedColorTest)
{
	unsigned char R, G, B = 0;
	R = (LedColorTest & 0x00ff0000) >> 16;
	G = (LedColorTest & 0x0000ff00) >> 8;
	B = LedColorTest & 0x000000ff;

	unsigned char temp = 0;
	temp = (R > G) ? R : G;

	unsigned char max = 0;
	max = (temp > B) ? temp : B;

	unsigned char bright = ((float)max / 255.00) * 100.00;
	//return ((float)max / 255.00) * 100.00;

	return bright;
}


// CChromaBroadcastSampleApplicationDlg dialog

CChromaBroadcastSampleApplicationDlg::CChromaBroadcastSampleApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CHROMABROADCASTSAMPLEAPPLICATION_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);

	GdiplusStartup(&m_Taken, &m_Input, NULL);

	pMulticast = NULL;

	memset(pLightPic, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pLightName, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pLightSelectGroup, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pSelectGroupBtn, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pBrightness, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pBrightnessSlider, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pBrightnessEdit, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pBrightnessStatic, NULL, MAX_LIGHTS_PER_PAGE);
	memset(pCtrlButton, NULL, MAX_LIGHTS_PER_PAGE + 1);
	memset(pPreviewButton, NULL, MAX_LIGHTS_PER_PAGE);

	pGuidanceDlg = NULL;

	pNoDevice = NULL;
	m_TotalPage = 1;
	m_Currentpage = 1;

	pYeelightLogo = NULL;
	pLineButton = NULL;
	pGuidanceButton = NULL;
	pLoadingButton = NULL;
	pDeviceList = NULL;
	pRazerLogo = NULL;

	pPageIconPreview = NULL;
	pPageIconNext = NULL;
	pPageCount = NULL;

	m_DialogWidth = 0;
	m_DialogHeight = 0;
	m_zoom = 0;

	m_light_cfg_all.need_persist = FALSE;
	m_global_cfg.need_persist = FALSE;
	m_global_cfg.bAutoStart = FALSE;
	m_global_cfg.bOpenMainUI = TRUE;

	m_FontName.CreateFontW(25, 0, 0, 0, 400,
		FALSE, FALSE, 0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("arial")
	);

	m_FontNormal.CreateFontW(16, 0, 0, 0, 400,
		FALSE, FALSE, 0,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("arial")
	);

	//SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, sPath.GetBuffer(MAX_PATH));//L"C:\\Users\\username\\AppData\\Roaming"
	//C:\Users\xxx\AppData\Local\YeelightChromaConnector
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, m_LocalPath.GetBuffer(MAX_PATH));//L"C:\\Users\\username\\AppData\\Local"
	m_LocalPath.ReleaseBuffer();

	CString AppCfgPath = m_LocalPath + _T(".\/\/YeelightChromaConnector");
	if (!PathFileExists(AppCfgPath))
	{
		CreateDirectory(AppCfgPath, NULL);
	}
}

CChromaBroadcastSampleApplicationDlg::~CChromaBroadcastSampleApplicationDlg()
{
	if (pMulticast != NULL)
	{
		pMulticast->Close();
		delete pMulticast;
		pMulticast = NULL;
	}
	if (pYeelightLogo != NULL)
	{
		delete pYeelightLogo;
		pYeelightLogo = NULL;
	}
	if (pLineButton != NULL)
	{
		delete pLineButton;
		pLineButton = NULL;
	}
	if (pGuidanceButton != NULL)
	{
		delete pGuidanceButton;
		pGuidanceButton = NULL;
	}
	if (pLoadingButton != NULL)
	{
		delete pLoadingButton;
		pLoadingButton = NULL;
	}
	if (pDeviceList != NULL)
	{
		delete pDeviceList;
		pDeviceList = NULL;
	}
	if (pRazerLogo != NULL)
	{
		delete pRazerLogo;
		pRazerLogo = NULL;
	}
	if (pPageIconNext != NULL)
	{
		delete pPageIconNext;
		pPageIconNext = NULL;
	}
	if (pPageIconPreview != NULL)
	{
		delete pPageIconPreview;
		pPageIconPreview = NULL;
	}
	if (pPageCount != NULL)
	{
		delete pPageCount;
		pPageCount = NULL;
	}
	if (pGuidanceDlg != NULL)
	{
		delete pGuidanceDlg;
		pGuidanceDlg = NULL;
	}
	if (m_Taken)
		GdiplusShutdown(m_Taken);
}

void CChromaBroadcastSampleApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChromaBroadcastSampleApplicationDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	//ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CChromaBroadcastSampleApplicationDlg::OnNMCustomdrawSlider1)
	ON_COMMAND_RANGE(IDC_CTRL_BTN, IDC_CTRL_BTN + MAX_LIGHTS_PER_PAGE - 1, &CChromaBroadcastSampleApplicationDlg::OnBnClickedCtrl)
	ON_COMMAND_RANGE(IDC_PREVIEW_BTN, IDC_PREVIEW_BTN + MAX_LIGHTS_PER_PAGE - 1, &CChromaBroadcastSampleApplicationDlg::OnBnClickedPreview)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_SELECT_GROUP, IDC_SELECT_GROUP + MAX_LIGHTS_PER_PAGE - 1, &CChromaBroadcastSampleApplicationDlg::OnCbnSelChangeGroup)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_BRIGHTNESS_EDIT, IDC_BRIGHTNESS_EDIT + MAX_LIGHTS_PER_PAGE - 1, &CChromaBroadcastSampleApplicationDlg::OnEnChangeBrightnessEdit)
	ON_BN_CLICKED(IDC_GUIDENCE, &CChromaBroadcastSampleApplicationDlg::OnBnClickedGuidance)
	ON_COMMAND_RANGE(IDC_PRE_BTN, IDC_NEXT_BTN, &CChromaBroadcastSampleApplicationDlg::OnBnClickedPage)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_USER_NOTIFYICON, &CChromaBroadcastSampleApplicationDlg::OnTrayMsg)//

	ON_COMMAND(ID_TRAY_EXIT, &CChromaBroadcastSampleApplicationDlg::OnTrayExit)
	ON_COMMAND(ID_TRAY_OPEN, &CChromaBroadcastSampleApplicationDlg::OnTrayOpen)
	ON_COMMAND(ID_TRAY_AUTO_START, &CChromaBroadcastSampleApplicationDlg::OnTrayAutoStart)
	ON_UPDATE_COMMAND_UI(ID_TRAY_AUTO_START, &CChromaBroadcastSampleApplicationDlg::OnUpdateTrayAutoStart)
	ON_COMMAND(ID_TRAY_OPEN_MAIN, &CChromaBroadcastSampleApplicationDlg::OnTrayOpenMainUI)
	ON_UPDATE_COMMAND_UI(ID_TRAY_OPEN_MAIN, &CChromaBroadcastSampleApplicationDlg::OnUpdateTrayOpenMainUI)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CChromaBroadcastSampleApplicationDlg message handlers
BOOL CChromaBroadcastSampleApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	pInstance = this;

	pDC = GetDC();

	GetClientRect(&dialogRect); // Get rectangle for dialog box
	m_DialogWidth = dialogRect.Width();
	m_DialogHeight = dialogRect.Height();


	float zoomX = (float)m_DialogWidth / (float)1024;
	float zoomY = (float)m_DialogHeight / (float)768;
	m_zoom = (zoomX > zoomY) ? zoomX : zoomY;

	GetAllAdaptersInfo(&g_pAdapter);


	IP_ADAPTER_INFO* pAdapterInfo;
	if (GetNextValidAdapterInfo(&pAdapterInfo) == true)
	{
		InitLanCtrlUdp(pAdapterInfo->IpAddressList.IpAddress.String);
	}
	else
	{
		InitLanCtrlUdp(NULL);
	}


	BroadcastDiscoverMsg();

	InitLightCfg();

	InitGlobalCfg();

	//Graw guidance button
	DrawGuidanceButton();

	//Draw Device count
	DrawDeviceCount(this);

	//Draw razer Logo
	DrawRazerLogo();

	DrawLoadingIcon();

	DrawLine();

	DrawYeelightLogo();

	DrawPageInfo();

	Draw404NoDevice();

	ResizeMainDlg();

	setTray();

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CChromaBroadcastSampleApplicationDlg::InitLightCfg(void)
{
	m_light_cfg_all.need_persist = FALSE;
	m_light_cfg_all.light_cfg_vect.clear();
	m_light_cfg_all.light_cfg_sections.RemoveAll();

	m_light_cfg_all.IniFileLight.SetFileName(m_LocalPath + _T("\/\/YeelightChromaConnector") + _T(".\/\/light_cfg.ini"));
	m_light_cfg_all.IniFileLight.GetProfileSectionNames(m_light_cfg_all.light_cfg_sections);
	for (int i = 0; i < m_light_cfg_all.light_cfg_sections.GetSize(); i++)
	{
		st_light_cfg light_cfg = { 0 };
		CString strTemp;
		int len = 0;

		m_light_cfg_all.IniFileLight.GetProfileStringW(
			m_light_cfg_all.light_cfg_sections[i],
			_T("did"),
			strTemp);
		len = WideCharToMultiByte(CP_ACP, 0, strTemp, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, strTemp, -1, light_cfg.cDID, len, NULL, NULL);

		strTemp.Empty();
		m_light_cfg_all.IniFileLight.GetProfileStringW(
			m_light_cfg_all.light_cfg_sections[i],
			_T("bright"),
			strTemp);

		swscanf_s(strTemp, _T("%d"), &light_cfg.bright);


		strTemp.Empty();
		m_light_cfg_all.IniFileLight.GetProfileStringW(
			m_light_cfg_all.light_cfg_sections[i],
			_T("select_group"),
			strTemp);

		swscanf_s(strTemp, _T("%d"), &light_cfg.select_group);

		strTemp.Empty();
		m_light_cfg_all.IniFileLight.GetProfileStringW(
			m_light_cfg_all.light_cfg_sections[i],
			_T("chroma_ctrl_flag"),
			strTemp);

		swscanf_s(strTemp, _T("%d"), &light_cfg.chroma_ctrl_flag);


		m_light_cfg_all.light_cfg_vect.push_back(light_cfg);

	}

	SetTimer(PERSIST_LIGHT_CFG_TIMER, PERSIST_LIGHT_CFG_PERIOD, NULL);
	SetTimer(LIGHT_SESSION_CHECK_TIMER, LIGHT_SESSION_CHECK_PERIOD, NULL);
	SetTimer(BROADCAST_DISVOVER_TIMER, BROADCAST_DISVOVER_PERIOD, NULL);
}

void CChromaBroadcastSampleApplicationDlg::InitGlobalCfg(void)
{
	m_global_cfg.need_persist = FALSE;
	m_global_cfg.global_cfg_sections.RemoveAll();


	m_global_cfg.IniFileGlobal.SetFileName(m_LocalPath + _T("\/\/YeelightChromaConnector") + _T(".\/\/chroma_yeelight_cfg.ini"));
	m_global_cfg.IniFileGlobal.GetProfileSectionNames(m_global_cfg.global_cfg_sections);
	for (int i = 0; i < m_global_cfg.global_cfg_sections.GetSize(); i++)
	{
		CString strTemp;
		strTemp.Empty();
		m_global_cfg.IniFileGlobal.GetProfileStringW(
			m_global_cfg.global_cfg_sections[i],
			_T("auto_start"),
			strTemp);

		swscanf_s(strTemp, _T("%d"), &m_global_cfg.bAutoStart);

		strTemp.Empty();
		m_global_cfg.IniFileGlobal.GetProfileStringW(
			m_global_cfg.global_cfg_sections[i],
			_T("open_main_UI"),
			strTemp);

		swscanf_s(strTemp, _T("%d"), &m_global_cfg.bOpenMainUI);
	}

	SetTimer(PERSIST_GLOBAL_CFG_TIMER, PERSIST_GLOBAL_CFG_PERIOD, NULL);
}

void CChromaBroadcastSampleApplicationDlg::ResizeMainDlg(void)
{

	CRect rect;
	GetClientRect(&rect);

	CRect rect_temp;
	GetWindowRect(rect_temp);

	int width_new = rect_temp.Width() - rect.Width();
	int height_new = rect_temp.Height() - rect.Height();

#ifndef AUTO_ADAPT_DPI
	SetWindowPos(NULL, 0, 0, 1024 + width_new, 768 + height_new, SWP_NOMOVE);
	return;
#endif
	SetWindowPos(NULL, 0, 0, int(m_zoom * 1024) + width_new, int(m_zoom * 768) + height_new, SWP_NOMOVE);
}

void CChromaBroadcastSampleApplicationDlg::InitLanCtrlUdp(char* LocalIP)
{
	AfxSocketInit();
	pMulticast = new CMulticast;

	//pass CDialogEx to m_multicast
	pMulticast->set_dialog(this);
	if (LocalIP)
	{
		int num = MultiByteToWideChar(0, 0, LocalIP, -1, NULL, 0);
		wchar_t* wide = new wchar_t[num];
		MultiByteToWideChar(0, 0, LocalIP, -1, wide, num);
		pMulticast->Create(PNP_PORT, SOCK_DGRAM, FD_READ | FD_WRITE, wide);
	}
	else
	{
		pMulticast->Create(PNP_PORT, SOCK_DGRAM, FD_READ | FD_WRITE);
	}


	ip_mreq multiCast;
	inet_pton(AF_INET, "239.255.255.250", &(multiCast.imr_multiaddr.s_addr));


	if (LocalIP)
	{
		inet_pton(AF_INET, LocalIP, &(multiCast.imr_interface.s_addr));
	}
	else
	{
		multiCast.imr_interface.s_addr = htonl(INADDR_ANY);
	}


	if (pMulticast->SetSockOpt(IP_ADD_MEMBERSHIP, (char FAR*) & multiCast, sizeof(ip_mreq), IPPROTO_IP) < 0)
	{
		//this->m_receivedData.Insert(this->m_receivedData.GetLength(), _T("join multicast failed!\r\n"));
		//this->m_messagecontrol.SetWindowText(this->m_receivedData);
	}
	else
	{
		//this->m_receivedData.Insert(this->m_receivedData.GetLength(), _T("join multicast success!\r\n"));
		//this->m_messagecontrol.SetWindowText(this->m_receivedData);
	}

	BOOL MultipleApps = TRUE;
	if (pMulticast->SetSockOpt(SO_REUSEADDR, (void*)&MultipleApps, sizeof(BOOL), SOL_SOCKET) < 0)
	{
		//this->m_receivedData.Insert(this->m_receivedData.GetLength(), _T("set SO_REUSEADDR failed!\r\n"));
		//this->m_messagecontrol.SetWindowText(this->m_receivedData);
	}

	unsigned char ttl = 255;
	if (pMulticast->SetSockOpt(IP_MULTICAST_TTL, (char*)&ttl, sizeof(BOOL), IPPROTO_IP) < 0)
	{
		//this->m_receivedData.Insert(this->m_receivedData.GetLength(), _T("set IP_MULTICAST_TTL failed!\r\n"));
		//this->m_messagecontrol.SetWindowText(this->m_receivedData);
	}

	unsigned char loop = FALSE;
	if (pMulticast->SetSockOpt(IP_MULTICAST_LOOP, &loop, sizeof(BOOL), IPPROTO_IP) < 0)
	{
		//this->m_receivedData.Insert(this->m_receivedData.GetLength(), _T("set IP_MULTICAST_LOOP failed!\r\n"));
		//this->m_messagecontrol.SetWindowText(this->m_receivedData);
	}

	if (LocalIP)
	{
		struct in_addr addr;
		inet_pton(AF_INET, LocalIP, &(addr.s_addr));

		//Choose a valid network card
		pMulticast->SetSockOpt(IP_MULTICAST_IF, &addr, sizeof(addr), IPPROTO_IP);
	}
}


void CChromaBroadcastSampleApplicationDlg::DrawDeviceCount(CWnd* pParentWnd)
{
	CRect RectDeviceList = { 396, 110, 396 + 95, 110 + 20 };
	CRect rect_dpi;
	AdaptDpi(rect_dpi, RectDeviceList);

	pDeviceList = new CStatic();
	pDeviceList->Create(_T("Device List 0"), WS_CHILD | WS_VISIBLE, rect_dpi, pParentWnd, IDC_DEVICE_LIST);
	GetDlgItem(IDC_DEVICE_LIST)->SetFont(&m_FontNormal);
}

void CChromaBroadcastSampleApplicationDlg::AdaptDpi(CRect& rect_dpi, CRect& rect_old)
{
#ifndef AUTO_ADAPT_DPI
	rect_dpi.left = rect_old.left;
	rect_dpi.right = rect_old.right;
	rect_dpi.top = rect_old.top;
	rect_dpi.bottom = rect_old.bottom;
	return;
#endif
	rect_dpi.left = rect_old.left * m_zoom;
	rect_dpi.right = rect_old.right * m_zoom;
	rect_dpi.top = rect_old.top * m_zoom;
	rect_dpi.bottom = rect_old.bottom * m_zoom;
}


void CChromaBroadcastSampleApplicationDlg::DrawBrightnessCtrl(char index, CUdpLight& UdpLight)
{
	CRect rect_brightness_title[] = {
	   CRect(394, 300, 394 + 85, 300 + 20),
	   CRect(714, 300, 714 + 85, 300 + 20),
	   CRect(394, 584, 394 + 85, 584 + 20),
	   CRect(714, 584, 714 + 85, 584 + 20)
	};

	int xl_offset1 = 40;
	int xr_offset1 = 10;
	CRect rect_brightness_slider[] = {
	   CRect(474 + xl_offset1, 300, 474 + 120 + xr_offset1, 300 + 20),
	   CRect(794 + xl_offset1, 300, 794 + 120 + xr_offset1, 300 + 20),
	   CRect(474 + xl_offset1, 584, 474 + 120 + xr_offset1, 584 + 20),
	   CRect(794 + xl_offset1, 584, 794 + 120 + xr_offset1, 584 + 20)
	};

	int x_offset = 0;
	CRect rect_brightness_edit[] = {
	   CRect(614 + x_offset, 300, 614 + 40 + x_offset - 10, 300 + 20),
	   CRect(934 + x_offset, 300, 934 + 40 + x_offset - 10, 300 + 20),
	   CRect(614 + x_offset, 584, 614 + 40 + x_offset - 10, 584 + 20),
	   CRect(934 + x_offset, 584, 934 + 40 + x_offset - 10, 584 + 20)
	};

	CRect rect_brightness_static[] = {
	   CRect(614 + 40 + x_offset - 9, 300, 614 + 40 + x_offset + 5, 300 + 20),
	   CRect(934 + 40 + x_offset - 9, 300, 934 + 40 + x_offset + 5, 300 + 20),
	   CRect(614 + 40 + x_offset - 9, 584, 614 + 40 + x_offset + 5, 584 + 20),
	   CRect(934 + 40 + x_offset - 9, 584, 934 + 40 + x_offset + 5, 584 + 20)
	};

	//draw Brightness
	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_brightness_title[index]);

	pBrightness[index] = new CStatic();
	pBrightness[index]->Create(_T("Brightness"), WS_CHILD | WS_VISIBLE, rect_dpi, this, IDC_BRIGHTNESS + index);
	GetDlgItem(IDC_BRIGHTNESS + index)->SetFont(&m_FontNormal);

	//draw slider
	AdaptDpi(rect_dpi, rect_brightness_slider[index]);
	pBrightnessSlider[index] = new CCustomSliderCtr2();
	pBrightnessSlider[index]->Create(WS_CHILD | WS_VISIBLE | TBS_BOTTOM, rect_dpi, this, IDC_BRIGHTNESS_SLIDER + index);

	pBrightnessSlider[index]->SetRange(1, 100, TRUE);
	pBrightnessSlider[index]->SetTicFreq(1);
	pBrightnessSlider[index]->SetPos(UdpLight.ucBright);
	pBrightnessSlider[index]->SetLineSize(10);

	//draw brightness edit
	AdaptDpi(rect_dpi, rect_brightness_edit[index]);
	pBrightnessEdit[index] = new CEdit();
	pBrightnessEdit[index]->Create(WS_CHILD | WS_VISIBLE & ~ES_WANTRETURN, rect_dpi, this, IDC_BRIGHTNESS_EDIT + index);

	//draw brightness static "%"
	AdaptDpi(rect_dpi, rect_brightness_static[index]);
	pBrightnessStatic[index] = new CStatic();
	pBrightnessStatic[index]->Create(_T("%"), WS_CHILD | WS_VISIBLE, rect_dpi, this, IDC_BRIGHTNESS_STATIC + index);

	CString temp;
	temp.Format(_T("%d"), UdpLight.ucBright);
	SetDlgItemText(IDC_BRIGHTNESS_EDIT + index, temp);
}

void CChromaBroadcastSampleApplicationDlg::DrawBtnCtrlChroma(char index, CUdpLight& UdpLight)
{
	CRect rect_btn_ctrl[] = {
	   CRect(394, 345, 394 + 120, 345 + 40),
	   CRect(714, 345, 714 + 120, 345 + 40),
	   CRect(394, 625, 394 + 120, 625 + 40),
	   CRect(714, 625, 714 + 120, 625 + 40)
	};

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_btn_ctrl[index]);

	pCtrlButton[index] = new CPngCheckBox();

	pCtrlButton[index]->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_CTRL_BTN + index);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pCtrlButton[index]->Init(IDB_PNG14, IDB_PNG2);// Load the bitmaps for this button.
	pCtrlButton[index]->SetChecked(UdpLight.bAllowCtrlFlag);
}

void CChromaBroadcastSampleApplicationDlg::DrawBtnPreview(char index, CUdpLight& UdpLight)
{
	CRect rect_btn_preview[] = {
	   CRect(534, 345, 534 + 120, 345 + 40),
	   CRect(854, 345, 854 + 120, 345 + 40),
	   CRect(534, 625, 534 + 120, 625 + 40),
	   CRect(854, 625, 854 + 120, 625 + 40)
	};

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_btn_preview[index]);

	pPreviewButton[index] = new CPngButton();

	pPreviewButton[index]->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_PREVIEW_BTN + index);// Create the bitmap button (must include the BS_OWNERDRAW style).

	pPreviewButton[index]->Init(BTN_TYPE_NORMAL, IDB_PNG16, IDB_PNG15, IDB_PNG15);// Load the bitmaps for this button.
}


void CChromaBroadcastSampleApplicationDlg::OnEnChangeBrightnessEdit(UINT uID)
{
	int bright_edit = 0;
	CString bright_show;
	bool valid_flag = FALSE;
	//range [IDC_BRIGHTNESS_EDIT, IDC_BRIGHTNESS_EDIT + MAX_LIGHTS_PER_PAGE - 1]
	if (uID < IDC_BRIGHTNESS_EDIT || uID >(IDC_BRIGHTNESS_EDIT + MAX_LIGHTS_PER_PAGE - 1))
	{
		return;
	}

	valid_flag = TRUE;
	GetDlgItemTextW(uID, bright_show);
	bright_edit = _ttoi(bright_show);

	if (valid_flag == FALSE)
	{
		return;
	}

	if (bright_edit < 1 || bright_edit > 100)
	{
		bright_edit = (bright_edit < 1) ? 1 : 100;
	}


	CUdpLight* selUdpLight = NULL;
	if (FindLightByPage(uID - IDC_BRIGHTNESS_EDIT, m_Currentpage, &selUdpLight))
	{
		if (selUdpLight->ucBright == bright_edit)
		{
			return;
		}
		selUdpLight->ucBright = bright_edit;
		pBrightnessSlider[uID - IDC_BRIGHTNESS_EDIT]->SetPos(bright_edit);

		SendBrightneesToLight(*selUdpLight);

		UpdateLightCfg(*selUdpLight);
	}

	CString temp;
	temp.Format(_T("%d"), bright_edit);
	SetDlgItemText(uID, temp);

	CEdit* input = (CEdit*)GetDlgItem(uID);
	input->SetSel(temp.GetLength(), temp.GetLength(), TRUE);
	input->SetFocus();
}


void CChromaBroadcastSampleApplicationDlg::DrawGuidanceButton(void)
{
	CRect rect_guidance = { 894, 41, 994, 61 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_guidance);

	pGuidanceButton = new CPngButton();
	pGuidanceButton->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_GUIDENCE);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pGuidanceButton->Init(BTN_TYPE_NORMAL, IDB_PNG12, IDB_PNG28, IDB_PNG12);// Load the bitmaps for this button.
}

void CChromaBroadcastSampleApplicationDlg::DrawRazerLogo(void)
{
	CRect rect_razer_logo = { 34, 658, 34 + 90, 658 + 90 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_razer_logo);

	pRazerLogo = new CPng();
	pRazerLogo->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_RAZER_LOGO);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pRazerLogo->Init(IDB_PNG9, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}

void CChromaBroadcastSampleApplicationDlg::Draw404NoDevice(void)
{
	CRect rect_404 = { 354, 0, 354 + 670, 0 + 768 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_404);

	pNoDevice = new CPng();
	pNoDevice->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_404_NO_DEVICE);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pNoDevice->Init(IDB_PNG23, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}

void CChromaBroadcastSampleApplicationDlg::Clear404NoDevice()
{
	if (pNoDevice != NULL)
	{
		delete pNoDevice;
		pNoDevice = NULL;
	}
}

//draw loading device icon
void CChromaBroadcastSampleApplicationDlg::DrawLoadingIcon(void)
{
	CRect rect_loading_icon = { 374, 112, 374 + 16, 112 + 16 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_loading_icon);

	pLoadingButton = new CPng();
	pLoadingButton->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_LOADING_ICON);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pLoadingButton->Init(IDB_PNG10, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}

//draw loading device icon
void CChromaBroadcastSampleApplicationDlg::DrawLine(void)
{
	CRect rect_loading_icon = { 374, 46, 374 + 122, 46 + 12 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_loading_icon);

	pLineButton = new CPng();
	pLineButton->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_DRAW_LINE);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pLineButton->Init(IDB_PNG11, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}

void CChromaBroadcastSampleApplicationDlg::DrawYeelightLogo(void)
{
	CRect rect_loading_icon = { 30, 30, 30 + 160, 30 + 44 };

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_loading_icon);

	pYeelightLogo = new CPng();
	pYeelightLogo->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_YEELIGHT_LOGO);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pYeelightLogo->Init(IDB_PNG1, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}

void CChromaBroadcastSampleApplicationDlg::DrawPageInfo(void)
{
	CRect rect_preview_page_icon = { 891, 106, 891 + 28, 106 + 28 };
	CRect rect_next_page_icon = { 966, 106, 966 + 28, 106 + 28 };

	pPageIconPreview = new CPngButton();

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_preview_page_icon);

	pPageIconPreview->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_PRE_BTN);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pPageIconPreview->Init(BTN_TYPE_NORMAL, IDB_PNG22, IDB_PNG21, IDB_PNG21, IDB_PNG27);// Load the bitmaps for this button.
	pPageIconPreview->SetState(CTRL_DISABLE);

	pPageIconNext = new CPngButton();

	AdaptDpi(rect_dpi, rect_next_page_icon);

	pPageIconNext->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, IDC_NEXT_BTN);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pPageIconNext->Init(BTN_TYPE_NORMAL, IDB_PNG20, IDB_PNG19, IDB_PNG19, IDB_PNG26);// Load the bitmaps for this button.
	pPageIconNext->SetState(CTRL_DISABLE);

	CRect rect_page_count = { 933, 112, 933 + 19, 112 + 17 };

	AdaptDpi(rect_dpi, rect_page_count);

	pPageCount = new CStatic();
	pPageCount->Create(_T("1/1 "), WS_CHILD | WS_VISIBLE, rect_dpi, this, IDC_PAGE_COUNT);
	GetDlgItem(IDC_PAGE_COUNT)->SetFont(&m_FontNormal);
}

void CChromaBroadcastSampleApplicationDlg::DrawLightIcon(char index, CUdpLight& UdpLight)
{
	CRect rect_light_pic[] = {
		CRect(394, 166, 394 + 44, 166 + 44),
		CRect(714, 166, 714 + 44, 166 + 44),
		CRect(394, 446, 394 + 44, 446 + 44),
		CRect(714, 446, 714 + 44, 446 + 44)
	};

	int icon_id = IDB_PNG18;
	if (strstr(UdpLight.cModel, "lamp19") != NULL)
	{
		icon_id = IDB_PNG18;
	}
	else if (strstr(UdpLight.cModel, "color") != NULL)
	{
		icon_id = IDB_PNG24;
	}
	else if (strstr(UdpLight.cModel, "strip") != NULL)
	{
		icon_id = IDB_PNG25;
	}
	else if (strstr(UdpLight.cModel, "lamp15") != NULL)
	{
		icon_id = IDB_PNG29;
	}

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_light_pic[index]);

	pLightPic[index] = new CPng();
	pLightPic[index]->Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect_dpi, this, NULL);// Create the bitmap button (must include the BS_OWNERDRAW style).
	pLightPic[index]->Init(icon_id, 1, BTN_TYPE_NORMAL);// Load the bitmaps for this button.
}


void CChromaBroadcastSampleApplicationDlg::DrawLightName(char index, CUdpLight& UdpLight)
{
	CRect rect_name[] = {
	   CRect(449, 176, 449 + 182, 176 + 25),
	   CRect(769, 176, 769 + 182, 176 + 25),
	   CRect(449, 460, 449 + 182, 460 + 25),
	   CRect(769, 460, 769 + 182, 460 + 25)
	};

	char echoName[40] = { 0 };
	memcpy(echoName, UdpLight.cModel, sizeof(UdpLight.cModel));
	memcpy(&echoName[strlen(echoName)], "-", 1);
	memcpy(&echoName[strlen(echoName)], UdpLight.cDID + 14, 4);

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_name[index]);

	CString strName(echoName);

	pLightName[index] = new CStatic();
	pLightName[index]->Create(strName, WS_CHILD | WS_VISIBLE, rect_dpi, this, IDC_LIGHT_NAME + index);
	GetDlgItem(IDC_LIGHT_NAME + index)->SetFont(&m_FontName);
}


void CChromaBroadcastSampleApplicationDlg::DrawSelectGroup(char index, CUdpLight& UdpLight)
{
	CRect rect_select_group[] = {
	   CRect(394, 240, 394 + 85, 240 + 20),
	   CRect(714, 240, 714 + 85, 240 + 20),
	   CRect(394, 524, 394 + 85, 524 + 20),
	   CRect(714, 524, 714 + 85, 524 + 20)
	};

	int xl_offset = -22;
	CRect rect_select_group_list[] = {
	   CRect(534 + xl_offset, 230, 534 + 120, 230 + 30),
	   CRect(854 + xl_offset, 230, 854 + 120, 230 + 30),
	   CRect(534 + xl_offset, 514, 534 + 120, 514 + 30),
	   CRect(854 + xl_offset, 514, 854 + 120, 514 + 30)

	};

	//draw text
	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_select_group[index]);

	pLightSelectGroup[index] = new CStatic();
	pLightSelectGroup[index]->Create(_T("Select Group"), WS_CHILD | WS_VISIBLE, rect_dpi, this, IDC_SELECT_GROUP_STATIC + index);
	GetDlgItem(IDC_SELECT_GROUP_STATIC + index)->SetFont(&m_FontNormal);

	//draw combobox
	AdaptDpi(rect_dpi, rect_select_group_list[index]);

	pSelectGroupBtn[index] = new CPngComboBox();
	pSelectGroupBtn[index]->Create(
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_AUTOHSCROLL,
		rect_dpi, this, IDC_SELECT_GROUP + index);// Create the button (must include the BS_OWNERDRAW style).

	pSelectGroupBtn[index]->AddNewString(_T("Global"));
	pSelectGroupBtn[index]->AddNewString(_T("Group1"));
	pSelectGroupBtn[index]->AddNewString(_T("Group2"));
	pSelectGroupBtn[index]->AddNewString(_T("Group3"));
	pSelectGroupBtn[index]->AddNewString(_T("Group4"));
	pSelectGroupBtn[index]->SetFontType(20, _T("arial"), FALSE, RGB(96, 92, 98));
	pSelectGroupBtn[index]->SetRightButton(IDB_PNG7, IDB_PNG6);
	pSelectGroupBtn[index]->SetCurSel(UdpLight.cMappingLED - 1);

	pSelectGroupBtn[index]->SetItemHeight(1, 30);
}



void CChromaBroadcastSampleApplicationDlg::DrawLight(char index, CUdpLight& UdpLight)
{
	Clear404NoDevice();

	//draw light icon
	DrawLightIcon(index, UdpLight);

	//draw light name
	DrawLightName(index, UdpLight);

	//draw select group
	DrawSelectGroup(index, UdpLight);

	//draw brightness
	DrawBrightnessCtrl(index, UdpLight);

	//draw button enable/disable
	DrawBtnCtrlChroma(index, UdpLight);

	//draw button Preview
	DrawBtnPreview(index, UdpLight);

}

void CChromaBroadcastSampleApplicationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC   dc(this);
		CDC   dcMem;
		dcMem.CreateCompatibleDC(&dc);

		GrawWndBg(dc, dcMem);

		GrawLightBgBitmap(dc, dcMem, 0);
		GrawLightBgBitmap(dc, dcMem, 1);
		GrawLightBgBitmap(dc, dcMem, 2);
		GrawLightBgBitmap(dc, dcMem, 3);

		//draw light bg --- CImage
		//GrawLightBgCImage();

		//draw light bg --- gdi+
		//GrawLightBgGgiPlus(0);
		//GrawLightBgGgiPlus(1);
		//GrawLightBgGgiPlus(2);
	}
}


void CChromaBroadcastSampleApplicationDlg::GrawWndBg(CPaintDC& dc, CDC& dcMem)
{
	CRect rect;
	GetClientRect(&rect);

	CBitmap   bmpBackground;
	bmpBackground.LoadBitmap(IDB_BITMAP3);

	BITMAP   bitmap;
	bmpBackground.GetBitmap(&bitmap);
	CBitmap* pbmpOld = dcMem.SelectObject(&bmpBackground);
	dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	printf("");
}

void CChromaBroadcastSampleApplicationDlg::GrawLightBgBitmap(CPaintDC& DC, CDC& CDC, char index)
{
	CRect rect_light_bg[4] = {
		CRect(374, 146, 374 + 300, 146 + 264),
		CRect(694, 146, 694 + 300, 146 + 264),
		CRect(374, 426, 374 + 300, 426 + 264),
		CRect(694, 430, 694 + 300, 430 + 264)
	};

	int id_bg = IDB_BITMAP4;
	switch (index)
	{
	case 0:
		id_bg = IDB_BITMAP4;
		break;
	case 1:
		id_bg = IDB_BITMAP5;
		break;
	case 2:
		id_bg = IDB_BITMAP9;
		break;
	case 3:
		id_bg = IDB_BITMAP10;
		break;
	default:
		break;
	}

	CBitmap   bmpLightBg;
	bmpLightBg.LoadBitmap(id_bg);

	BITMAP   bitmap1;
	bmpLightBg.GetBitmap(&bitmap1);
	CDC.SelectObject(&bmpLightBg);

	CRect rect_dpi;
	AdaptDpi(rect_dpi, rect_light_bg[index]);

	bool ret;

	ret = DC.StretchBlt(rect_dpi.left, rect_dpi.top, rect_dpi.Width(), rect_dpi.Height(),
		&CDC,
		0, 0, bitmap1.bmWidth, bitmap1.bmHeight,
		SRCCOPY);

	//dcMem.SelectObject(pbmpOld);
}

void CChromaBroadcastSampleApplicationDlg::GrawLightBgGgiPlus(char index)
{
	int light_bg[3][2] = {
		{374, 146},
		{694, 146},
		{374, 426}
	};

	CDC* pDC = this->GetDC();
	CGraphicsRoundRectPath  RoundRectPath;//Create a rounded rectangle path object
	RoundRectPath.AddRoundRect(light_bg[index][0], light_bg[index][1], 300, 264, 10, 10);

	Graphics  graph(pDC->GetSafeHdc());
	Gdiplus::Pen red(Gdiplus::Color(255, 0X12, 0X12, 0X2B), 2);
	Gdiplus::SolidBrush brush_blue(Gdiplus::Color(255, 0X12, 0X12, 0X2B));
	graph.DrawPath(&red, &RoundRectPath);
	graph.FillPath(&brush_blue, &RoundRectPath);

	graph.ReleaseHDC(pDC->GetSafeHdc());
}





void CChromaBroadcastSampleApplicationDlg::OnDestroy()
{

	if (pDC)
	{
		ReleaseDC(pDC);
		pDC = nullptr;
	}
	Shell_NotifyIcon(NIM_DELETE, &m_notify);//Delete icon after exiting the program
}

void CChromaBroadcastSampleApplicationDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case PERSIST_LIGHT_CFG_TIMER:
		PersistLightCfg();
		break;
	case LIGHT_SESSION_CHECK_TIMER:
		LightSessionCheck();
		break;
	case BROADCAST_DISVOVER_TIMER:
		//if (!HasOnlineUdpLight())
		//{
		//	UpdateMulticastIF();
		//}
		//UpdateMulticastIF();
		pMulticast->Close();
		delete pMulticast;
		pMulticast = NULL;
		IP_ADAPTER_INFO* pAdapterInfo;
		if (GetNextValidAdapterInfo(&pAdapterInfo) == true)
		{
			InitLanCtrlUdp(pAdapterInfo->IpAddressList.IpAddress.String);
		}
		else
		{
			//lan_ctrl
			InitLanCtrlUdp(NULL);
		}
		BroadcastDiscoverMsg();
		break;
	case PERSIST_GLOBAL_CFG_TIMER:
		PersistGlobalCfg();
		break;
	case PREVIEW_WAIT_COOLING_TIMER:
		ProcPreviewTimer();
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


HBRUSH CChromaBroadcastSampleApplicationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetTextColor(RGB(68, 214, 44));//green
	pDC->SetBkColor(RGB(34, 34, 34));//blace

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC_CL1:
	case IDC_STATIC_CL2:
	case IDC_STATIC_CL3:
	case IDC_STATIC_CL4:
	case IDC_STATIC_CL5:
		pDC->SetTextColor(RGB(225, 225, 225));//white
		pDC->SetBkColor(RGB(248, 248, 255));//gray
		break;
	case IDC_LIGHT_NAME:
	case IDC_LIGHT_NAME + 1:
	case IDC_LIGHT_NAME + 2:
#if !USE_ADV_PIC
	case IDC_LIGHT_NAME + 3:
#endif
		pDC->SetTextColor(RGB(0xff, 0xff, 0xff));//Set text color
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));//Set text background color
		pDC->SetBkMode(OPAQUE);//Set the text background mode to transparent
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));//Set the color outside the control text
		break;
	case IDC_BRIGHTNESS:
	case IDC_BRIGHTNESS + 1:
	case IDC_BRIGHTNESS + 2:
#if !USE_ADV_PIC
	case IDC_BRIGHTNESS + 3:
#endif
		pDC->SetTextColor(RGB(0x96, 0x92, 0x98));
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));
		pDC->SetBkMode(OPAQUE);
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));
		break;
	case IDC_SELECT_GROUP_STATIC:
	case IDC_SELECT_GROUP_STATIC + 1:
	case IDC_SELECT_GROUP_STATIC + 2:
#if !USE_ADV_PIC
	case IDC_SELECT_GROUP_STATIC + 3:
#endif
		pDC->SetTextColor(RGB(0x96, 0x92, 0x98));
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));
		pDC->SetBkMode(OPAQUE);
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));
		break;
	case IDC_DEVICE_LIST:
		pDC->SetTextColor(RGB(0xff, 0xff, 0xff));
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));
		pDC->SetBkMode(OPAQUE);
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));
		break;
	case IDC_BRIGHTNESS_STATIC:
	case IDC_BRIGHTNESS_STATIC + 1:
	case IDC_BRIGHTNESS_STATIC + 2:
	case IDC_BRIGHTNESS_EDIT:
	case IDC_BRIGHTNESS_EDIT + 1:
	case IDC_BRIGHTNESS_EDIT + 2:
#if !USE_ADV_PIC
	case IDC_BRIGHTNESS_STATIC + 3:
	case IDC_BRIGHTNESS_EDIT + 3:
#endif
		pDC->SetTextColor(RGB(0x96, 0x92, 0x98));
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));
		pDC->SetBkMode(OPAQUE);
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));
	case IDC_PAGE_COUNT:
		//case IDC_PAGE_CURRENT:
		pDC->SetTextColor(RGB(0xff, 0xff, 0xff));
		pDC->SetBkColor(RGB(0x12, 0x12, 0x2b));
		pDC->SetBkMode(OPAQUE);
		hbr = CreateSolidBrush(RGB(0x12, 0x12, 0x2b));
		break;
	default:
		break;
	}


	if (nCtlColor == CTLCOLOR_DLG)
	{
		HBRUSH m_Brush = CreateSolidBrush(RGB(34, 34, 34));//black
		hbr = m_Brush;
	}

	return hbr;
}


void CChromaBroadcastSampleApplicationDlg::OnClose()
{

	if (pDC)
	{
		ReleaseDC(pDC);
		pDC = nullptr;
	}
	KillTimer(PERSIST_LIGHT_CFG_TIMER);
	KillTimer(LIGHT_SESSION_CHECK_TIMER);
	KillTimer(BROADCAST_DISVOVER_TIMER);
	ClearLightDetail();

	CUdpLight* pFoundLight = NULL;
	vector<CUdpLight*>::iterator it;
	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{

		pFoundLight = *it;
		if (pFoundLight != NULL)
		{
			delete pFoundLight;
			pFoundLight = NULL;
		}
	}
	udp_light_vect.clear();


	CDialogEx::OnClose();
}


void CChromaBroadcastSampleApplicationDlg::OnConnect()
{
	//this->m_receivedData.Insert(m_receivedData.GetLength(), _T("Successfully connected to the server!\r\n"));
	//this->m_messagecontrol.SetWindowText(m_receivedData);
	//m_btn_con.EnableWindow(false);
	//m_btn_discon.EnableWindow(true);
}
void CChromaBroadcastSampleApplicationDlg::OnMulticastClose()
{
	pMulticast->Close();
}

void CChromaBroadcastSampleApplicationDlg::OnReceive()
{
	char* pBuf = new char[2048];

	int iLen;
	SOCKADDR fromSocket;
	int addrLen = sizeof(SOCKADDR);
	iLen = pMulticast->ReceiveFrom(pBuf, 2048, &fromSocket, &addrLen, 0);
	if (iLen == SOCKET_ERROR)
	{
		//::MessageBox(GetSafeHwnd(), _T("Unable to accept data!"), MB_OK);
	}
	else
	{
		ProcRecvData(pBuf);
		pBuf[iLen] = NULL;
		delete[] pBuf;
		return;
	}
	delete[] pBuf;
}



void CChromaBroadcastSampleApplicationDlg::ProcRecvData(char* recv_data)
{
	CUdpLight* p_UdpLight = new CUdpLight();

	if (ParseDiscoveryMSG(recv_data, *p_UdpLight) == false)
	{
		delete(p_UdpLight);
		return;
	}
	if (IsSupportUDP(*p_UdpLight) == false)
	{
		delete(p_UdpLight);
		return;
	}
	if (udp_light_vect.size() >= MAX_LIGHTS_SUPPORT)
	{
		delete(p_UdpLight);
		return;
	}

	if (IsAlreadyFound(*p_UdpLight) == false)
	{
		CString strModel(p_UdpLight->cIP);

		udp_light_vect.push_back(p_UdpLight);
		RefreshLightList(*p_UdpLight);

		if (LoadLightCfg(*p_UdpLight) == false)
		{
			AddLightCfg(*p_UdpLight);
		}
		//if (g_pCurrentAdapter)
		//{
		//	struct sockaddr_in BindAddr;
		//	BindAddr.sin_family = AF_INET;
		//	BindAddr.sin_port = htons(LAN_SERVER_UDP_PORT);
		//	inet_pton(AF_INET, g_pCurrentAdapter->IpAddressList.IpAddress.String, &(BindAddr.sin_addr.s_addr));

		//	p_UdpLight->Bind((const SOCKADDR*)&BindAddr, sizeof(struct sockaddr_in));
		//}

		p_UdpLight->AcquireToken();

		//update device list count
		CString strDeviceCount;
		strDeviceCount.Format(_T("Device List (""%d"")"), udp_light_vect.size());
		GetDlgItem(IDC_DEVICE_LIST)->SetWindowTextW(strDeviceCount);

		m_TotalPage = (((int)udp_light_vect.size() - 1) / MAX_LIGHTS_PER_PAGE) + 1;
		CString strPageCount;
		strPageCount.Format(_T("""%d""\/""%d"""), m_Currentpage, m_TotalPage);
		GetDlgItem(IDC_PAGE_COUNT)->SetWindowTextW(strPageCount);

		if (m_Currentpage == m_TotalPage)
		{
			pPageIconNext->SetState(CTRL_DISABLE);
			pPageIconPreview->SetState(CTRL_NOFOCUS);
			if (m_Currentpage == 1)
			{
				pPageIconPreview->SetState(CTRL_DISABLE);
			}
		}
		else if (m_Currentpage < m_TotalPage)
		{
			pPageIconNext->SetState(CTRL_NOFOCUS);
			if (m_Currentpage == 1)
			{
				pPageIconPreview->SetState(CTRL_DISABLE);
			}
			else
			{
				pPageIconPreview->SetState(CTRL_NOFOCUS);
			}
		}
		ClearLightDetail();
		RefreshLightShowPage();
	}
	else
	{
		delete(p_UdpLight);
	}
}

bool CChromaBroadcastSampleApplicationDlg::LoadLightCfg(CUdpLight& UdpLight)
{
	vector<st_light_cfg>::iterator it;


	for (it = m_light_cfg_all.light_cfg_vect.begin(); it != m_light_cfg_all.light_cfg_vect.end(); it++)
	{
		if (memcmp(UdpLight.cDID, &((*it).cDID), sizeof(UdpLight.cDID)) == 0)
		{
			UdpLight.ucBright = (*it).bright;
			UdpLight.cMappingLED = (*it).select_group;
			UdpLight.bAllowCtrlFlag = (*it).chroma_ctrl_flag;
			return true;
		}
	}
	return false;
}

void CChromaBroadcastSampleApplicationDlg::AddLightCfg(CUdpLight& UdpLight)
{
	st_light_cfg light_cfg_temp = { 0 };
	memcpy(light_cfg_temp.cDID, UdpLight.cDID, sizeof(UdpLight.cDID));

	light_cfg_temp.bright = UdpLight.ucBright;
	light_cfg_temp.chroma_ctrl_flag = UdpLight.bAllowCtrlFlag;
	light_cfg_temp.select_group = UdpLight.cMappingLED;

	m_light_cfg_all.light_cfg_vect.push_back(light_cfg_temp);
	m_light_cfg_all.need_persist = TRUE;
}

void CChromaBroadcastSampleApplicationDlg::UpdateLightCfg(CUdpLight& UdpLight)
{
	vector<st_light_cfg>::iterator it;

	for (it = m_light_cfg_all.light_cfg_vect.begin(); it != m_light_cfg_all.light_cfg_vect.end(); it++)
	{
		if (memcmp(UdpLight.cDID, &((*it).cDID), sizeof(UdpLight.cDID)) == 0)
		{
			(*it).bright = UdpLight.ucBright;
			(*it).select_group = UdpLight.cMappingLED;
			(*it).chroma_ctrl_flag = UdpLight.bAllowCtrlFlag;

			m_light_cfg_all.need_persist = TRUE;
			return;
		}
	}
}

void CChromaBroadcastSampleApplicationDlg::PersistLightCfg()
{
	if (m_light_cfg_all.need_persist == TRUE)
	{
		vector<st_light_cfg>::iterator it;

		for (it = m_light_cfg_all.light_cfg_vect.begin(); it != m_light_cfg_all.light_cfg_vect.end(); it++)
		{
			int num = MultiByteToWideChar(0, 0, (*it).cDID, -1, NULL, 0);
			wchar_t* section_name = new wchar_t[num];
			memset(section_name, 0, num);
			MultiByteToWideChar(0, 0, (*it).cDID, -1, section_name, num);
			m_light_cfg_all.IniFileLight.SetProfileString(section_name, _T("did"), section_name);

			wchar_t* bright = new wchar_t[5];
			wsprintfW(bright, L"%d", (*it).bright);
			m_light_cfg_all.IniFileLight.SetProfileString(section_name, _T("bright"), bright);
			delete[] bright;

			wchar_t* chroma_ctrl_flag = new wchar_t[5];
			wsprintfW(chroma_ctrl_flag, L"%d", (*it).chroma_ctrl_flag);
			m_light_cfg_all.IniFileLight.SetProfileString(section_name, _T("chroma_ctrl_flag"), chroma_ctrl_flag);
			delete[] chroma_ctrl_flag;

			wchar_t* select_group = new wchar_t[5];
			wsprintfW(select_group, L"%d", (*it).select_group);
			m_light_cfg_all.IniFileLight.SetProfileString(section_name, _T("select_group"), select_group);
			delete[] select_group;

			delete[] section_name;
		}
		m_light_cfg_all.need_persist = FALSE;
	}

}

void CChromaBroadcastSampleApplicationDlg::PersistGlobalCfg()
{
	if (m_global_cfg.need_persist == TRUE)
	{
		wchar_t* wchar_temp = new wchar_t[5];

		memset(wchar_temp, 0, 5);
		wsprintfW(wchar_temp, L"%d", m_global_cfg.bAutoStart);
		m_global_cfg.IniFileGlobal.SetProfileString(_T("global"), _T("auto_start"), wchar_temp);

		memset(wchar_temp, 0, 5);
		wsprintfW(wchar_temp, L"%d", m_global_cfg.bOpenMainUI);
		m_global_cfg.IniFileGlobal.SetProfileString(_T("global"), _T("open_main_UI"), wchar_temp);


		delete[] wchar_temp;

		m_global_cfg.need_persist = FALSE;
	}

}

void CChromaBroadcastSampleApplicationDlg::ProcPreviewTimer()
{
	CUdpLight* pFoundLight = NULL;
	vector<CUdpLight*>::iterator it;

	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		pFoundLight = *it;

		if (pFoundLight->PreviewIngFlag)
		{
			pFoundLight->PreviewIngFlag = 0;
		}
	}
	KillTimer(PREVIEW_WAIT_COOLING_TIMER);
}



//Send a heartbeat packet every LIGHT_SESSION_CHECK_PERIOD ms
void CChromaBroadcastSampleApplicationDlg::LightSessionCheck()
{
	CUdpLight* pFoundLight = NULL;
	vector<CUdpLight*>::iterator it;

	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		pFoundLight = *it;
		if (pFoundLight->iSendKplCount++ > 2)
		{
			pFoundLight->udp_light_state = UDP_STATE_DISCONNECT;
			pFoundLight->iSendKplCount = 0;
		}

		switch (pFoundLight->udp_light_state)
		{
		case UDP_STATE_IDLE:
		case UDP_STATE_DISCONNECT:
			pFoundLight->Close();
			if (pFoundLight->Create(0, SOCK_DGRAM, FD_READ | FD_WRITE, NULL) == TRUE)
			{
				pFoundLight->udp_light_state = UDP_STATE_CREAT_SOCKET;
				pFoundLight->AcquireToken();//update token
			}
			break;
		case UDP_STATE_CREAT_SOCKET:
			pFoundLight->AcquireToken();//update token
			break;
		case UDP_STATE_CONNECTED:
			pFoundLight->SendKplMsg();
			break;
		default:
			break;
		}
	}
}

void CChromaBroadcastSampleApplicationDlg::BroadcastDiscoverMsg()
{
	//lan_ctrl send broadcast msg to scan device
	char sendBuf[200];
	int len;
	int ret;

	len = snprintf(sendBuf,
		sizeof(sendBuf),
		"M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1982\r\nMAN: \"ssdp:discover\"\r\nST: wifi_bulb\r\n");
	ret = pMulticast->SendTo(sendBuf, len + 1, PNP_PORT, _T("239.255.255.250"), 0);
}



bool CChromaBroadcastSampleApplicationDlg::ParseDiscoveryMSG(char* recv_data, CUdpLight& UdpLight)
{
	char p_IPPort[40] = { 0 };
	char* p_Port = NULL;

	if (FindValue(recv_data, "support:", strlen("support:") + 1, UdpLight.cSupport) == false)
	{
		return false;
	}
	if (FindValue(recv_data, "model:", 7, UdpLight.cModel) == false)
	{
		return false;
	}
	if (FindValue(recv_data, "id:", 4, UdpLight.cDID) == false)
	{
		return false;
	}
	if (FindValue(recv_data, "yeelight://", 11, p_IPPort) == false)
	{
		return false;
	}

	p_Port = strstr(p_IPPort, ":");
	if (p_Port == NULL)
	{
		return false;
	}
	memcpy(UdpLight.cIP, p_IPPort, p_Port - p_IPPort);

	p_Port += 1;
	UdpLight.iPort = atoi(p_Port);

	return true;
}


bool CChromaBroadcastSampleApplicationDlg::FindValue(char* recv_data, char* p_name, char nameLen, char* p_dst)
{
	char* p_start = NULL;
	char* p_end = NULL;

	p_start = strstr(recv_data, p_name);
	if (p_start == NULL)
	{
		return false;
	}
	p_end = strstr(p_start, "\r\n");
	if (p_end == NULL)
	{
		return false;
	}

	memcpy(p_dst, p_start + nameLen, p_end - p_start - nameLen);
	return true;
}

bool CChromaBroadcastSampleApplicationDlg::IsAlreadyFound(CUdpLight& p_UdpLight)
{
	vector<CUdpLight*>::iterator it;

	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		if (memcmp(p_UdpLight.cDID, &((*(*it)).cDID), sizeof(p_UdpLight.cDID)) == 0)
		{
			return true;
		}
	}

	return false;
}

//Filter out unsupported devices
bool CChromaBroadcastSampleApplicationDlg::IsSupportUDP(CUdpLight& UdpLight)
{
	if (strstr(UdpLight.cSupport, "chroma") == NULL)
	{
		return false;
	}
	if (strstr(UdpLight.cSupport, "udp_sess_new") != NULL)
	{
		UdpLight.cProtocolVer = 2;
	}
	else if (strstr(UdpLight.cSupport, "udp_new") != NULL)
	{
		UdpLight.cProtocolVer = 1;
	}
	if (strstr(UdpLight.cSupport, "bg_set_rgb") != NULL)
	{
		UdpLight.cDeviceType = 1;//bg_device

		if (strstr(UdpLight.cSupport, "bg_set_scene") != NULL)
		{
			UdpLight.cDeviceType = 2;//bg_device enable bg_set_scene (diable set_scene_bundle)
		}
	}

	return true;
}

bool CChromaBroadcastSampleApplicationDlg::RefreshLightList(CUdpLight& UdpLight)
{
	//connect to the new device
	if (UdpLight.bConnectStat == true)
	{
		return false;
	}
	CString strIP(UdpLight.cIP);
	if (UdpLight.Create(0, SOCK_DGRAM, FD_READ | FD_WRITE, NULL) == TRUE)
	{
		UdpLight.bConnectStat = true;
		UdpLight.udp_light_state = UDP_STATE_CREAT_SOCKET;
	}
	else
	{
	}
	return true;
}



bool CChromaBroadcastSampleApplicationDlg::FindLightByEchoName(char* echoName, CUdpLight** FoundLight)
{
	char DIDSuffix[5] = { 0 };
	char* p_line = strstr(echoName, "-");
	if (p_line == NULL)
	{
		return false;
	}
	memcpy(DIDSuffix, p_line + 1, 4);

	vector<CUdpLight*>::iterator it;

	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		if (strstr((char*)((*(*it)).cDID), DIDSuffix) != NULL)
		{
			*FoundLight = *it;
			return true;
		}
	}

	return false;
}

bool CChromaBroadcastSampleApplicationDlg::HasOnlineUdpLight()
{
	if (udp_light_vect.size() < 1)
	{
		return false;
	}
	vector<CUdpLight*>::iterator it;

	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		if (((*(*it)).udp_light_state) == UDP_STATE_CONNECTED)
		{
			return true;
		}
	}

	return false;
}

void CChromaBroadcastSampleApplicationDlg::ProcBrightnessSliderMSG(char index)
{
	//update the brightness of the light, and send it out
	CUdpLight* selUdpLight = NULL;
	if (FindLightByPage(index, m_Currentpage, &selUdpLight))
	{
		if (selUdpLight->ucBright == pBrightnessSlider[index]->GetPos())
		{
			return;
		}
		selUdpLight->ucBright = pBrightnessSlider[index]->GetPos();
		SendBrightneesToLight(*selUdpLight);
		UpdateLightCfg(*selUdpLight);
	}

	CString temp;
	temp.Format(_T("%d"), pBrightnessSlider[index]->GetPos());
	SetDlgItemText(IDC_BRIGHTNESS_EDIT + index, temp);
}

void CChromaBroadcastSampleApplicationDlg::SendBrightneesToLight(CUdpLight& UdpLight)
{
	char sendBuf[200];
	int len;

	char method_name[30] = { 0 };

	switch (UdpLight.cDeviceType)
	{
	case 0:
		strcpy_s(method_name, "set_bright");
		break;
	case 1:
	case 2:
		strcpy_s(method_name, "bg_set_bright");
		break;
	default:
		strcpy_s(method_name, "set_bright");
		break;
	}


	len = snprintf(sendBuf,
		sizeof(sendBuf),
		"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [%d, \"sudden\", 0], \"token\":\"%s\" }\r\n", UdpLight.msg_id, method_name, UdpLight.ucBright, UdpLight.cToken);

	CString strIP(UdpLight.cIP);
	int ret = UdpLight.SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (UdpLight.msg_id++ < 0)
	{
		UdpLight.msg_id = 1;
	}
}

void CChromaBroadcastSampleApplicationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

		if (pSlider->GetDlgCtrlID() < IDC_BRIGHTNESS_SLIDER || pSlider->GetDlgCtrlID() > (IDC_BRIGHTNESS_SLIDER + MAX_LIGHTS_PER_PAGE - 1))
		{
			return;
		}

		ProcBrightnessSliderMSG(pSlider->GetDlgCtrlID() - IDC_BRIGHTNESS_SLIDER);
	}
}


void CChromaBroadcastSampleApplicationDlg::OnBnClickedCtrl(UINT uID)
{
	bool check = pCtrlButton[uID - IDC_CTRL_BTN]->GetChecked();

	CUdpLight* selUdpLight = NULL;
	if (FindLightByPage(uID - IDC_CTRL_BTN, m_Currentpage, &selUdpLight))
	{
		selUdpLight->bAllowCtrlFlag = check;
		UpdateLightCfg(*selUdpLight);
	}
}

void CChromaBroadcastSampleApplicationDlg::OnBnClickedPage(UINT uID)
{
	bool need_refresh = FALSE;
	switch (uID)
	{
	case IDC_PRE_BTN:
		if (m_Currentpage > 1)
		{
			m_Currentpage--;
			need_refresh = TRUE;
		}
		break;
	case IDC_NEXT_BTN:
		if (m_Currentpage < m_TotalPage)
		{
			m_Currentpage++;
			need_refresh = TRUE;
		}
		break;
	default:
		break;
	}

	if (need_refresh == FALSE)
	{
		return;
	}
	ClearLightDetail();

	RefreshLightShowPage();
}

void CChromaBroadcastSampleApplicationDlg::RefreshLightShowPage()
{
	int need_show_count = 0;
	int start_index = 0;//range [0 udp_light_vect.size()-1]
	start_index = (m_Currentpage - 1) * MAX_LIGHTS_PER_PAGE;

	if ((udp_light_vect.size() - start_index) >= MAX_LIGHTS_PER_PAGE)
	{
		need_show_count = MAX_LIGHTS_PER_PAGE;
	}
	else
	{
		need_show_count = (int)udp_light_vect.size() - start_index;
	}

	CUdpLight* pFoundLight[MAX_LIGHTS_PER_PAGE] = { NULL };
	vector<CUdpLight*>::iterator it;

	int vect_index = 0;
	int light_index = 0;
	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		if (vect_index++ == start_index)
		{
			pFoundLight[light_index] = *it;

			if (light_index++ < (need_show_count - 1))
			{
				start_index++;
			}
		}
	}

	for (int i = 0; i < need_show_count; i++)
	{
		if (pFoundLight[i] == NULL)
		{
			break;
		}
		DrawLight(i, *(pFoundLight[i]));
	}

	CString strPageCurrent;
	strPageCurrent.Format(_T("    ""%d"""), m_Currentpage);

	CString strPageCount;
	strPageCount.Format(_T("""%d""\/""%d"""), m_Currentpage, m_TotalPage);
	GetDlgItem(IDC_PAGE_COUNT)->SetWindowTextW(strPageCount);


	if (m_Currentpage == m_TotalPage)
	{
		pPageIconNext->SetState(CTRL_DISABLE);
		pPageIconPreview->SetState(CTRL_NOFOCUS);
		if (m_Currentpage == 1)
		{
			pPageIconPreview->SetState(CTRL_DISABLE);
		}
	}
	else if (m_Currentpage < m_TotalPage)
	{
		pPageIconNext->SetState(CTRL_NOFOCUS);
		if (m_Currentpage == 1)
		{
			pPageIconPreview->SetState(CTRL_DISABLE);
		}
		else
		{
			pPageIconPreview->SetState(CTRL_NOFOCUS);
		}
	}
}

void CChromaBroadcastSampleApplicationDlg::ClearLightDetail()
{
	for (int i = 0; i < MAX_LIGHTS_PER_PAGE; i++)
	{
		if (pLightPic[i] != NULL)
		{
			delete pLightPic[i];
			pLightPic[i] = NULL;
		}
		if (pLightName[i] != NULL)
		{
			delete pLightName[i];
			pLightName[i] = NULL;
		}
		if (pLightSelectGroup[i] != NULL)
		{
			delete pLightSelectGroup[i];
			pLightSelectGroup[i] = NULL;
		}
		if (pSelectGroupBtn[i] != NULL)
		{
			delete pSelectGroupBtn[i];
			pSelectGroupBtn[i] = NULL;
		}
		if (pBrightness[i] != NULL)
		{
			delete pBrightness[i];
			pBrightness[i] = NULL;
		}
		if (pBrightnessSlider[i] != NULL)
		{
			delete pBrightnessSlider[i];
			pBrightnessSlider[i] = NULL;
		}
		if (pBrightnessEdit[i] != NULL)
		{
			delete pBrightnessEdit[i];
			pBrightnessEdit[i] = NULL;
		}
		if (pBrightnessStatic[i] != NULL)
		{
			delete pBrightnessStatic[i];
			pBrightnessStatic[i] = NULL;
		}
		if (pCtrlButton[i] != NULL)
		{
			delete pCtrlButton[i];
			pCtrlButton[i] = NULL;
		}
		if (pPreviewButton[i] != NULL)
		{
			delete pPreviewButton[i];
			pPreviewButton[i] = NULL;
		}
	}
}


void CChromaBroadcastSampleApplicationDlg::OnBnClickedPreview(UINT uID)
{
	CUdpLight* selUdpLight = NULL;
	if (FindLightByPage(uID - IDC_PREVIEW_BTN, m_Currentpage, &selUdpLight))
	{
		selUdpLight->PreviewIngFlag = 1;
		selUdpLight->SendCtrlMsgPreview();
		SetTimer(PREVIEW_WAIT_COOLING_TIMER, PREVIEW_WAIT_COOLING_PERIOD, NULL);
	}
}

void CChromaBroadcastSampleApplicationDlg::OnCbnSelChangeGroup(UINT uID)
{
	CString strGroup;
	int nSel = 0;

	nSel = pSelectGroupBtn[uID - IDC_SELECT_GROUP]->GetCurSel();

	if (nSel < 0 || nSel > 4)
	{
		return;
	}

	CUdpLight* selUdpLight = NULL;
	if (FindLightByPage(uID - IDC_SELECT_GROUP, m_Currentpage, &selUdpLight))
	{
		selUdpLight->cMappingLED = nSel + 1;
		UpdateLightCfg(*selUdpLight);
	}
}

bool CChromaBroadcastSampleApplicationDlg::FindLightByPage(int index, int current_page, CUdpLight** FoundLight)
{
	if (index < 0 || index > MAX_LIGHTS_PER_PAGE - 1)//index range: [0 MAX_LIGHTS_PER_PAGE - 1]
	{
		return false;
	}

	if (current_page < 0 || current_page > m_TotalPage)
	{
		return false;
	}
	index = index + MAX_LIGHTS_PER_PAGE * (current_page - 1);

	vector<CUdpLight*>::iterator it;

	int vect_index = 0;
	for (it = udp_light_vect.begin(); it != udp_light_vect.end(); it++)
	{
		if (vect_index++ == index)
		{
			*FoundLight = *it;
			return true;
		}
	}

	return false;
}


void CChromaBroadcastSampleApplicationDlg::OnBnClickedGuidance()
{
	//CGuidanceDlg m_pGuidanceDlg;
	//m_pGuidanceDlg.DoModal();
	if (pGuidanceDlg != NULL)
	{
		return;
	}

	pGuidanceDlg = new CGuidanceDlg();

	if (!pGuidanceDlg) //Create failed.
	{
		AfxMessageBox(_T("Error creating Dialog"));
		delete pGuidanceDlg;
		return;
	}
	pGuidanceDlg->SetParentDialog(this);
	BOOL ret = pGuidanceDlg->Create(CGuidanceDlg::IDD, this);
	if (!ret) //Create failed.
	{
		//AfxMessageBox(_T("Error creating Dialog"));
		delete pGuidanceDlg;
	}

	CRect rect_guidance;
	CRect rect_parent_win;
	CRect rect_parent_client;
	//Calculate the coordinates of the child window according to the coordinates of the parent window
	GetWindowRect(&rect_parent_win);
	GetClientRect(&rect_parent_client);
	int h_title = rect_parent_win.Height() - rect_parent_client.Height();
	int w_title = rect_parent_win.Width() - rect_parent_client.Width();

	//ClientToScreen(&rect_parent_win);
	rect_guidance.left = rect_parent_win.left + w_title / 2 + 634;
	rect_guidance.top = rect_parent_win.top + h_title + 82;
	rect_guidance.right = rect_guidance.left + 360 + w_title;
	rect_guidance.bottom = rect_guidance.top + 545 + h_title;

	pGuidanceDlg->MoveWindow(rect_guidance);
	pGuidanceDlg->ShowWindow(SW_SHOW);
}

//Respond to the carriage return event of the edit box to prevent the program from exiting
BOOL CChromaBroadcastSampleApplicationDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN) {
		if (GetFocus()->GetDlgCtrlID() >= IDC_BRIGHTNESS_EDIT &&
			GetFocus()->GetDlgCtrlID() <= (IDC_BRIGHTNESS_EDIT + MAX_LIGHTS_PER_PAGE - 1))
		{
			return TRUE;
		}
	}
	else if (pMsg->message == 647)
	{
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}



LRESULT CChromaBroadcastSampleApplicationDlg::OnTrayMsg(WPARAM wparam, LPARAM lparam)
{
	if (wparam != IDR_MAINFRAME)
		return    1;
	switch (lparam)
	{
	case WM_RBUTTONUP://A shortcut menu pops up when right-clicking
	{
		CPoint point;
		CMenu    menu;
		CMenu* pSubMenu;

		menu.LoadMenu(IDR_MENU1);
		pSubMenu = menu.GetSubMenu(0);
		GetCursorPos(&point);
		SetForegroundWindow();
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);

		//Recycle
		HMENU hmenu = menu.Detach();
		menu.DestroyMenu();
	}
	break;
	case WM_LBUTTONUP://Left click processing
	{
		this->ShowWindow(SW_SHOWNORMAL);
		this->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//Make the window always on top
		this->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//Make the window can be covered by other windows
	}
	break;
	}
	return 0;
}

void CChromaBroadcastSampleApplicationDlg::setTray()
{
	m_notify.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_notify.hWnd = this->m_hWnd;
	m_notify.uID = IDR_MAINFRAME;
	m_notify.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_notify.uCallbackMessage = WM_USER_NOTIFYICON;
	m_notify.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON1));
	wcscpy_s(m_notify.szTip, _T("YeelightChromaConnector")); //Information Prompt
	Shell_NotifyIcon(NIM_ADD, &m_notify); //Add an icon in the tray area

	if (m_global_cfg.bOpenMainUI == FALSE)//Hide the main interface
	{
		//ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
		ShowWindow(SW_MINIMIZE);//Using only this line of will cause the taskbar to still be displayed
		PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);//With the above, only using this will cause the dialog box to flash out first, and then minimize
	}
}



void CChromaBroadcastSampleApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);
	if (nID == SC_MINIMIZE)
	{
		Shell_NotifyIcon(NIM_ADD, &m_notify);
		ShowWindow(SW_HIDE);
	}
}


void CChromaBroadcastSampleApplicationDlg::OnTrayExit()
{
	DestroyWindow();
}


void CChromaBroadcastSampleApplicationDlg::OnTrayOpen()
{
	this->ShowWindow(SW_SHOWNORMAL);//show main interface
	this->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//Make the window always on top
	this->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//Make the window can be covered by other windows
}


void CChromaBroadcastSampleApplicationDlg::OnTrayAutoStart()
{
	m_global_cfg.bAutoStart = !m_global_cfg.bAutoStart;

	HKEY hKey;
	CString strRegPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

	if (m_global_cfg.bAutoStart)
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, strRegPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
		{
			TCHAR szModule[MAX_PATH];
			GetModuleFileName(NULL, szModule, MAX_PATH);//Get the full path of the program itself
			RegSetValueEx(hKey, _T("Yeelight Chroma Connector"), 0, REG_SZ, (LPBYTE)szModule, (lstrlen(szModule) + 1) * sizeof(TCHAR));
			//Add a child key and set the value,"Yeelight Chroma Connector"is the name of the application(no suffix .exe)
			RegCloseKey(hKey); //Close the registry
		}
		else
		{
			AfxMessageBox(_T("Error can't set auto start"));
		}
	}
	else
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, strRegPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
		{
			RegDeleteValue(hKey, _T("Yeelight Chroma Connector"));
			RegCloseKey(hKey);
		}
	}

	m_global_cfg.need_persist = TRUE;
}


void CChromaBroadcastSampleApplicationDlg::OnUpdateTrayAutoStart(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_global_cfg.bAutoStart);
	//GetMenu()->GetSubMenu(0)->CheckMenuItem(ID_TRAY_AUTO_START, MF_BYCOMMAND | MF_CHECKED);
}

void CChromaBroadcastSampleApplicationDlg::OnTrayOpenMainUI()
{
	m_global_cfg.bOpenMainUI = !m_global_cfg.bOpenMainUI;
	m_global_cfg.need_persist = TRUE;
}

void CChromaBroadcastSampleApplicationDlg::OnUpdateTrayOpenMainUI(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_global_cfg.bOpenMainUI);
}

void CChromaBroadcastSampleApplicationDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child Windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}



//Get all the network card information
bool CChromaBroadcastSampleApplicationDlg::GetAllAdaptersInfo(IP_ADAPTER_INFO** pAdapter)
{
	CString ValidIPWLAN;
	CString ValidIPEthernet;

	char FindValidIPEthernet = 0;
	char FindValidIPWLAN = 0;

	PIP_ADAPTER_INFO pAdapterInfo;
	DWORD errValue = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (!pAdapterInfo)
		return false;
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		if (!pAdapterInfo)
			return false;
	}
	CString info;
	if ((errValue = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		*pAdapter = pAdapterInfo;
		return true;
	}
	else
	{
		return false;
	}
}


//Get next network card information
bool CChromaBroadcastSampleApplicationDlg::GetNextValidAdapterInfo(IP_ADAPTER_INFO** pAdapter)
{
	while (true)
	{
		if (g_pCurrentAdapter == NULL)
		{
			g_pCurrentAdapter = g_pAdapter;
		}
		else
		{
			g_pCurrentAdapter = g_pCurrentAdapter->Next;
		}


		if (g_pCurrentAdapter)
		{
			if (strcmp(g_pCurrentAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				*pAdapter = g_pCurrentAdapter;
				return true;
			}
			//continue to check next adapter
		}
		else // loop to the end of list
		{
			free(g_pAdapter);//update g_Adapter then try again
			GetAllAdaptersInfo(&g_pAdapter);
			g_pCurrentAdapter = g_pAdapter;
			if (strcmp(g_pCurrentAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				*pAdapter = g_pCurrentAdapter;
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

//Get next network card information
bool CChromaBroadcastSampleApplicationDlg::UpdateMulticastIF()
{
	int ret;

	IP_ADAPTER_INFO* pAdapterInfo;
	if (GetNextValidAdapterInfo(&pAdapterInfo) == true)
	{
		struct in_addr addr;
		inet_pton(AF_INET, pAdapterInfo->IpAddressList.IpAddress.String, &(addr.s_addr));

		//Choose a valid network card
		ret = pMulticast->SetSockOpt(IP_MULTICAST_IF, &addr, sizeof(addr), IPPROTO_IP);
		if (ret < 0)
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}
