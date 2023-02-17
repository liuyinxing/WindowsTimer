#include "BaseMainManage.h"

CBaseMainManage* _MainMange = nullptr;

bool CBaseMainManage::SetTimer(UINT uTimerID, UINT uElapse)
{
	if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
	{
		::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
		return true;
	}
	return false;
}

bool CBaseMainManage::KillTimer(UINT uTimerID)
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::KillTimer(m_hWindow, uTimerID);
		return true;
	}
	return false;
}

bool CBaseMainManage::Init()
{
	m_bInit = true;
	Start(1);
	return true;
}
bool CBaseMainManage::UnInit()
{

	m_bInit = false;
	return true;
}
bool CBaseMainManage::Start(int num)
{
	CEvent StartEvent(FALSE, TRUE, NULL, NULL);
	m_bRun = true;
	m_nThreadNum = num;
	if (num > 10) m_nThreadNum = 10;
	m_hCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hCompletePort == NULL) return -1;
	m_DataLine.SetCompletionHandle(m_hCompletePort);

	HandleThreadStartStruct	ThreadStartData;
	ThreadStartData.pMainManage = this;
	ThreadStartData.hCompletionPort = m_hCompletePort;
	ThreadStartData.hEvent = StartEvent;
	for (int i = 0; i < m_nThreadNum; i++)
		m_workthread[i].reset(new std::thread(CBaseMainManage::run,&ThreadStartData, i));
	for (int i = 0; i < m_nThreadNum; i++)
		m_workthread[i]->detach();
	UINT uThreadID = 11;
	m_hWindowThread.reset(new std::thread(CBaseMainManage::WindowMsgThread,&ThreadStartData, uThreadID));
	m_hWindowThread->detach();
	while (m_bRun)
	{


	}
	return true;
}
bool CBaseMainManage::Stop()
{
	return false;
}
void CBaseMainManage::run(HandleThreadStartStruct* pThreadData, int id)
{
	printf("run workthread...%d\n", id);

	HandleThreadStartStruct* pData = pThreadData;		//�߳���������ָ��
	CBaseMainManage* pMainManage = pData->pMainManage;					//���ݹ���ָ��
	CDataLine* m_pDataLine = &pMainManage->m_DataLine;				//���ݶ���ָ��
	HANDLE						hCompletionPort = pData->hCompletionPort;				//��ɶ˿�
	bool& bRun = pMainManage->m_bRun;							//���б�־
	ULONG_PTR    key = 1;//��ɶ˿ڰ󶨵��ֶ�
	OVERLAPPED* overlapped = nullptr;//����Socket��ʱ�������Ǹ��ص��ṹ  
	DWORD        recvBytes = 0;//������ɷ����ֽ���
	BOOL						bSuccess = FALSE;
	BYTE						szBuffer[LD_MAX_PART];
	DataLineHead* pDataLineHead = (DataLineHead*)szBuffer;
	while (pData->pMainManage->m_bRun)
	{
		bSuccess = GetQueuedCompletionStatus(pData->hCompletionPort, &recvBytes, &key, &overlapped, INFINITE);
		if(bSuccess==FALSE || recvBytes == 0) ::_endthreadex(0);
		while (m_pDataLine->GetDataSize())
		{
				if ((bRun==false)||(m_pDataLine->GetData(pDataLineHead,sizeof(szBuffer))==0)) continue;
				switch (pDataLineHead->uDataKind)
				{
				case HD_TIMER_MESSAGE:		//��ʱ����Ϣ
				{
					WindowTimerLine* pTimerMessage = (WindowTimerLine*)pDataLineHead;
					pMainManage->OnTimerMessage(pTimerMessage->uTimerID);
					break;
				}
				default:
					break;
				}
		}
	}
}
int CBaseMainManage::WindowMsgThread(HandleThreadStartStruct* pThreadData,UINT id)
{
	HandleThreadStartStruct* pStartData = (HandleThreadStartStruct*)pThreadData;
	try
	{
		//ע�ᴰ����
		LOGBRUSH		LogBrush;
		WNDCLASS		WndClass;
		TCHAR			szClassName[] = TEXT("CMainManageWindow");
		LogBrush.lbColor = RGB(0, 0, 0);
		LogBrush.lbStyle = BS_SOLID;
		LogBrush.lbHatch = 0;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hCursor = NULL;
		WndClass.hIcon = NULL;
		WndClass.lpszMenuName = NULL;
		WndClass.lpfnWndProc = WindowProcFunc;
		WndClass.lpszClassName = szClassName;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;
		WndClass.hInstance = NULL;
		WndClass.hbrBackground = (HBRUSH)::CreateBrushIndirect(&LogBrush);
		::RegisterClass(&WndClass);

		//��������
		pStartData->pMainManage->m_hWindow = ::CreateWindow(szClassName, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, pStartData->pMainManage);
		if (pStartData->pMainManage->m_hWindow == NULL) throw TEXT("���ڽ���ʧ��");
		pStartData->pMainManage->SetTimer(1000, 100);
	}
	catch (...)
	{
		TRACE("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		//��������
		pStartData->bSuccess = FALSE;
		_endthreadex(0);
	}
	pStartData->bSuccess = TRUE;
	MSG	Message;
	while (::GetMessage(&Message, NULL, 0, 0))
	{
		if (!::TranslateAccelerator(Message.hwnd, NULL, &Message))
		{
			::TranslateMessage(&Message);
			::DispatchMessage(&Message);
		}
	}
	_endthreadex(0);
	return 0;
}
//��ʱ��֪ͨ��Ϣ
bool CBaseMainManage::WindowTimerMessage(UINT uTimerID)
{
	WindowTimerLine WindowTimer;
	WindowTimer.uTimerID = uTimerID;
	return (m_DataLine.AddData(&WindowTimer.LineHead, sizeof(WindowTimer), HD_TIMER_MESSAGE) != 0);
}

//���ڻص�����
LRESULT CALLBACK CBaseMainManage::WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:		//���ڽ�����Ϣ
	{
		DWORD iIndex = TlsAlloc();
		CBaseMainManage* pMainManage = (CBaseMainManage*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		TlsSetValue(iIndex, pMainManage);
		::SetWindowLong(hWnd, GWL_USERDATA, iIndex);
		break;
	}
	case WM_TIMER:		//��ʱ����Ϣ
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseMainManage* pMainManage = (CBaseMainManage*)::TlsGetValue(iIndex);
		if ((pMainManage != NULL) && (pMainManage->WindowTimerMessage((UINT)wParam) == false)) ::KillTimer(hWnd, (UINT)wParam);
		break;
	}
	case WM_CLOSE:		//���ڹر���Ϣ
	{
		DestroyWindow(hWnd);
		break;
	}
	case WM_DESTROY:	//���ڹر���Ϣ
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseMainManage* pMainManage = (CBaseMainManage*)::TlsGetValue(iIndex);
		if (pMainManage != NULL) pMainManage->m_hWindow = NULL;
		::TlsFree(iIndex);
		PostQuitMessage(0);
		break;
	}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


bool CBaseMainManage::OnTimerMessage(UINT uTimerID)
{
	//��Ϸ��ʱ��
	if (uTimerID >= TIME_START_ID)
	{
		BYTE bDeskIndex = (BYTE)((uTimerID - TIME_START_ID) / TIME_SPACE);
	/*	if (bDeskIndex < m_InitData.uDeskCount)
		{
			return (*(m_pDesk + bDeskIndex))->OnTimer((uTimerID - TIME_START_ID) % TIME_SPACE);
		}*/
		printf("��ʱ����Ϣ����");
		return true;
	}
}
//���캯��
CBaseMainManage::CBaseMainManage(void)
{
	m_bInit = false;
	m_hWindow = NULL;
	m_hHandleThread = NULL;
	m_hCompletePort = NULL;
}