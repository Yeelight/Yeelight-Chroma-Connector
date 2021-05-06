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

#pragma once
#include <afxsock.h>

#define LAN_SERVER_UDP_PORT 55444

typedef enum {
	UDP_STATE_IDLE = 0,//default status
	UDP_STATE_CREAT_SOCKET,//after creat soket
	UDP_STATE_CONNECTED,//after get token
	UDP_STATE_DISCONNECT//after can not recv resp of kplv msg for 4 times
}udp_light_state_t;

class CUdpLight :
    public CAsyncSocket
{
public:
	CUdpLight();
	//CUdpLight(const CUdpLight &c);
	~CUdpLight();
	virtual void OnClose(int nErrorCode);
	virtual void OnOutOfBandData(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	bool AcquireToken();
	void SendKplMsg();

	bool IsAllowedSendCtrlMsg();

	void SendCtrlMsgSwitchPower(bool SwitchStatus, char* effect, int duration);

	void SendCtrlMsgSetScene(COLORREF color, int bright, char* effect, int duration);

	void SendCtrlMsgPreview();

	char cModel[16];
	char cDID[32];
	char cIP[32];
	char cToken[33];
	unsigned int msg_id;
	int iPort;
	unsigned char ucBright;
	unsigned char SwitchStatus;//power on/off.  on:0, 1:off the same as yglc
	bool bAllowCtrlFlag;
	bool bConnectStat;//
	udp_light_state_t udp_light_state;
	char cMappingLED;
	int iSendKplCount;
	char cSupport[512];
	char cProtocolVer;//1:udp_new 2:udp_sess_new
	char cDeviceType;//0:default  1:bg_set_rgb bg_set_bright  2:bg_device enable bg_set_scene (diable set_scene_bundle)
	int  PreviewIngFlag;//Quiet for a while after pressing the preview button
private:
	void ProcRecvData(char* recv_data);
};

