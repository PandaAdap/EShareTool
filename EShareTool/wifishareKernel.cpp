#include "stdafx.h"
#include "pch.h"

#include "wifishareKernel.h"
#include <winsvc.h>


#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib,"Iphlpapi.lib") 

namespace lqx {

	BOOL _wifishareKernel_Enabled = FALSE;

	void QueryHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo)
	{

		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		//获得承载网络的句柄
		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle调用失败。";
		//如果从未对承载网络进行设置，则先自动配置承载网络。如果已经配置过（比如设置了SSID，密码之类的），则保留原有设置。
		if (WlanHostedNetworkInitSettings(ClientHandle, NULL, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkInitSettings调用失败。";


		//获取网卡状态
		PWLAN_HOSTED_NETWORK_STATUS pHostedNetworkStatus = NULL;
		if (WlanHostedNetworkQueryStatus(ClientHandle, &pHostedNetworkStatus, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkQueryStatus调用失败。";
		if (pHostedNetworkStatus->HostedNetworkState != wlan_hosted_network_active)  //是否正在运行
			HostedNetworkInfo->Active = false;
		else
			HostedNetworkInfo->Active = true;
		HostedNetworkInfo->NumberOfPeers = pHostedNetworkStatus->dwNumberOfPeers;    //当前连接数
		WlanFreeMemory(pHostedNetworkStatus);


		//查看是否已启用承载网络（即netsh wlan set hostednetwork mode=allow/disallow设置的内容）
		DWORD ReturnValue;
		DWORD DataSize;
		PBOOL pEnabled = NULL;
		WLAN_OPCODE_VALUE_TYPE WlanOpcodeValueType;
		ReturnValue = WlanHostedNetworkQueryProperty(
			ClientHandle,
			wlan_hosted_network_opcode_enable,
			&DataSize,
			(PVOID*)&pEnabled,
			&WlanOpcodeValueType,
			NULL
		);
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQueryProperty调用失败。";
		_wifishareKernel_Enabled = *pEnabled;
		WlanFreeMemory(pEnabled);


		//获取当前承载网络的配置情况
		ULONG SSIDLengthInBytes;
		UCHAR *SSID_;
		PWLAN_HOSTED_NETWORK_CONNECTION_SETTINGS pHostedNetworkConnectionSettings = NULL;
		ReturnValue = WlanHostedNetworkQueryProperty(
			ClientHandle,
			wlan_hosted_network_opcode_connection_settings,
			&DataSize,
			(PVOID*)&pHostedNetworkConnectionSettings,
			&WlanOpcodeValueType,
			NULL
		);

		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQueryProperty调用失败。";
		SSIDLengthInBytes = pHostedNetworkConnectionSettings->hostedNetworkSSID.uSSIDLength;  //SSID长度（不包括\0）
		SSID_ = pHostedNetworkConnectionSettings->hostedNetworkSSID.ucSSID;                   //SSID字符串
		strcpy_s(HostedNetworkInfo->SSID, 32, (char*)SSID_);
		HostedNetworkInfo->MaxNumberOfPeers = pHostedNetworkConnectionSettings->dwMaxNumberOfPeers; //最大连接数
		WlanFreeMemory(pHostedNetworkConnectionSettings);


		//获得当前设置的密码
		DWORD KeyLength;
		PUCHAR KeyData;
		BOOL IsPassPhrase;
		BOOL Persistent;
		WLAN_HOSTED_NETWORK_REASON FailReason;
		ReturnValue = WlanHostedNetworkQuerySecondaryKey(
			ClientHandle,
			&KeyLength,
			&KeyData,
			&IsPassPhrase,
			&Persistent,
			&FailReason,
			NULL
		);
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQuerySecondaryKey调用失败。";
		if (IsPassPhrase) {
			HostedNetworkInfo->IsPassPhase = true;
			if (KeyData) {
				strcpy_s(HostedNetworkInfo->Key, 64, (char*)KeyData);
			}
			else {
				HostedNetworkInfo->Key[0] = '\0';
			}
		}
		else {
			if (KeyData) {
				HostedNetworkInfo->IsPassPhase = false;
				memcpy_s(HostedNetworkInfo->Key, 64, KeyData, 32);
			}
			else {
				HostedNetworkInfo->IsPassPhase = true;
				HostedNetworkInfo->Key[0] = '\0';
			}
		}
		WlanFreeMemory(KeyData);


		//获取认证方式，加密算法
		PWLAN_HOSTED_NETWORK_SECURITY_SETTINGS pHostedNetworkSecuritySettings = NULL;
		ReturnValue = WlanHostedNetworkQueryProperty(
			ClientHandle,
			wlan_hosted_network_opcode_security_settings,
			&DataSize,
			(PVOID*)&pHostedNetworkSecuritySettings,
			&WlanOpcodeValueType,
			NULL
		);
		HostedNetworkInfo->dot11AuthAlgo = pHostedNetworkSecuritySettings->dot11AuthAlgo;
		HostedNetworkInfo->dot11CipherAlgo = pHostedNetworkSecuritySettings->dot11CipherAlgo;
		WlanFreeMemory(pHostedNetworkSecuritySettings);


		WlanCloseHandle(ClientHandle, NULL);

	}



	void SetHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo)
	{

		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle调用失败。";

		//配置承载网络（SSID以及最大连接数）
		DWORD ReturnValue;
		WLAN_HOSTED_NETWORK_REASON FailReason;
		WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS HostedNetworkConnectionSettings;
		DOT11_SSID Dot11SSID;
		strcpy_s((char*)Dot11SSID.ucSSID, 32, (char*)(HostedNetworkInfo->SSID));
		Dot11SSID.uSSIDLength = strlen((char*)(HostedNetworkInfo->SSID));
		HostedNetworkConnectionSettings.dwMaxNumberOfPeers = HostedNetworkInfo->MaxNumberOfPeers;
		HostedNetworkConnectionSettings.hostedNetworkSSID = Dot11SSID;
		ReturnValue = WlanHostedNetworkSetProperty(
			ClientHandle,
			wlan_hosted_network_opcode_connection_settings,
			sizeof(HostedNetworkConnectionSettings),
			(PVOID)&HostedNetworkConnectionSettings,
			&FailReason,
			NULL
		);
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetProperty调用失败。";


		//设置密码
		if (HostedNetworkInfo->IsPassPhase) {     //密码为字符串
			ReturnValue = WlanHostedNetworkSetSecondaryKey(
				ClientHandle,
				strlen((char*)(HostedNetworkInfo->Key)) + 1,   //(这里的长度规定把'\0'算在内)
				(PUCHAR)(HostedNetworkInfo->Key),
				TRUE,
				TRUE,
				&FailReason,
				NULL
			);
		}
		else {                                    //密码为32字节的二进制数据
			ReturnValue = WlanHostedNetworkSetSecondaryKey(
				ClientHandle,
				32,
				(PUCHAR)(HostedNetworkInfo->Key),
				FALSE,
				TRUE,
				&FailReason,
				NULL
			);
		}
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetSecondaryKey调用失败。";

		WlanCloseHandle(ClientHandle, NULL);
	}



	void StartHostedNetwork()
	{
		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle调用失败。";

		DWORD ReturnValue;
		BOOL Enabled = TRUE;
		WLAN_HOSTED_NETWORK_REASON FailReason;

		//如果承载网络被禁用（即netsh wlan set hostednetwork mode=disallow的作用），则先解除禁用状态。
		if (!_wifishareKernel_Enabled) {
			ReturnValue = WlanHostedNetworkSetProperty(
				ClientHandle,
				wlan_hosted_network_opcode_enable,
				sizeof(Enabled),
				(PVOID)&Enabled,
				&FailReason,
				NULL
			);
			if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetProperty启用承载网络失败。";
		}


		//启用承载网络。使用WlanHostedNetworkForceStart，即使本程序退出，承载网络仍会继续运行。
		if (WlanHostedNetworkForceStart(ClientHandle, &FailReason, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkForceStart调用失败。";

		WlanCloseHandle(ClientHandle, NULL);
	}


	void StopHostedNetwork()
	{
		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle调用失败。";

		WLAN_HOSTED_NETWORK_REASON FailReason;
		if (WlanHostedNetworkForceStop(ClientHandle, &FailReason, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkForceStop调用失败。";

		WlanCloseHandle(ClientHandle, NULL);
	}


	void EnumConnections(std::function<void(_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction)> callback)
	{
		CoInitialize(NULL);

		CoInitializeSecurity(
			NULL,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_PKT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_NONE,
			NULL
		);

		{

			CComPtr< INetSharingManager> pNSM = NULL;
			HRESULT hr = ::CoCreateInstance(
				__uuidof(NetSharingManager),
				NULL, CLSCTX_ALL,
				__uuidof(INetSharingManager),
				(void**)&pNSM
			);
			if (!pNSM) throw L"NetSharingManager获取失败。";

			CComPtr<INetConnection> pNC = NULL;
			CComPtr<INetSharingEveryConnectionCollection> pNSECC = NULL;


			// 枚举设备(即本地连接，无线网络连接。。。)  
			hr = pNSM->get_EnumEveryConnection(&pNSECC);
			if (!pNSECC) {

				throw L"get_EnumEveryConnection失败。";
			}


			CComPtr<IEnumVARIANT> pEV = NULL;
			CComPtr<IUnknown> pUnk = NULL;
			CComPtr<INetSharingConfiguration>  pNSC = NULL;
			hr = pNSECC->get__NewEnum(&pUnk);
			if (pUnk)
			{
				hr = pUnk->QueryInterface(__uuidof(IEnumVARIANT), (void**)&pEV);
				if (!pEV) {
					throw L"QueryInterface失败。";
				}
			}
			else {

				throw L"get__NewEnum失败。";
			}


			if (pEV)
			{
				VARIANT v;
				VariantInit(&v);// 初始化 错误 类型VARIANT（是错误可捕捉）  
				BOOL bFoundIt = FALSE;

				while (S_OK == pEV->Next(1, &v, NULL))// 枚举序列中的元素 ，返回错误值  
				{
					if (V_VT(&v) == VT_UNKNOWN)// 返回位置类型  
					{
						V_UNKNOWN(&v)->QueryInterface(__uuidof(INetConnection), (void**)&pNC);  // 查询设备是否支持接口  
						if (pNC)
						{
							NETCON_PROPERTIES *pNP = NULL;
							_NetConnectionInfo NetConnectionInfo;
							_SharingType SharingType, SharingType_old;
							_ConnectingAction ConnectingAction = ConnectingAction_None;

							pNC->GetProperties(&pNP);// 获取设备属性  
							NetConnectionInfo.DeviceName = pNP->pszwDeviceName;
							NetConnectionInfo.Name = pNP->pszwName;
							NetConnectionInfo.Status = pNP->Status;
							NetConnectionInfo.guid = pNP->guidId;


							hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC, &pNSC);
							if (hr != S_OK) throw L"get_INetSharingConfigurationForINetConnection调用失败。";
							VARIANT_BOOL SharingEnabled;
							SHARINGCONNECTIONTYPE SharingConnectionType;
							pNSC->get_SharingEnabled(&SharingEnabled);
							pNSC->get_SharingConnectionType(&SharingConnectionType);
							if (!SharingEnabled) {
								SharingType_old = SharingType = SharingType_None;
							}
							else {
								if (SharingConnectionType == ICSSHARINGTYPE_PUBLIC) SharingType_old = SharingType = SharingType_Public;
								if (SharingConnectionType == ICSSHARINGTYPE_PRIVATE) SharingType_old = SharingType = SharingType_Private;
							}

							callback(&NetConnectionInfo, &SharingType, &ConnectingAction);


							switch (SharingType)
							{
							case SharingType_None:
								if (SharingType_old != SharingType_None) {
									hr = pNSC->DisableSharing();
									if (hr != S_OK) throw L"DisableSharing调用失败。";
								}
								break;
							case SharingType_Private:
								if (SharingType_old != SharingType_Private) {
									hr = pNSC->EnableSharing(ICSSHARINGTYPE_PRIVATE);
									if (hr != S_OK) throw L"EnableSharing(ICSSHARINGTYPE_PRIVATE)调用失败。";
								}
								break;
							case SharingType_Public:
								if (SharingType_old != SharingType_Public) {
									hr = pNSC->EnableSharing(ICSSHARINGTYPE_PUBLIC);
									if (hr != S_OK) throw L"EnableSharing(ICSSHARINGTYPE_PUBLIC)调用失败。";
								}
								break;
							default:
								break;
							}

							switch (ConnectingAction)
							{
							case ConnectingAction_None:
								break;
							case ConnectingAction_Connect:
								if (pNC->Connect() != S_OK) throw L"Connect调用失败";
								break;
							case ConnectingAction_Disconnect:
								if (pNC->Disconnect() != S_OK) throw L"Disconnect调用失败";
								break;
							default:
								break;
							}

						}
					}

					pNSC.Release();
					pNC.Release();
				}

			}



		}

		CoUninitialize();

	}



	void SetWlanPowerState(bool PowerOn)
	{
		DWORD dwResult = 0;
		DWORD dwMaxClient = 2;
		DWORD dwCurVersion = 0;
		HANDLE hClient = NULL;
		PWLAN_INTERFACE_INFO_LIST pIfList = NULL;

		dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanOpenHandle调用失败。";
		}

		dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanEnumInterfaces调用失败。";

		}


		DWORD DataSize;
		WLAN_PHY_RADIO_STATE *state;
		dwResult = WlanQueryInterface(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, wlan_intf_opcode_radio_state, NULL, &DataSize, (PVOID*)&state, NULL);
		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanQueryInterface调用失败。";
		}

		state->dwPhyIndex = 0;
		state->dot11SoftwareRadioState = PowerOn ? dot11_radio_state_on : dot11_radio_state_off;

		if (state->dot11HardwareRadioState == state->dot11SoftwareRadioState) return;
		dwResult = WlanSetInterface(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,
			wlan_intf_opcode_radio_state, sizeof(WLAN_PHY_RADIO_STATE), state, NULL);

		WlanFreeMemory(state);
		WlanFreeMemory(pIfList);

		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanSetInterface调用失败。";
		}

	}


	//调用这个函数之前要先调用 WSAStartup （一个程序只需调用一次WSAStartup)
	//成功返回true
	bool GetAdaptersInfo(std::function<void(PIP_ADAPTER_ADDRESSES pIpAdapterInfo)> callback)
	{
		PIP_ADAPTER_ADDRESSES pIpAdapterInfo = new IP_ADAPTER_ADDRESSES;    //存储 IP_ADAPTER_ADDRESSES 列表
		ULONG size = sizeof(IP_ADAPTER_ADDRESSES);                          //当前 pIpAdapterInfo 的大小
		ULONG ret = GetAdaptersAddresses(
			AF_UNSPEC,  //同时返回IPv4和IPv6地址
			0,          //flags
			NULL,       //Reserved
			pIpAdapterInfo,
			&size
		);

		if (ret == ERROR_BUFFER_OVERFLOW) {
			//分配的空间不够
			delete pIpAdapterInfo;
			//此时size储存了最终需要的空间
			pIpAdapterInfo = (PIP_ADAPTER_ADDRESSES)new BYTE[size];
			ret = GetAdaptersAddresses(
				AF_UNSPEC,  //同时返回IPv4和IPv6地址
				0,          //flags
				NULL,       //Reserved
				pIpAdapterInfo,
				&size
			);
		}

		if (ret == ERROR_SUCCESS || ret == ERROR_ADDRESS_NOT_ASSOCIATED) {
			while (pIpAdapterInfo)
			{
				callback(pIpAdapterInfo);
				pIpAdapterInfo = pIpAdapterInfo->Next;
			}

			delete pIpAdapterInfo;
			return true;
		}
		else {
			delete pIpAdapterInfo;
			return false;
		}


	}




	int WaitForServiceStatusW(SC_HANDLE schService, DWORD Status, int TimeOut/*单位：秒*/)
	{
		SERVICE_STATUS status;
		BOOL ret;
		int n = 0;
		while (n<TimeOut * 2) {
			Sleep(500);
			n++;
			ret = QueryServiceStatus(schService, &status);
			if (!ret) return 0;
			if (status.dwCurrentState == Status) return 1;
		}

		return 2;

	}

	DWORD ChangeServiceStatusW(const wchar_t *Name, bool Start, int TimeOut/*单位：秒*/)
	{
		SERVICE_STATUS status;
		SC_HANDLE schSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);   // 打开服务控制管
																					  //理数据库，并返回服务控制管理数据库的句柄
		int ret;
		if (schSCManager == NULL)
		{
			return FALSE;
		}
		SC_HANDLE schService = OpenServiceW(schSCManager, Name, SERVICE_ALL_ACCESS);    // 获得服务句柄
		if (schService == NULL)
		{
			return FALSE;
		}
		//QueryServiceStatus(schService, &status);   // 获得服务的当前状态
		//if (status.dwCurrentState = SERVICE_STOPPED)   // 如果服务处于停止状态，则将其状态设置为启动状态
		//	StartService(schService, 0, NULL);   //启动服务
		if (Start) {
			if (!StartServiceW(schService, 0, NULL)) {
				return GetLastError();
			}
			ret = WaitForServiceStatusW(schService, SERVICE_RUNNING, TimeOut);

		}
		else {
			if (!ControlService(schService, SERVICE_CONTROL_STOP, &status)) {
				return GetLastError();
			}
			ret = WaitForServiceStatusW(schService, SERVICE_STOPPED, TimeOut);
		}

		CloseServiceHandle(schSCManager);   // 关闭服务句柄
		CloseServiceHandle(schService);

		if (!ret) return GetLastError();
		if (ret == 2) return ERROR_SERVICE_REQUEST_TIMEOUT;
		return ERROR_SUCCESS;

	}

}


