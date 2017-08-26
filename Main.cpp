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

using namespace std;

void enumerate_providers();

int main(int argc, char *argv[]) {
	PEVENT_TRACE_PROPERTIES p2darpa_session_properties = allocate_session_properties(ETW_SESSION_NAME, ETW_LOGFILE_NAME);
	TRACEHANDLE darpa_session_handle = start_etw_session(p2darpa_session_properties);
	if (NULL == darpa_session_handle) {
		printf("start_etw_session() failed!\n");
		getchar();
	}
	else {
		printf("start_etw_session() successed! press any key to continue...\n");
		getchar();
		TDHSTATUS stop_session_status = stop_etw_session(darpa_session_handle, p2darpa_session_properties);
		if (ERROR_SUCCESS != stop_session_status) {
			printf("Stop trace session failed! %lu. press any key to continue...\n", stop_session_status);
			getchar();
			return 1;
		}
		free_session_properties(p2darpa_session_properties);
		printf("stop_etw_session() successed! press any key to continue...\n");
		getchar();
	}
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