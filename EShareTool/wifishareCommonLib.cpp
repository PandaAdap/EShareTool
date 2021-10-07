#include "stdafx.h"
#include "pch.h"
#include "wifishareCommonLib.h"
#include "wifishareKernel.h"

#include <functional>

namespace lqx {


	size_t BytesToHexStringW(const unsigned char *input, size_t inputlen, wchar_t *output, size_t outputlen) {
		wchar_t HEXW[16] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'A', 'B',
			'C', 'D', 'E', 'F'
		};
		size_t n = 0;
		if (outputlen < 3) return 0;
		for (size_t i = 0; i < inputlen && n < outputlen - 2; ++i) {
			int t = input[i];
			int a = t / 16;
			int b = t % 16;
			output[n++] = HEXW[a];
			output[n++] = HEXW[b];
		}
		output[n] = 0;
		return n;
	}

	size_t HexStringToBytesW(const wchar_t *input, unsigned char *output, size_t OutputBuffSize)
	{
		size_t n = 0;
		wchar_t tmp[3];
		wchar_t *endpos;
		tmp[2] = 0;
		size_t i, j;
		for (i = 0, j = 0; input[i] && j < OutputBuffSize; ++i) {
			wchar_t c = input[i];
			if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f') {
				tmp[n] = c;
				n++;
				if (n == 2) {
					output[j] = (unsigned char)wcstoul(tmp, &endpos, 16);
					j++;
					n = 0;
				}
			}
		}
		if (n != 0) throw L"字符 0~9，a~f，A~F 出现的次数不是偶数。";
		return j;
	}


	wchar_t *trimw(wchar_t *str, wchar_t *Charactors, _Trimtype Trimtype)
	{
		int start = 0, end;
		for (end = 0; str[end]; end++);
		int end_old = end -= 1;
		int i;
		wchar_t c;
		bool f;

		if (Trimtype&TRIM_LEFT) {
			for (; start <= end; start++) {
				f = true;
				for (int i = 0; c = Charactors[i]; i++) {
					if (str[start] == c) {
						f = false;
						break;
					}
				}
				if (f) break;
			}
		}
		if (Trimtype&TRIM_RIGHT) {
			for (; end >= start; end--) {
				f = true;
				for (int i = 0; c = Charactors[i]; i++) {
					if (str[end] == c) {
						f = false;
						break;
					}
				}
				if (f) break;
			}
		}
		if (start == 0 && end == end_old) return str;
		for (i = 0; i <= end - start; i++)
			str[i] = str[start + i];
		str[i] = '\0';

		return str;


	}


	BOOL IsRunasAdmin()
	{
		BOOL bElevated = FALSE;
		HANDLE hToken = NULL;

		// Get current process token  
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			return FALSE;

		TOKEN_ELEVATION tokenEle;
		DWORD dwRetLen = 0;

		// Retrieve token elevation information  
		if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
		{
			if (dwRetLen == sizeof(tokenEle))
			{
				bElevated = tokenEle.TokenIsElevated;
			}
		}

		CloseHandle(hToken);
		return bElevated;
	}







	size_t GetAllAdaptersInfo(ADAPTER_INFO *AdaptersInfo, size_t MaxCount)
	{
		MaxCount = MaxCount;
		size_t AdaptersInfoCount = 0;

		lqx::EnumConnections([&AdaptersInfo, &AdaptersInfoCount, &MaxCount](_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction) {
			if (AdaptersInfoCount < MaxCount) {
				AdaptersInfo[AdaptersInfoCount].Name.SetString(NetConnectionInfo->Name);
				AdaptersInfo[AdaptersInfoCount].DeviceName.SetString(NetConnectionInfo->DeviceName);
				AdaptersInfo[AdaptersInfoCount].Status = NetConnectionInfo->Status;
				AdaptersInfo[AdaptersInfoCount].guid = NetConnectionInfo->guid;
				AdaptersInfo[AdaptersInfoCount].SharingType = *SharingType;
			}
			AdaptersInfoCount++;
		});


		bool ret = lqx::GetAdaptersInfo([&AdaptersInfoCount, &AdaptersInfo](PIP_ADAPTER_ADDRESSES pIpAdapterInfo) {
			wchar_t str[256];
			DWORD strsize = 256;
			NET_LUID luid;

			bool foundIPv4 = false;
			bool foundIPv6 = false;
			INT ret;

			for (size_t i = 0; i < AdaptersInfoCount; i++) {
				if (ConvertInterfaceGuidToLuid(&AdaptersInfo[i].guid, &luid) != NO_ERROR) {
					throw L"ConvertInterfaceGuidToLuid调用失败。";
				}
				if(pIpAdapterInfo->Luid.Value==luid.Value){
					foundIPv4 = false;
					foundIPv6 = false;

					auto pUnicast = pIpAdapterInfo->FirstUnicastAddress;
					if (pUnicast != NULL) {
						int j;
						for (j = 0; pUnicast != NULL; j++) {
							if (pUnicast->Address.lpSockaddr->sa_family == AF_INET && !foundIPv4) {   //IPv4 
																									  //只管第一个IPv4地址
								foundIPv4 = true;
								ret = WSAAddressToStringW(
									pUnicast->Address.lpSockaddr,
									pUnicast->Address.iSockaddrLength,
									NULL,
									str,
									&strsize
								);
								if (ret) {
									throw L"WSAAddressToStringW(IPv4)调用失败。";
								}
								AdaptersInfo[i].IPv4.SetString(str);
							}

							if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6 && !foundIPv6) {
								//只管第一个IPv6地址
								foundIPv6 = true;
								ret = WSAAddressToStringW(
									pUnicast->Address.lpSockaddr,
									pUnicast->Address.iSockaddrLength,
									NULL,
									str,
									&strsize
								);
								if (ret) {
									throw L"WSAAddressToStringW(IPv6)调用失败。";
								}
								AdaptersInfo[i].IPv6.SetString(str);
							}
							pUnicast = pUnicast->Next;
						}
					}

					if (!foundIPv4) AdaptersInfo[i].IPv4.SetString(L"不存在");
					if (!foundIPv6) AdaptersInfo[i].IPv6.SetString(L"不存在");


				}

			}



		});

		if (!ret) {
			throw L"lqx::GetAdaptersInfo调用失败。";
		}

		return AdaptersInfoCount;

	}






	void StartSharing(const wchar_t *DeviceName)
	{
		bool found;

		StopSharing();

		found = false;
		EnumConnections([&DeviceName,&found](_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction) {
			if (!wcscmp(NetConnectionInfo->DeviceName, DeviceName)) {
				*SharingType = SharingType_Public;
			}
			else if (wcsstr(NetConnectionInfo->DeviceName, HOSTEDNETWORK_DEVICENAME)) {
				if (!found)
				{
					*SharingType = SharingType_Private;
					found = true;
				}
			}
		});

	}

	void StopSharing()
	{
		EnumConnections([](_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction) {
			*SharingType = lqx::SharingType_None;
		});
	}



	void ChangeAdapterSdate(const wchar_t *DeviceName, bool Connect)
	{
		EnumConnections([&DeviceName,&Connect](_NetConnectionInfo *NetConnectionInfo, _SharingType *SharingType, _ConnectingAction *ConnectingAction) {
			if (!wcscmp(NetConnectionInfo->DeviceName, DeviceName)) {
				*ConnectingAction = Connect ? ConnectingAction_Connect : ConnectingAction_Disconnect;
			}
		});
	}




}