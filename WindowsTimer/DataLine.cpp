#include "DataLine.h"

#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib,"mswsock")
//构造函数
CDataLine::CDataLine()
{
}

//析构函数
/*
Function	:~CDataLIne
Memo		:析构函数，如果队列中还有数据，要从内容存中清除
Author			:Fred Huang
Add Data		:2008-3-4
Modify Data		:none
*/
CDataLine::~CDataLine(void)
{
	m_csLock.lock();
	std::list <ListItemData*> tempList;
	m_DataList.swap(tempList);
	m_csLock.unlock();
	ListItemData* pListItem = nullptr;
	while(tempList.size()>0)
	{
		pListItem=tempList.front();
		tempList.pop_front();
		delete pListItem->pData;
		delete pListItem;
	}
//	SafeDeleteArray(m_pDataBuffer);
}

//加入消息队列
/*
Function		:AddData
Memo			:将数据压入到队列当中
Author			:Fred Huang
Add Data		:2008-3-4
Modify Data		:none
Parameter		:
[IN]		pDataInfo	:要压入队列的数据指针
[IN]		uAddSize	:数据大小
[IN]		uDataKind	:数据类型
[IN]		pAppendData	:附加数据，可能是空的
[IN]		pAppendAddSize	:附加数据大小，可以为0，此时实体数据为空
Return			:指压入队列的大小	
*/
UINT CDataLine::AddData(DataLineHead * pDataInfo, UINT uAddSize, UINT uDataKind, void * pAppendData, UINT uAppendAddSize)
{
	
	m_csLock.lock();
	if(!m_hCompletionPort)
		return 0;

	ListItemData *pListItem=new ListItemData;//创建一个队列项

	pListItem->pData=NULL;										//先设为0，以保证后续不出错
	pListItem->stDataHead.uSize=uAddSize;						//数据大小
	pListItem->stDataHead.uDataKind=uDataKind;					//数据类型
	if(pAppendData)												//如果有附加数据
		pListItem->stDataHead.uSize+=uAppendAddSize;
	pListItem->pData=new BYTE[pListItem->stDataHead.uSize+1];	//申请数据项内存
	memset(pListItem->pData,0,pListItem->stDataHead.uSize+1);	//清空内存
	
	pDataInfo->uDataKind=uDataKind;
	pDataInfo->uSize=pListItem->stDataHead.uSize;

	memcpy(pListItem->pData,pDataInfo,uAddSize);				//复制实体数据
	if(pAppendData!=NULL)										//如果有附加数据，复制在实体数据后面
		memcpy(pListItem->pData+uAddSize,pAppendData,uAppendAddSize);
	m_DataList.push_back(pListItem);
	m_csLock.unlock();
	::PostQueuedCompletionStatus(m_hCompletionPort,pListItem->stDataHead.uSize,NULL,NULL);	//通知完成端口

	return pListItem->stDataHead.uSize;		//返回大小
}

//提取消息数据
/*
Function		:GetData
Memo			:从队列中取出数据
Author			:Fred Huang
Add Data		:2008-3-4
Modify Data		:none
Parameter		:
[OUT]		pDataBuffer	:取出数据的缓存
[IN]		uBufferSize	:缓存大小，缺省为 LD_MAX_PART = 3096
Return			:取出数据的实际大小	
*/
UINT CDataLine::GetData(DataLineHead * pDataBuffer, UINT uBufferSize)
{
	m_csLock.lock();
	
	memset(pDataBuffer,0,uBufferSize);
	//如果队列是空的，直接返回
	if(m_DataList.size()<=0)
		return 0;
	//取数据
	ListItemData *pListItem=m_DataList.front();
	m_DataList.pop_front();
	m_csLock.unlock();
	UINT uDataSize=pListItem->stDataHead.uSize;
	//投篮数据
	memcpy((void*)pDataBuffer,pListItem->pData,uDataSize);
	
	//删除队列中的数据
	delete []pListItem->pData;
	delete pListItem;

	return uDataSize;
}

//清理所有数据
/*
Function	:CleanLIneData
Memo		:清空队列
Author			:Fred Huang
Add Data		:2008-3-4
Modify Data		:none
Parameter		:
Return			:Wether clean up
*/
bool CDataLine::CleanLineData()
{
	m_csLock.lock();
	std::list <ListItemData*> tempList;
	m_DataList.swap(tempList);
	m_csLock.unlock();
	ListItemData* pListItem = nullptr;
	while (tempList.size() > 0)
	{
		pListItem = tempList.front();
		tempList.pop_front();
		delete pListItem->pData;
		delete pListItem;
	}
	return true;
}

//清理所有数据
/*
Function	:GetDataCount
Memo		:Get count of data in list buffer
Author			:Fred Huang
Add Data		:2008-3-5
Modify Data		:none
Parameter		:none
Return			:Count of data in list buffer
*/
int CDataLine::GetDataSize(void)
{
	m_csLock.lock();
	int size = m_DataList.size();
	m_csLock.unlock();
	return size;
}
