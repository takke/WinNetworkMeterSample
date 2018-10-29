#include "stdafx.h"
#include "worker.h"

CWorker::CWorker(void)
{
	myMutex = CreateMutex(NULL, TRUE, NULL);
	myExitFlag = false;
}


CWorker::~CWorker(void)
{
	CloseHandle(myMutex);
}

void CWorker::SetParams(HWND hWnd)
{
	this->hWnd = hWnd;
}

DWORD WINAPI CWorker::ThreadFunc(LPVOID lpParameter)
{
	return ((CWorker*)lpParameter)->ExecThread();
}

DWORD WINAPI CWorker::ExecThread()
{

	while (true) {
		// 必要な処理

		CollectTraffic();

		InvalidateRect(hWnd, NULL, FALSE);

		Sleep(1000);



		// 終了チェック
		WaitForSingleObject(myMutex, 0);
		bool currentFlag = myExitFlag;
		ReleaseMutex(myMutex);
		if (currentFlag) {
			break;
		}
	}
	return S_OK;
}

void CWorker::CollectTraffic()
{
	DWORD i;
	PMIB_IFTABLE ifTable;
	MIB_IFROW MibIfRow;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	/* GetIfTable()で必要になるサイズを取得 */
	if (GetIfTable(NULL, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
		ifTable = (MIB_IFTABLE *)malloc(dwSize);
	}
	else {
		return;
	}

	lastTrafficIn = trafficIn;
	lastTrafficOut = trafficOut;
	lastTrafficTick = trafficTick;

	trafficIn = 0;
	trafficOut = 0;
	trafficTick = GetTickCount();

	if ((dwRetVal = GetIfTable(ifTable, &dwSize, 0)) == NO_ERROR) {
		if (ifTable->dwNumEntries > 0) {
//			printf("Number of Adapters: %ld\n\n", ifTable->dwNumEntries);
			for (i = 1; i <= ifTable->dwNumEntries; i++) {
				MibIfRow.dwIndex = i;
				if ((dwRetVal = GetIfEntry(&MibIfRow)) == NO_ERROR) {
//					printf("Description: %s\n", MibIfRow.bDescr);

					trafficIn += MibIfRow.dwInOctets;
					trafficOut += MibIfRow.dwOutOctets;
				}
			}
		}
	}
	else {
		printf("no adapters");
	}


}

void CWorker::Terminate()
{
	WaitForSingleObject(myMutex, 0);
	myExitFlag = true;
	ReleaseMutex(myMutex);
}
