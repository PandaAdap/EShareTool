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

		//��ó�������ľ��
		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle����ʧ�ܡ�";
		//�����δ�Գ�������������ã������Զ����ó������硣����Ѿ����ù�������������SSID������֮��ģ�������ԭ�����á�
		if (WlanHostedNetworkInitSettings(ClientHandle, NULL, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkInitSettings����ʧ�ܡ�";


		//��ȡ����״̬
		PWLAN_HOSTED_NETWORK_STATUS pHostedNetworkStatus = NULL;
		if (WlanHostedNetworkQueryStatus(ClientHandle, &pHostedNetworkStatus, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkQueryStatus����ʧ�ܡ�";
		if (pHostedNetworkStatus->HostedNetworkState != wlan_hosted_network_active)  //�Ƿ���������
			HostedNetworkInfo->Active = false;
		else
			HostedNetworkInfo->Active = true;
		HostedNetworkInfo->NumberOfPeers = pHostedNetworkStatus->dwNumberOfPeers;    //��ǰ������
		WlanFreeMemory(pHostedNetworkStatus);


		//�鿴�Ƿ������ó������磨��netsh wlan set hostednetwork mode=allow/disallow���õ����ݣ�
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
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQueryProperty����ʧ�ܡ�";
		_wifishareKernel_Enabled = *pEnabled;
		WlanFreeMemory(pEnabled);


		//��ȡ��ǰ����������������
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

		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQueryProperty����ʧ�ܡ�";
		SSIDLengthInBytes = pHostedNetworkConnectionSettings->hostedNetworkSSID.uSSIDLength;  //SSID���ȣ�������\0��
		SSID_ = pHostedNetworkConnectionSettings->hostedNetworkSSID.ucSSID;                   //SSID�ַ���
		strcpy_s(HostedNetworkInfo->SSID, 32, (char*)SSID_);
		HostedNetworkInfo->MaxNumberOfPeers = pHostedNetworkConnectionSettings->dwMaxNumberOfPeers; //���������
		WlanFreeMemory(pHostedNetworkConnectionSettings);


		//��õ�ǰ���õ�����
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
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkQuerySecondaryKey����ʧ�ܡ�";
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


		//��ȡ��֤��ʽ�������㷨
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
			throw L"WlanOpenHandle����ʧ�ܡ�";

		//���ó������磨SSID�Լ������������
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
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetProperty����ʧ�ܡ�";


		//��������
		if (HostedNetworkInfo->IsPassPhase) {     //����Ϊ�ַ���
			ReturnValue = WlanHostedNetworkSetSecondaryKey(
				ClientHandle,
				strlen((char*)(HostedNetworkInfo->Key)) + 1,   //(����ĳ��ȹ涨��'\0'������)
				(PUCHAR)(HostedNetworkInfo->Key),
				TRUE,
				TRUE,
				&FailReason,
				NULL
			);
		}
		else {                                    //����Ϊ32�ֽڵĶ���������
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
		if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetSecondaryKey����ʧ�ܡ�";

		WlanCloseHandle(ClientHandle, NULL);
	}



	void StartHostedNetwork()
	{
		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle����ʧ�ܡ�";

		DWORD ReturnValue;
		BOOL Enabled = TRUE;
		WLAN_HOSTED_NETWORK_REASON FailReason;

		//����������类���ã���netsh wlan set hostednetwork mode=disallow�����ã������Ƚ������״̬��
		if (!_wifishareKernel_Enabled) {
			ReturnValue = WlanHostedNetworkSetProperty(
				ClientHandle,
				wlan_hosted_network_opcode_enable,
				sizeof(Enabled),
				(PVOID)&Enabled,
				&FailReason,
				NULL
			);
			if (ReturnValue != ERROR_SUCCESS) throw L"WlanHostedNetworkSetProperty���ó�������ʧ�ܡ�";
		}


		//���ó������硣ʹ��WlanHostedNetworkForceStart����ʹ�������˳������������Ի�������С�
		if (WlanHostedNetworkForceStart(ClientHandle, &FailReason, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkForceStart����ʧ�ܡ�";

		WlanCloseHandle(ClientHandle, NULL);
	}


	void StopHostedNetwork()
	{
		HANDLE ClientHandle;
		DWORD NegotiatedVersion;

		if (WlanOpenHandle(2, NULL, &NegotiatedVersion, &ClientHandle) != ERROR_SUCCESS)
			throw L"WlanOpenHandle����ʧ�ܡ�";

		WLAN_HOSTED_NETWORK_REASON FailReason;
		if (WlanHostedNetworkForceStop(ClientHandle, &FailReason, NULL) != ERROR_SUCCESS)
			throw L"WlanHostedNetworkForceStop����ʧ�ܡ�";

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
			if (!pNSM) throw L"NetSharingManager��ȡʧ�ܡ�";

			CComPtr<INetConnection> pNC = NULL;
			CComPtr<INetSharingEveryConnectionCollection> pNSECC = NULL;


			// ö���豸(���������ӣ������������ӡ�����)  
			hr = pNSM->get_EnumEveryConnection(&pNSECC);
			if (!pNSECC) {

				throw L"get_EnumEveryConnectionʧ�ܡ�";
			}


			CComPtr<IEnumVARIANT> pEV = NULL;
			CComPtr<IUnknown> pUnk = NULL;
			CComPtr<INetSharingConfiguration>  pNSC = NULL;
			hr = pNSECC->get__NewEnum(&pUnk);
			if (pUnk)
			{
				hr = pUnk->QueryInterface(__uuidof(IEnumVARIANT), (void**)&pEV);
				if (!pEV) {
					throw L"QueryInterfaceʧ�ܡ�";
				}
			}
			else {

				throw L"get__NewEnumʧ�ܡ�";
			}


			if (pEV)
			{
				VARIANT v;
				VariantInit(&v);// ��ʼ�� ���� ����VARIANT���Ǵ���ɲ�׽��  
				BOOL bFoundIt = FALSE;

				while (S_OK == pEV->Next(1, &v, NULL))// ö�������е�Ԫ�� �����ش���ֵ  
				{
					if (V_VT(&v) == VT_UNKNOWN)// ����λ������  
					{
						V_UNKNOWN(&v)->QueryInterface(__uuidof(INetConnection), (void**)&pNC);  // ��ѯ�豸�Ƿ�֧�ֽӿ�  
						if (pNC)
						{
							NETCON_PROPERTIES *pNP = NULL;
							_NetConnectionInfo NetConnectionInfo;
							_SharingType SharingType, SharingType_old;
							_ConnectingAction ConnectingAction = ConnectingAction_None;

							pNC->GetProperties(&pNP);// ��ȡ�豸����  
							NetConnectionInfo.DeviceName = pNP->pszwDeviceName;
							NetConnectionInfo.Name = pNP->pszwName;
							NetConnectionInfo.Status = pNP->Status;
							NetConnectionInfo.guid = pNP->guidId;


							hr = pNSM->get_INetSharingConfigurationForINetConnection(pNC, &pNSC);
							if (hr != S_OK) throw L"get_INetSharingConfigurationForINetConnection����ʧ�ܡ�";
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
									if (hr != S_OK) throw L"DisableSharing����ʧ�ܡ�";
								}
								break;
							case SharingType_Private:
								if (SharingType_old != SharingType_Private) {
									hr = pNSC->EnableSharing(ICSSHARINGTYPE_PRIVATE);
									if (hr != S_OK) throw L"EnableSharing(ICSSHARINGTYPE_PRIVATE)����ʧ�ܡ�";
								}
								break;
							case SharingType_Public:
								if (SharingType_old != SharingType_Public) {
									hr = pNSC->EnableSharing(ICSSHARINGTYPE_PUBLIC);
									if (hr != S_OK) throw L"EnableSharing(ICSSHARINGTYPE_PUBLIC)����ʧ�ܡ�";
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
								if (pNC->Connect() != S_OK) throw L"Connect����ʧ��";
								break;
							case ConnectingAction_Disconnect:
								if (pNC->Disconnect() != S_OK) throw L"Disconnect����ʧ��";
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
			throw L"WlanOpenHandle����ʧ�ܡ�";
		}

		dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanEnumInterfaces����ʧ�ܡ�";

		}


		DWORD DataSize;
		WLAN_PHY_RADIO_STATE *state;
		dwResult = WlanQueryInterface(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, wlan_intf_opcode_radio_state, NULL, &DataSize, (PVOID*)&state, NULL);
		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanQueryInterface����ʧ�ܡ�";
		}

		state->dwPhyIndex = 0;
		state->dot11SoftwareRadioState = PowerOn ? dot11_radio_state_on : dot11_radio_state_off;

		if (state->dot11HardwareRadioState == state->dot11SoftwareRadioState) return;
		dwResult = WlanSetInterface(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,
			wlan_intf_opcode_radio_state, sizeof(WLAN_PHY_RADIO_STATE), state, NULL);

		WlanFreeMemory(state);
		WlanFreeMemory(pIfList);

		if (dwResult != ERROR_SUCCESS) {
			throw L"WlanSetInterface����ʧ�ܡ�";
		}

	}


	//�����������֮ǰҪ�ȵ��� WSAStartup ��һ������ֻ�����һ��WSAStartup)
	//�ɹ�����true
	bool GetAdaptersInfo(std::function<void(PIP_ADAPTER_ADDRESSES pIpAdapterInfo)> callback)
	{
		PIP_ADAPTER_ADDRESSES pIpAdapterInfo = new IP_ADAPTER_ADDRESSES;    //�洢 IP_ADAPTER_ADDRESSES �б�
		ULONG size = sizeof(IP_ADAPTER_ADDRESSES);                          //��ǰ pIpAdapterInfo �Ĵ�С
		ULONG ret = GetAdaptersAddresses(
			AF_UNSPEC,  //ͬʱ����IPv4��IPv6��ַ
			0,          //flags
			NULL,       //Reserved
			pIpAdapterInfo,
			&size
		);

		if (ret == ERROR_BUFFER_OVERFLOW) {
			//����Ŀռ䲻��
			delete pIpAdapterInfo;
			//��ʱsize������������Ҫ�Ŀռ�
			pIpAdapterInfo = (PIP_ADAPTER_ADDRESSES)new BYTE[size];
			ret = GetAdaptersAddresses(
				AF_UNSPEC,  //ͬʱ����IPv4��IPv6��ַ
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




	int WaitForServiceStatusW(SC_HANDLE schService, DWORD Status, int TimeOut/*��λ����*/)
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

	DWORD ChangeServiceStatusW(const wchar_t *Name, bool Start, int TimeOut/*��λ����*/)
	{
		SERVICE_STATUS status;
		SC_HANDLE schSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);   // �򿪷�����ƹ�
																					  //�����ݿ⣬�����ط�����ƹ������ݿ�ľ��
		int ret;
		if (schSCManager == NULL)
		{
			return FALSE;
		}
		SC_HANDLE schService = OpenServiceW(schSCManager, Name, SERVICE_ALL_ACCESS);    // ��÷�����
		if (schService == NULL)
		{
			return FALSE;
		}
		//QueryServiceStatus(schService, &status);   // ��÷���ĵ�ǰ״̬
		//if (status.dwCurrentState = SERVICE_STOPPED)   // ���������ֹͣ״̬������״̬����Ϊ����״̬
		//	StartService(schService, 0, NULL);   //��������
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

		CloseServiceHandle(schSCManager);   // �رշ�����
		CloseServiceHandle(schService);

		if (!ret) return GetLastError();
		if (ret == 2) return ERROR_SERVICE_REQUEST_TIMEOUT;
		return ERROR_SUCCESS;

	}

}


