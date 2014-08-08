// Copyright (c) Microsoft Open Technologies, Inc.  All rights reserved.  
// Licensed under the BSD 2-Clause License.  
// See License.txt in the project root for license information.

#ifndef WINDOWS_TIME_H
#define WINDOWS_TIME_H

#include <Windows.h>

// implement the Arduino Time functions for Windows
class WindowsTime {

	// The windows timer ticks every 16ms
	// For shorter precise delays the thread must spin
	const   DWORD SLEEP_THRESHOLD_MS = 17;
	const   DWORD SLEEP_MARGIN_MS = 16;
	const   DWORD SLEEP_MAX_MS = 0x3fffffff;

	LARGE_INTEGER qpFrequency;
	LARGE_INTEGER qpStartCount;

	DWORD qpcDiffToMS(LARGE_INTEGER& qpc, LARGE_INTEGER& qpcStop)
	{
		LARGE_INTEGER qpcDiff;
		qpcDiff.QuadPart = qpcStop.QuadPart - qpc.QuadPart;
		if (qpcDiff.QuadPart > 0)
		{
			qpcDiff.QuadPart = qpcDiff.QuadPart * 1000 / qpFrequency.QuadPart;
			if (qpcDiff.QuadPart > SLEEP_MAX_MS)
			{
				return SLEEP_MAX_MS;
			}
			else
			{
				return qpcDiff.LowPart;
			}
		}
		return 0;
	}

    void operator= (WindowsTime &wt_) {
        UNREFERENCED_PARAMETER(wt_);
    }

public:

	WindowsTime()
	{
		QueryPerformanceFrequency(&qpFrequency);
		QueryPerformanceCounter(&qpStartCount);
	}

	void delay(unsigned long ms)
	{
		LARGE_INTEGER us;
		us.QuadPart = ms;
		us.QuadPart *= 1000;
		delayMicroseconds(us);
	}

	void delayMicroseconds(LARGE_INTEGER& us)
	{
		LARGE_INTEGER qpc, qpcStop;

		QueryPerformanceCounter(&qpcStop);
		qpc = qpcStop;
		qpcStop.QuadPart += us.QuadPart * qpFrequency.QuadPart / 1000000;

		// use windows Sleep for lengthy wait
		DWORD delayMS = qpcDiffToMS(qpc, qpcStop);
		while (delayMS >= SLEEP_THRESHOLD_MS)
		{
			Sleep(delayMS - SLEEP_MARGIN_MS);

			QueryPerformanceCounter(&qpc);

			delayMS = qpcDiffToMS(qpc, qpcStop);
		}

		// spin remaining time, for µs accurate timing
		do {
			QueryPerformanceCounter(&qpc);
		} while (qpcStop.QuadPart > qpc.QuadPart);

	}

	unsigned long millis(void)
	{
		LARGE_INTEGER qpc;
		QueryPerformanceCounter(&qpc);
		qpc.QuadPart -= qpStartCount.QuadPart;
		return (unsigned long)(qpc.QuadPart * 1000 / qpFrequency.QuadPart);
	}

	unsigned long micros(void)
	{
		LARGE_INTEGER qpc;
		QueryPerformanceCounter(&qpc);
		qpc.QuadPart -= qpStartCount.QuadPart;
		return (unsigned long)(qpc.QuadPart * 1000000 / qpFrequency.QuadPart);
	}

};

__declspec(selectany) WindowsTime _WindowsTime;

#endif
