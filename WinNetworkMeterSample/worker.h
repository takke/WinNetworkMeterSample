#pragma once

#include "stdafx.h"

class CWorker
{
public:
	CWorker(void);
	~CWorker(void);

	void SetParams(HWND hWnd);
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);
	void Terminate();


	DWORD trafficIn;
	DWORD trafficOut;
	DWORD trafficTick = 0;
	DWORD lastTrafficIn;
	DWORD lastTrafficOut;
	DWORD lastTrafficTick = 0;


private:
	HWND hWnd;


	bool myExitFlag; // ���I���w����ێ�����t���O 
	HANDLE myMutex;  // ���r���p Mutex 

	DWORD WINAPI ExecThread();
	void CollectTraffic();
};

