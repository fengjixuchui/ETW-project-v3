#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>
#include <iostream>

#include "trace_parser.h"


using namespace std;

VOID WINAPI Trace_parser::parser_event(PEVENT_TRACE Buffer) {
	PEVENT_TRACE temp = Buffer;
	wcout << "successful----------------" << endl;
	return;
}