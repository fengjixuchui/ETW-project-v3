#pragma once

#include <Windows.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>

#include <stdio.h>
#include <iostream>

#include "etw_control.h"


using namespace std;

class Trace_parser {
public:
	static VOID WINAPI parser_event(PEVENT_TRACE_LOGFILE Buffer); // WINAPI callback function only recognize "C"
};