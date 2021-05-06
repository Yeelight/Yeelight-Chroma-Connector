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

#include "CMulticast.h"
#include "CUdpLight.h"
#include<vector>
#include "Public.h"
#include "CGuidanceDlg.h"
#include "PngComboBox.h"
#include "CPngCheckBox.h"
#include "PngButton.h"
#include "CCustomSliderCtrl2.h"
#include "IniFile.h"
#include "CPng.h"

// ChromaBroadcastSampleApplicationDlg.h : header file
//

#pragma once
using namespace std;


#define USE_ADV_PIC      0

#if USE_ADV_PIC
#define MAX_LIGHTS_PER_PAGE 3
#else
#define MAX_LIGHTS_PER_PAGE 4
#endif

#define MAX_LIGHTS_SUPPORT 10

typedef struct
{
	char cDID[32];
	int bright;
	int select_group;
	int chroma_ctrl_flag;
}st_light_cfg;

typedef struct
{
	CIniFile IniFileLight;
	CStringArray light_cfg_sections;
	vector<st_light_cfg> light_cfg_vect;
	bool need_persist;
}st_light_cfg_all;

typedef struct
{
	CIniFile IniFileGlobal;
	CStringArray global_cfg_sections;
	bool bAutoStart;
	bool bOpenMainUI;
	bool need_persist;
}st_global_cfg;

// CChromaBroadcastSampleApplicationDlg dialog
class CChromaBroadcastSampleApplicationDlg : public CDialogEx
{
// Construction
public:
	CChromaBroadcastSampleApplicationDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CChromaBroadcastSampleApplicationDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHROMABROADCASTSAMPLEAPPLICATION_DIALOG };
#endif
	//lan_ctrl func
	void OnReceive();
	void OnMulticastClose();
	void OnConnect();
	CGuidanceDlg* pGuidanceDlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
	//lan_ctrl func
	void ResizeMainDlg();
	void InitLanCtrlUdp(char* LocalIP);
	void InitLightCfg(void);
	void InitGlobalCfg(void);
	bool LoadLightCfg(CUdpLight& UdpLight);
	void AddLightCfg(CUdpLight& UdpLight);
	void PersistLightCfg(void);
	void PersistGlobalCfg();
	void ProcPreviewTimer();
	void LightSessionCheck(void);
	void BroadcastDiscoverMsg(void);
	void UpdateLightCfg(CUdpLight& UdpLight);
	void GrawWndBg(CPaintDC& dc, CDC& dcMem);
	void ProcRecvData(char* recv_data);
	bool ParseDiscoveryMSG(char* recv_data, CUdpLight& UdpLight);
	bool FindValue(char* recv_data, char* p_name, char nameLen, char* p_dst);
	bool IsAlreadyFound(CUdpLight& p_UdpLight);
	bool RefreshLightList(CUdpLight& UdpLight);
	bool FindLightByEchoName(char* echoName, CUdpLight** FoundLight);
	bool HasOnlineUdpLight();
	bool FindLightByPage(int index, int current_page, CUdpLight** FoundLight);
	bool IsSupportUDP(CUdpLight& UdpLight);
	void DrawDeviceCount(CWnd* pParentWnd);

	void DrawLight(char index, CUdpLight& UdpLight);
	void DrawLightIcon(char index, CUdpLight& UdpLight);
	void DrawLightName(char index, CUdpLight& UdpLight);
	void DrawSelectGroup(char index, CUdpLight& UdpLight);
	void DrawBrightnessCtrl(char index, CUdpLight& UdpLight);
	void DrawBtnCtrlChroma(char index, CUdpLight& UdpLight);
	void DrawBtnPreview(char index, CUdpLight& UdpLight);

	void DrawYeelightLogo();
	void DrawGuidanceButton();

	void DrawRazerLogo();
	void Draw404NoDevice();
	void Clear404NoDevice();
	void DrawLoadingIcon();
	void DrawLine();
	void DrawPageInfo();
	void ProcBrightnessSliderMSG(char index);
	void SendBrightneesToLight(CUdpLight& UdpLight);
	void GrawLightBgGgiPlus(char index);
	void GrawLightBgBitmap(CPaintDC& DC, CDC& CDC, char index);
	void ClearLightDetail();
	void RefreshLightShowPage();

	CMulticast * pMulticast;
	CUdpLight m_UdpLight;
	CFont m_FontName;
	CFont m_FontNormal;

	int m_DialogWidth;//The actual width of the window
	int m_DialogHeight;//Actual height of the window
	//int m_baseunitX;
	//int m_baseunitY;
	float m_zoom;//Due to different resolutions and dpi scaling settings, the pixel scaling value is calculated

	CPng* pLightPic[MAX_LIGHTS_PER_PAGE];
	CStatic* pLightName[MAX_LIGHTS_PER_PAGE];
	CStatic* pLightSelectGroup[MAX_LIGHTS_PER_PAGE];
	CPngComboBox* pSelectGroupBtn[MAX_LIGHTS_PER_PAGE];
	CStatic* pBrightness[MAX_LIGHTS_PER_PAGE];
	CCustomSliderCtr2* pBrightnessSlider[MAX_LIGHTS_PER_PAGE];
	CEdit* pBrightnessEdit[MAX_LIGHTS_PER_PAGE];
	CStatic* pBrightnessStatic[MAX_LIGHTS_PER_PAGE];//Brightness unit character %
	CPngCheckBox* pCtrlButton[MAX_LIGHTS_PER_PAGE+1];
	CPngButton* pPreviewButton[MAX_LIGHTS_PER_PAGE];

	CPng* pYeelightLogo;
	CPng* pLineButton;
	CPngButton* pGuidanceButton;
	CPng* pLoadingButton;
	CStatic* pDeviceList;
	CPng* pRazerLogo;
	CPngButton* pPageIconPreview;
	CPngButton* pPageIconNext;
	CStatic* pPageCount;

	int m_TotalPage;
	int m_Currentpage;


	CPng* pNoDevice;

	SCROLLINFO vscrollinfo;
	st_light_cfg_all m_light_cfg_all;
	st_global_cfg m_global_cfg;
	CString m_LocalIP;
	CString m_LocalPath;

	NOTIFYICONDATA m_notify;
	afx_msg LRESULT OnTrayMsg(WPARAM wparan, LPARAM lparam); //Handle tray msg
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	//Minimize button response
	afx_msg void setTray();	//set tray icon

// Implementation
protected:
	//GDI+ init
	ULONG_PTR m_Taken;
	GdiplusStartupInput m_Input;

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
    afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
public:
	void AdaptDpi(CRect& rect_dpi, CRect& rect_old);
    void SetBroadcastStatus(CString message);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	CEdit m_messagecontrol;//Message reminder and receiving box
	CString	m_receivedData;
	vector<CUdpLight*> udp_light_vect;//Searched device container
	CSliderCtrl m_slider;
	afx_msg void OnBnClickedGuidance();
	afx_msg void OnBnClickedCtrl(UINT uID);//light ctrl button clicked callback
	afx_msg void OnBnClickedPreview(UINT uID);//light preview button clicked callback
	afx_msg void OnCbnSelChangeGroup(UINT uID);
	afx_msg void OnEnChangeBrightnessEdit(UINT uID);
	afx_msg void OnBnClickedPage(UINT uID);
	afx_msg void OnTrayExit();
	afx_msg void OnTrayOpen();
	afx_msg void OnTrayAutoStart();
	afx_msg void OnUpdateTrayAutoStart(CCmdUI* pCmdUI);
	afx_msg void OnTrayOpenMainUI();
	afx_msg void OnUpdateTrayOpenMainUI(CCmdUI* pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

	bool GetAllAdaptersInfo(IP_ADAPTER_INFO** pAdapterInfo);

	bool GetNextValidAdapterInfo(IP_ADAPTER_INFO** pAdapter);

	bool UpdateMulticastIF();

};
