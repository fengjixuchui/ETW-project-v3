#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>
#include <iostream>

#include "etw_control.h"
#include "trace_parser.h"


// Attention here!!
#pragma comment(lib,"tdh.lib")


using namespace std;

//void enumerate_providers();

int main(int argc, char *argv[]) {
	//Microsoft-Windows-Eventlog  {D8909C24-5BE9-4502-98CA-AB7BDC24899D}
	GUID provider_guid_eventlog = { 0xAE4BD3BE, 0xF36F, 0x45B6,{ 0x8D, 0x21, 0xBD, 0xD6, 0xFB, 0x83, 0x28, 0x53 } };
	//Microsoft-Windows-Audio AE4BD3BE-F36F-45B6-8D21-BDD6FB832853
	GUID provider_guid_audio = { 0xD8909C24, 0x5BE9, 0x4502,{ 0x98, 0xCA, 0xAB, 0x7B, 0xDC, 0x24, 0x89, 0x9D } };
	//Microsoft-Windows-Kernel-File EDD08927-9CC4-4E65-B970-C2560FB5C289
	GUID provider_guid_file = { 0xEDD08927, 0x9CC4, 0x4E65,{ 0xB9, 0x70, 0xC2, 0x56, 0x0F, 0xB5, 0xC2, 0x89 } };
	GUID provider_guid = provider_guid_file;

	// query etw session's information
	//ULONG session_properites_size = sizeof(EVENT_TRACE_PROPERTIES) + (ETW_SESSION_NAME_MAX_LENGTH + ETW_LOGFILE_NAME_MAX_LENGTH) * sizeof(WCHAR);;
	//PEVENT_TRACE_PROPERTIES p2session_properties = (PEVENT_TRACE_PROPERTIES)malloc(session_properites_size);
	//p2session_properties->Wnode.BufferSize = 1024;
	//TDHSTATUS query_status = QueryTrace(NULL, KERNEL_LOGGER_NAME, p2session_properties);


	Etw_control etw;
	Trace_parser parser;

	TDHSTATUS provider_status = etw.enable_etw_provider(&provider_guid_file);
	if (ERROR_SUCCESS != provider_status) {
		wprintf(L"enable_etw_provider() failed with %lu!\n", provider_status);
	}
	else 
		wprintf(L"enable_etw_provider() successfully!\n");

	//TDHSTATUS process_status = ProcessTrace(&etw.trace_handle, 1, 0, 0);
	//if (ERROR_SUCCESS == process_status) {
	//	wprintf(L"ProcessTrace() failed with %lu!\n", process_status);
	//}
	//else
	//	wprintf(L"ProcessTrace() successfully!\n");

	return 0;
}



//// Enumerate all(include manifest-based & MOF registered on computer) ETW providers and output:)
//void enumerate_providers() {
//	ULONG provider_info_size = 10, *p2provider_info_size = &provider_info_size;
//	PROVIDER_ENUMERATION_INFO *p2provider_info = (PROVIDER_ENUMERATION_INFO *)malloc(provider_info_size);
//	TDHSTATUS enumerate_status = TdhEnumerateProviders(p2provider_info, p2provider_info_size);
//	if (ERROR_INSUFFICIENT_BUFFER == enumerate_status) {
//		// Malloc enough space for provider information.
//		free(p2provider_info);
//		p2provider_info = (PROVIDER_ENUMERATION_INFO *)malloc(provider_info_size);
//	}
//	else if (ERROR_INVALID_PARAMETER == enumerate_status) {
//		printf("Enumerate Providers failed! Invalid parameter.\n");
//		return;
//	}
//	enumerate_status = TdhEnumerateProviders(p2provider_info, p2provider_info_size);
//	if (ERROR_SUCCESS != enumerate_status) {
//		printf("Enumerate Providers failed! Unknown problem.\n");
//		return;
//	}
//	for (ULONG i = p2provider_info->NumberOfProviders - 1; i >= 0; i--) {
//		// output format is wrong, but who cares:]
//		//cout << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data1 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data2 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data3 << '-' << p2provider_info->TraceProviderInfoArray[i].ProviderGuid.Data4 << endl;
//	}
//	free(p2provider_info);
//}