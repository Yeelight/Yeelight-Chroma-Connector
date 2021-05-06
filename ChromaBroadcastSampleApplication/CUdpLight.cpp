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
#include "CUdpLight.h"
//#include <json/json.h>


CUdpLight::CUdpLight()
{
	memset(cModel, 0, sizeof(cModel));
	memset(cDID, 0, sizeof(cDID));
	memset(cIP, 0, sizeof(cIP));
	memset(cToken, 0, sizeof(cToken));
	msg_id = 1;
	iPort = 0;
	ucBright = 100;
	SwitchStatus = 0;//default status on
	bAllowCtrlFlag = TRUE;
	bConnectStat = FALSE;
	cMappingLED = 1;
	iSendKplCount = 0;
	udp_light_state = UDP_STATE_IDLE;
	memset(cSupport, 0, sizeof(cSupport));
	cProtocolVer = 1;
	cDeviceType = 0;
	PreviewIngFlag = 0;
}

CUdpLight::~CUdpLight()
{
}


void CUdpLight::OnClose(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		;
	}
	CAsyncSocket::OnClose(nErrorCode);
}



void CUdpLight::OnOutOfBandData(int nErrorCode)
{
	CAsyncSocket::OnOutOfBandData(nErrorCode);
}


void CUdpLight::OnReceive(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		char pBuf[1025] = {0};

		int iLen = 0;
		SOCKADDR fromSocket;
		int addrLen = sizeof(SOCKADDR);
		iLen = this->ReceiveFrom(pBuf, 1024, &fromSocket, &addrLen, 0);
		if (iLen == SOCKET_ERROR)
		{
			printf("recv:%s\r\n", pBuf);
		}
		else
		{
			ProcRecvData(pBuf);
			printf("recv:%s\r\n", pBuf);
		}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}


void CUdpLight::OnSend(int nErrorCode)
{
	CAsyncSocket::OnSend(nErrorCode);
}


void CUdpLight::ProcRecvData(char* recv_data)
{
	//todo use json lib
	//Json::Value root;
	//Json::Reader reader;
	//proc msg = {"id":2,"method":"udp_token","params":{"token":"1420cc0cfd377cf441181585db77611c"}}

	char method_name[30] = { 0 };

	switch (this->cProtocolVer)
	{
	case 1:
		strcpy_s(method_name, "udp_token");
		break;
	case 2:
		strcpy_s(method_name, "udp_sess_token");
		break;
	default:
		break;
	}

	if (strstr(recv_data, method_name) != NULL)
	{
		char* p_token = NULL;
		p_token = strstr(recv_data, "token\":");
		if (p_token == NULL)
		{
			return;
		}
		memcpy(this->cToken, p_token + 8, 32);
		this->iSendKplCount = 0;
		this->udp_light_state = UDP_STATE_CONNECTED;
	}
	else if (strstr(recv_data, "token") != NULL)
	{
		if (strstr(recv_data, "first") != NULL)
		{
			return;
		}
		char* p_token = NULL;
		p_token = strstr(recv_data, "token\":");
		if (p_token == NULL)
		{
			return;
		}
		else if (strstr(p_token, this->cToken) != NULL)
		{
			this->iSendKplCount = 0;
		}
	}
}

bool CUdpLight::AcquireToken()
{
	//is connected to the device
	if (this->bConnectStat == false)
	{
		return false;
	}

	//send acquire msg to the device
	char sendBuf[200] = {0};
	int len;
	char method_name[30] = {0};

	switch (this->cProtocolVer)
	{
	case 1:
		strcpy_s(method_name, "udp_new");
		break;
	case 2:
		strcpy_s(method_name, "udp_sess_new");
		break;
	default:
		break;
	}

	len = snprintf(sendBuf,
		sizeof(sendBuf),
		"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [] }\r\n", this->msg_id, method_name);

	CString strIP(this->cIP);
	int ret = this->SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (this->msg_id++ < 0)
	{
		this->msg_id = 1;
	}

	return true;
}


void CUdpLight::SendKplMsg()
{
	//is connected to the device
	if (this->bConnectStat == false)
	{
		return;
	}

	//send keep alive msg to the device
	char sendBuf[200];
	int len;

	char method_name[30] = { 0 };

	switch (this->cProtocolVer)
	{
	case 1:
		strcpy_s(method_name, "udp_keep_alive");
		break;
	case 2:
		strcpy_s(method_name, "udp_sess_keep_alive");
		break;
	default:
		break;
	}

	len = snprintf(sendBuf,
		sizeof(sendBuf),
		"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [\"keeplive_interval\",\"10\"],\"token\":\"%s\" }\r\n", this->msg_id, method_name, this->cToken);

	CString strIP(this->cIP);
	int ret = this->SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (this->msg_id++ < 0)
	{
		this->msg_id = 1;
	}
}

bool CUdpLight::IsAllowedSendCtrlMsg()
{
	bool ret = false;

	if (this->bConnectStat != TRUE)
	{
		return ret;
	}

	if (this->udp_light_state != UDP_STATE_CONNECTED)
	{
		return ret;
	}

	if (this->bAllowCtrlFlag != TRUE)
	{
		return ret;
	}

	if (this->cToken[0] == 0)
	{
		return ret;
	}

	if (this->PreviewIngFlag == 1)
	{
		return ret;
	}

	return true;
}


void CUdpLight::SendCtrlMsgSwitchPower(bool SwitchStatus, char* effect, int duration)
{
	char method_name[30] = { 0 };
	char sendBuf[200];
	int len;

	if (this->SwitchStatus == 1)//no need to send power off again
	{
		return;
	}
	switch (this->cDeviceType)
	{
	case 0://normal device
		strcpy_s(method_name, "set_power");
		break;
	case 1://bg device
	case 2://bg_device enable bg_set_scene (diable set_scene_bundle)
		strcpy_s(method_name, "bg_set_power");
		break;
	default:
		break;
	}
	len = snprintf(sendBuf, sizeof(sendBuf),
		"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [\"off\", \"%s\", %d], \"token\":\"%s\" }\r\n",
		this->msg_id,
		method_name,
		effect,
		duration,
		this->cToken);

	this->SwitchStatus = 1;//off

	CString strIP(this->cIP);
	int ret = this->SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (this->msg_id++ < 0)
	{
		this->msg_id = 1;
	}
}

//{"id":1,"method":"set_scene","params":["color",65537,100,1000,"smooth"]}
void CUdpLight::SendCtrlMsgSetScene(COLORREF color, int bright, char* effect, int duration)
{
	char method_name[30] = { 0 };
	char sendBuf[200];
	int len;

	switch (this->cDeviceType)
	{
	case 0:
		strcpy_s(method_name, "set_scene");
		//{"id":1,"method":"set_scene","params":["color",65537,100,1000,"smooth"]}
		len = snprintf(sendBuf, sizeof(sendBuf),
			"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [\"color\",%d, %d, %d, \"%s\"], \"token\":\"%s\" }\r\n",
			this->msg_id,
			method_name,
			color,
			bright,
			duration,
			effect,
			this->cToken);
		break;
	case 1:
		if (this->SwitchStatus == 1)//if off first open it
		{
			strcpy_s(method_name, "bg_set_power");
			len = snprintf(sendBuf,
				sizeof(sendBuf),
				"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [\"on\"], \"token\":\"%s\" }\r\n",
				this->msg_id,
				method_name,
				this->cToken);
		}
		else
		{
			strcpy_s(method_name, "bg_set_rgb");
			len = snprintf(sendBuf,
				sizeof(sendBuf),
				"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [%d, \"sudden\", 0], \"token\":\"%s\" }\r\n",
				this->msg_id,
				method_name,
				color,
				this->cToken);
		}
		break;
	case 2:
		strcpy_s(method_name, "bg_set_scene");
		//{"id":1,"method":"set_scene","params":["color",65537,100,1000,"smooth"]}
		len = snprintf(sendBuf, sizeof(sendBuf),
			"{ \"id\":%d, \"method\" : \"%s\", \"params\" : [\"color\",%d, %d, %d, \"%s\"], \"token\":\"%s\" }\r\n",
			this->msg_id,
			method_name,
			color,
			bright,
			duration,
			effect,
			this->cToken);
		break;
	default:
		break;
	}

	this->SwitchStatus = 0;//on

	CString strIP(this->cIP);
	int ret = this->SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (this->msg_id++ < 0)
	{
		this->msg_id = 1;
	}
}

void CUdpLight::SendCtrlMsgPreview()
{
	char sendBuf[200];
	int len;

	len = snprintf(sendBuf,
		sizeof(sendBuf),
		"{ \"id\":%d, \"method\" : \"set_scene\", \"params\" : [\"cf\", 6, 0, \"600, 2, 4000, 70, 400, 2, 4000, 1\"], \"token\":\"%s\" }\r\n",
		this->msg_id,
		this->cToken);

	CString strIP(this->cIP);
	int ret = this->SendTo(sendBuf, (int)strlen(sendBuf), LAN_SERVER_UDP_PORT, strIP, 0);

	if (this->msg_id++ < 0)
	{
		this->msg_id = 1;
	}
}
