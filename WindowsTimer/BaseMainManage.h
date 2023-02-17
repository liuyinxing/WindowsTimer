#pragma once

#include <windef.h>
#include "DataLine.h"
#include <afxmt.h>
class  CBaseMainManage;
//�����߳������ṹ
#define GWL_USERDATA        (-21)
struct HandleThreadStartStruct
{
	//��������
	HANDLE								hEvent;						//�����¼�
	HANDLE								hCompletionPort;			//��ɶ˿�
	CBaseMainManage*					pMainManage;				//���ݹ���ָ��
	BOOL								bSuccess;					//�����ɹ���־
};

class  CBaseMainManage
{
protected:
	HWND									m_hWindow;					///���ھ��
	bool									m_bInit;					///��ʼ����־
	bool									m_bRun;					///��ʼ����־
	INT										m_nThreadNum;			//�߳�����
	HANDLE									m_hCompletePort;			///������ɶ˿�
	std::shared_ptr<std::thread>			m_hHandleThread;			///�����߳�
	std::shared_ptr<std::thread>			m_workthread[10];			//�����߳�
	std::shared_ptr<std::thread>			m_hWindowThread;			///�����߳�
	CDataLine								m_DataLine;					///���ݶ���
public:
	///���캯��
	CBaseMainManage(void);
	///��������
	virtual ~CBaseMainManage(void) { UnInit(); };
public:
	///�趨��ʱ��
	bool SetTimer(UINT uTimerID, UINT uElapse);
	///�����ʱ��
	bool KillTimer(UINT uTimerID);

	///����ӿں��� �����̵߳��ã�
public:
	///��ʼ������
	virtual bool Init();
	///ȡ����ʼ������
	virtual bool UnInit();
	///��������
	virtual bool Start(int num);
	///ֹͣ����
	virtual bool Stop();
	static void run(HandleThreadStartStruct* pThreadData, int id);
	static int WindowMsgThread(HandleThreadStartStruct* pThreadData,UINT id);
	bool WindowTimerMessage(UINT uTimerID);
	static LRESULT CALLBACK WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//��ʱ����Ϣ
	bool OnTimerMessage(UINT uTimerID);
};

extern CBaseMainManage* _MainMange;