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
		CollectTraffic();

		InvalidateRect(hWnd, NULL, FALSE);

		Sleep(20);



		// �I���`�F�b�N
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
	PMIB_IF_TABLE2 ifTable2;

	Traffic t;
	t.in = 0;
	t.out = 0;
	t.tick = GetTickCount();

	if (GetIfTable2(&ifTable2) == NO_ERROR) {
		if (ifTable2->NumEntries > 0) {
//			printf("----\n");
//			printf("Number of Adapters: %ld\n\n", ifTable2->NumEntries);
			for (int i = ifTable2->NumEntries - 1; i >= 0; i--) {

				MIB_IF_ROW2* ifrow2 = &ifTable2->Table[i];
//				wprintf(L"[%d] %s\n", i, ifrow2->Description);


				if (!ifrow2->InterfaceAndOperStatusFlags.FilterInterface &&
					(ifrow2->Type == IF_TYPE_ETHERNET_CSMACD ||
					 ifrow2->Type == IF_TYPE_IEEE80211 ||
					 ifrow2->Type == IF_TYPE_PPP)
					)
				{
					GetIfEntry2(ifrow2);
					t.in += ifrow2->InOctets;
					t.out += ifrow2->OutOctets;
				}
			}

			traffics.push_back(t);
//			printf("=> in[%lld], out[%lld]\n", t.in, t.out);
		}
	}
	else {
		printf("no adapters");
	}

	if (traffics.size() > 50) {
		traffics.erase(traffics.begin());
	}


}

void CWorker::Terminate()
{
	WaitForSingleObject(myMutex, 0);
	myExitFlag = true;
	ReleaseMutex(myMutex);
}
