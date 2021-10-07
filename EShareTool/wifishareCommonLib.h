#pragma once

#include "framework.h"
#include "wifishareKernel.h"

namespace lqx {

	struct ADAPTER_INFO {
		CStringW Name;
		CStringW DeviceName;
		GUID guid;
		NETCON_STATUS Status;
		CString IPv4;
		CString IPv6;
		_SharingType SharingType;
	};

	enum _Trimtype {
		TRIM_LEFT = 0x1,
		TRIM_RIGHT = 0x2,
		TRIM_BOTH = 0x3
	};

	BOOL IsRunasAdmin();
	size_t HexStringToBytesW(const wchar_t *input, unsigned char *output, size_t OutputBuffSize);
	size_t BytesToHexStringW(const unsigned char *input, size_t inputlen, wchar_t *output, size_t outputlen);
	wchar_t *trimw(__inout wchar_t *str, wchar_t *Charactors = L" \t", _Trimtype Trimtype = TRIM_BOTH);
	size_t GetAllAdaptersInfo(ADAPTER_INFO *AdaptersInfo, size_t MaxCount);
	void StartSharing(const wchar_t *DeviceName);
	void StopSharing();
	void ChangeAdapterSdate(const wchar_t *DeviceName, bool Connect);
}