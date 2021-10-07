#pragma once


#include <wlanapi.h>
#include <NetCon.h> 
#include <winsock2.h>
#include <Iphlpapi.h>

#include <functional>

namespace lqx {

	enum _SharingType {
		//将SharingType_Public的网络共享给SharingType_Private
		SharingType_None,
		SharingType_Private,
		SharingType_Public
	};

	enum _ConnectingAction {
		ConnectingAction_None,     //无动作
		ConnectingAction_Connect,  //连接
		ConnectingAction_Disconnect//断开连接
	};

	struct _HostedNetworkInfo {
		bool Active;                             //承载网络当前是否正在运行
		char SSID[32];                           //字符串最大长度（包括末尾的\0）：31字节   (亲自测试得到的结论)
		bool IsPassPhase;                        //true:密码是字符串（8到63个ascii字符 + '\0'）。false:密码是长度为32字节的二进制数据。
		char Key[64];                            //如果IsPassPhase为false，则Key不是字符串，而是长度为32字节的二进制数据。
		unsigned int NumberOfPeers;              //当前连接数
		unsigned int MaxNumberOfPeers;           //最大连接数
		DOT11_AUTH_ALGORITHM dot11AuthAlgo;      //认证方式
		DOT11_CIPHER_ALGORITHM dot11CipherAlgo;  //加密算法
	};

	struct _NetConnectionInfo {
		wchar_t *Name;     //网络连接名，比如“本地连接”（注意编码）
		wchar_t *DeviceName;//网络连接名，比如"Realtek PCIe GBE Family Controller"（注意编码）
		GUID guid;
		NETCON_STATUS Status; //当前网卡的状态
	};

	void QueryHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo);
	void SetHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo);
	void StartHostedNetwork();
	void StopHostedNetwork();
	void EnumConnections(std::function<void(_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction)> callback);
	void SetWlanPowerState(bool PowerOn);

	//调用这个函数之前要先调用 WSAStartup （一个程序只需调用一次WSAStartup)
	//成功返回true
	bool GetAdaptersInfo(std::function<void(PIP_ADAPTER_ADDRESSES pIpAdapterInfo)> callback);
	DWORD ChangeServiceStatusW(const wchar_t *Name, bool Start, int TimeOut/*单位：秒*/);

}