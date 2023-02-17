#pragma once

#include <windef.h>
#include "DataLine.h"
#include <afxmt.h>
class  CBaseMainManage;
//处理线程启动结构
#define GWL_USERDATA        (-21)
struct HandleThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	HANDLE								hCompletionPort;			//完成端口
	CBaseMainManage*					pMainManage;				//数据管理指针
	BOOL								bSuccess;					//启动成功标志
};

class  CBaseMainManage
{
protected:
	HWND									m_hWindow;					///窗口句柄
	bool									m_bInit;					///初始化标志
	bool									m_bRun;					///初始化标志
	INT										m_nThreadNum;			//线程数量
	HANDLE									m_hCompletePort;			///处理完成端口
	std::shared_ptr<std::thread>			m_hHandleThread;			///处理线程
	std::shared_ptr<std::thread>			m_workthread[10];			//工作线程
	std::shared_ptr<std::thread>			m_hWindowThread;			///窗口线程
	CDataLine								m_DataLine;					///数据队列
public:
	///构造函数
	CBaseMainManage(void);
	///析构函数
	virtual ~CBaseMainManage(void) { UnInit(); };
public:
	///设定定时器
	bool SetTimer(UINT uTimerID, UINT uElapse);
	///清除定时器
	bool KillTimer(UINT uTimerID);

	///服务接口函数 （主线程调用）
public:
	///初始化函数
	virtual bool Init();
	///取消初始化函数
	virtual bool UnInit();
	///启动函数
	virtual bool Start(int num);
	///停止服务
	virtual bool Stop();
	static void run(HandleThreadStartStruct* pThreadData, int id);
	static int WindowMsgThread(HandleThreadStartStruct* pThreadData,UINT id);
	bool WindowTimerMessage(UINT uTimerID);
	static LRESULT CALLBACK WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//定时器消息
	bool OnTimerMessage(UINT uTimerID);
};

extern CBaseMainManage* _MainMange;