#pragma once

#include "stdafx.h"

struct Traffic {
	ULONGLONG in;
	ULONGLONG out;
	DWORD tick;
};

class CWorker
{
public:
	CWorker(void);
	~CWorker(void);

	void SetParams(HWND hWnd);
	static DWORD WINAPI ThreadFunc(LPVOID lpParameter);
	void Terminate();

	std::vector<Traffic> traffics;


private:
	HWND hWnd;


	bool myExitFlag; // �I���w����ێ�����t���O 
	HANDLE myMutex;  // �r������

	DWORD WINAPI ExecThread();
	void CollectTraffic();
};

