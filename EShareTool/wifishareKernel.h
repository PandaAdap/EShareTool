#pragma once


#include <wlanapi.h>
#include <NetCon.h> 
#include <winsock2.h>
#include <Iphlpapi.h>

#include <functional>

namespace lqx {

	enum _SharingType {
		//��SharingType_Public�����繲���SharingType_Private
		SharingType_None,
		SharingType_Private,
		SharingType_Public
	};

	enum _ConnectingAction {
		ConnectingAction_None,     //�޶���
		ConnectingAction_Connect,  //����
		ConnectingAction_Disconnect//�Ͽ�����
	};

	struct _HostedNetworkInfo {
		bool Active;                             //�������統ǰ�Ƿ���������
		char SSID[32];                           //�ַ�����󳤶ȣ�����ĩβ��\0����31�ֽ�   (���Բ��Եõ��Ľ���)
		bool IsPassPhase;                        //true:�������ַ�����8��63��ascii�ַ� + '\0'����false:�����ǳ���Ϊ32�ֽڵĶ��������ݡ�
		char Key[64];                            //���IsPassPhaseΪfalse����Key�����ַ��������ǳ���Ϊ32�ֽڵĶ��������ݡ�
		unsigned int NumberOfPeers;              //��ǰ������
		unsigned int MaxNumberOfPeers;           //���������
		DOT11_AUTH_ALGORITHM dot11AuthAlgo;      //��֤��ʽ
		DOT11_CIPHER_ALGORITHM dot11CipherAlgo;  //�����㷨
	};

	struct _NetConnectionInfo {
		wchar_t *Name;     //���������������硰�������ӡ���ע����룩
		wchar_t *DeviceName;//����������������"Realtek PCIe GBE Family Controller"��ע����룩
		GUID guid;
		NETCON_STATUS Status; //��ǰ������״̬
	};

	void QueryHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo);
	void SetHostedNetwork(_HostedNetworkInfo *HostedNetworkInfo);
	void StartHostedNetwork();
	void StopHostedNetwork();
	void EnumConnections(std::function<void(_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction)> callback);
	void SetWlanPowerState(bool PowerOn);

	//�����������֮ǰҪ�ȵ��� WSAStartup ��һ������ֻ�����һ��WSAStartup)
	//�ɹ�����true
	bool GetAdaptersInfo(std::function<void(PIP_ADAPTER_ADDRESSES pIpAdapterInfo)> callback);
	DWORD ChangeServiceStatusW(const wchar_t *Name, bool Start, int TimeOut/*��λ����*/);

}