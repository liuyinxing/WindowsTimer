#pragma once
#include "StdAfx.h"
#include <windef.h>
#include <list> 
#include <mutex>
#include <winsock2.h>
#pragma comment(lib,"ws2_32") 
#include <MSWSock.h>
#pragma comment(lib,"mswsock")
//�����궨��
#define LD_DEF_STEP				4096								//���ݶ��л��岽��
#define LD_MAX_PART				8192								//���ݰ���󳤶�//3096 -> 1024*5 �ټ��� modify by wlr 20090716
#define LD_MAX_DATA				650000								//���ݶ�����󳤶�
#define HD_TIMER_MESSAGE				4							//��ʱ����Ϣ����
#define TIME_SPACE						50L									///��Ϸ ID ���
#define TIME_START_ID					100L								///��ʱ����ʼ ID
#define _AFXDLL
typedef unsigned int        UINT;
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
#ifndef STRUCT_DATALINEHEAD
#define STRUCT_DATALINEHEAD
//���ݶ�����Ϣͷ
struct DataLineHead
{
	UINT						uSize;								//���ݴ�С
	UINT						uDataKind;							//��������
};
#endif //STRUCT_DATALINEHEAD
#ifndef STRUCT_DATABASERESULTLINE
#define STRUCT_DATABASERESULTLINE
///���ݿ�����Ϣ�ṹ����
struct DataBaseResultLine
{
	DataLineHead						LineHead;					///����ͷ
	UINT								uHandleRusult;				///������
	UINT								uHandleKind;				///��������
	UINT								uIndex;						///��������
	unsigned long								dwHandleID;					///�����ʶ
};
#endif //STRUCT_DATABASERESULTLINE

struct WindowTimerLine
{
	DataLineHead						LineHead;					//����ͷ
	UINT								uTimerID;					//��ʱ�� ID
};
/*
Struct		:ListItemData
Memo		:���������ݽṹ
Author		:Fred Huang
Add Data	:2008-3-4
Modify Data	:none
Parameter	:
	stDataHead	:���ݰ�ͷ
	pData		:ÿ�����������ݵ�ָ�룬ʹ����new��ʽ������ڴ棬ע�⣬�ڳ�����ʱ��Ҫ��ʽ��delete ���ڴ�
*/
struct ListItemData
{
	DataLineHead				stDataHead;
	unsigned char						* pData;
};
//���ݶ�����
class CDataLine
{
	//��������
private:
	std::list <ListItemData*> m_DataList;

	void *						m_hCompletionPort;					//��ɶ˿�
	std::mutex					m_csLock;							//ͬ����

	//��������
public:
	//���캯��
	CDataLine();
	//��������
	virtual ~CDataLine(void);

	//���ܺ���
public:
	//��ȡ��
	std::mutex* GetLock() { return &m_csLock; };
	//������������
	bool CleanLineData();
	//������ɶ˿�
	void SetCompletionHandle(HANDLE hCompletionPort) { m_hCompletionPort=hCompletionPort; }
	//������Ϣ����
	virtual UINT AddData(DataLineHead * pDataInfo, UINT uAddSize, UINT uDataKind, void * pAppendData=NULL, UINT uAppendAddSize=0);
	//��ȡ��Ϣ����
	virtual UINT GetData(DataLineHead * pDataBuffer, UINT uBufferSize);
public:
	int GetDataSize(void);
};

