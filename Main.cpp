#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>
#include <iostream>

#include "etw_control.h"


// Attention here!!
#pragma comment(lib,"tdh.lib")


//Microsoft-Windows-Eventlog  {D8909C24-5BE9-4502-98CA-AB7BDC24899D}
static const GUID provider_guid_eventlog = { 0xAE4BD3BE, 0xF36F, 0x45B6,{ 0x8D, 0x21, 0xBD, 0xD6, 0xFB, 0x83, 0x28, 0x53 } };
//Microsoft-Windows-Audio AE4BD3BE-F36F-45B6-8D21-BDD6FB832853
static const GUID provider_guid_audio = { 0xD8909C24, 0x5BE9, 0x4502,{ 0x98, 0xCA, 0xAB, 0x7B, 0xDC, 0x24, 0x89, 0x9D } };
//Microsoft-Windows-Kernel-File EDD08927-9CC4-4E65-B970-C2560FB5C289
static const GUID provider_guid_file = { 0xEDD08927, 0x9CC4, 0x4E65,{ 0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89 } };
GUID provider_guid = provider_guid_file;

using namespace std;

void enumerate_providers();

int main(int argc, char *argv[]) {
	Etw_control etw;

	return 0;
}



// Enumerate all(include manifest-based & MOF registered on computer) ETW providers and output:)
void enumerate_providers() {
	ULONG provider_info_size = 10, *p2provider_info_size = &provider_info_size;
	PROVIDER_ENUMERATION_INFO *p2provider_info = (PROVIDER_ENUMERATION_INFO *)malloc(provider_info_size);
	TDHSTATUS enumerate_status = TdhEnumerateProviders(p2provider_info, p2provider_info_size);
	if (ERROR_INSUFFICIENT_BUFFER == enumerate_status) {
		// Malloc enough space for provider information.
		free(p2provider_info);
		p2provider_info = (PROVIDER_ENUMERATION_INFO *)malloc(provider_info_size);
	}
	else if (ERROR_INVALID_PARAMETER == enumerate_status) {
		printf("Enumerate Providers failed! Invalid parameter.\n");
		return;
	}
	enumerate_status = TdhEnumerateProviders(p2provider_info, p2provider_info_size);
	if (ERROR_SUCCESS != enumerate_status) {
		printf("Enumerate Providers failed! Unkown problem.\n");
		return;
	}
	for (ULONG i = p2provider_info->NumberOfProviders - 1; i >= 0; i--) {
		// output format is worng, but who cares:]
		//cout << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data1 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data2 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data3 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data4 << endl;
	}
	free(p2provider_info);
}