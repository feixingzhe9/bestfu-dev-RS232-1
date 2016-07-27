/***************************Copyright BestFu 2014-05-14*************************
��	����	Thread.c
˵	����	�߳�ʵ�ֺ���
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2014.06.18  
�ޡ��ģ�	2014.08.30 //�޸ĵ�����ʱע���̣߳���ʱ����벻��ִ������
*******************************************************************************/
#include "BestFuLib.h"
#include "Thread.h"
#include "SysTick.h"

#define THREAD_TOTAL    (thread.runCnt + thread.hangCnt)    //�߳�����

static ThreadManage_t   thread;

static u8 Thread_Check(u32 start, u32 end, void *fun);

/*******************************************************************************
  ������:  Thread_Init
����˵��:  �߳����ݳ�ʼ��
    ����:  ��
  ����ֵ:  ��
*******************************************************************************/
void Thread_Init(void)
{
    *(u32*)&thread.runCnt = 0;   //�߳������ÿ�
}

/*******************************************************************************
  ������:  Thread_Login
����˵��:  �߳�ע�� 
    ����:  fun:     �߳���
           ִ������
  ����ֵ:  ע����, 
*******************************************************************************/
u32 Thread_Login(ThreadCon_t con, u16 cnt, u32 frep, void * fun)
{
	u32 i;
	
    ERRR(THREAD_TOTAL >= THREAD_NUM, return 0);     //�߳̿ռ�
	if (Thread_Check(0, THREAD_TOTAL, fun) == 0)		//�߳�û�г������̱߳���
	{
		memmove(&thread.thread[thread.runCnt+1], &thread.thread[thread.runCnt],     //����һ���ռ䣬���½��߳�
                    sizeof(Thread_t)*(thread.hangCnt));   
		i = thread.runCnt;
		thread.runCnt++;
		thread.thread[i].flag = 0;
	}	
	else 
	{
		Thread_Wake(fun);	//�̻߳���
		i = Thread_Check(0, THREAD_TOTAL, fun) - 1;
	}	
   
    thread.thread[i].con = con;
    thread.thread[i].fun = fun;
    if (con == ONCEDELAY)   //������ʱ�߳�
    {
		if (0 == frep)
		{
			frep++;
		}
		cnt = 1;
		thread.thread[i].flag = 0;
    }
	thread.thread[i].cnt = cnt;
	thread.thread[i].frep = frep;
	thread.thread[i].countTime = 0; //yanhuan adding 2015/10/28
    
    return 1;
}

/*******************************************************************************
  ������:  Thread_Logout
����˵��:  �߳�ע�� 
    ����:  fun:     �߳���
  ����ֵ:  ��
*******************************************************************************/
void Thread_Logout(void * fun)
{
    u32 i = Thread_Check(0, THREAD_TOTAL, fun);

    if (i)    //�ҵ��߳�λ��
    {
        memmove(&thread.thread[i-1], &thread.thread[i], 
                    sizeof(Thread_t)*(THREAD_TOTAL - i));
        
        if (i > thread.runCnt)  //�߳�λ�����пռ�
        {
			thread.hangCnt--;
        }
        else
        {
            thread.runCnt--; 
        }
    }
}

/*******************************************************************************
  ������:  Thread_Hang
����˵��:  �̹߳��� 
    ����:  fun:     �߳���
  ����ֵ:  ��
*******************************************************************************/
void Thread_Hang(void * fun)
{
    u32 i = Thread_Check(0, thread.runCnt, fun);

    if (i)    //�߳�λ�������߳̿ռ�
    {
        Thread_t th;
        
        memcpy(&th, &thread.thread[i-1], sizeof(Thread_t)); 
        memmove(&thread.thread[i-1], &thread.thread[i], 
                    sizeof(Thread_t)*(thread.runCnt - i));
        memcpy(&thread.thread[--thread.runCnt], &th, sizeof(Thread_t));
        thread.hangCnt++;
    }
}

/*******************************************************************************
  ������:  Thread_Wake
����˵��:  �̻߳���
    ����:  fun:     �߳���
  ����ֵ:  ��
ע    ��:  ������ֻ�ܶ������̵߳Ĳ��� 
*******************************************************************************/
void Thread_Wake(void * fun)
{
    u32 i = Thread_Check(thread.runCnt, THREAD_TOTAL, fun);

    if (i)    //�ҵ��߳�λ��
    {
        if ((i - thread.runCnt) > 1)    //i���ǽ��������߳�
        {
            Thread_t th;
            memcpy(&th, &thread.thread[i-1], sizeof(Thread_t)); 
            memmove(&thread.thread[thread.runCnt + 1], &thread.thread[thread.runCnt], //�ڳ��洢�ռ�
                        sizeof(Thread_t)*(i - thread.runCnt));
            memcpy(&thread.thread[thread.runCnt], &th,  sizeof(Thread_t));
        }
        thread.runCnt++;
        thread.hangCnt--;
    }
}

/*******************************************************************************
  ������:  Thread_Process
����˵��:  �̴߳���
    ����:  ��
  ����ֵ:  ��
*******************************************************************************/
void Thread_Process(void)
{
    Thread_t *pThread = NULL;
	
    for (pThread = &thread.thread[0]; pThread < &thread.thread[thread.runCnt]; pThread++)
    {
        if (pThread->flag)
        {
			void *fun = pThread->fun;;
			
            pThread->flag = 0;
            switch (pThread->con)
            {
                case ONCEDELAY:	pThread->flag = 1;
                case MANY:      if (!pThread->cnt--)
								{	
                                    Thread_Logout(fun);
									pThread --;
									break;
                                }
                     
				case FOREVER:   ((fun1)(fun))();
                     break;				
                case RESULT:  	if (((fun2)(fun))() == 0)  //�߳����з���ʧ��
								{
									Thread_Logout(fun);
									pThread --;
								}
                    break;
                default : break;
            }
        }
    }
}
    
/*******************************************************************************
  ������:  Thread_RunCheck
����˵��:  �߳����������˶�
    ����:  ��
  ����ֵ:  ��
*******************************************************************************/
void Thread_RunCheck(void)
{
    Thread_t *pThread;

    for (pThread = &thread.thread[0]; pThread < &thread.thread[thread.runCnt]; pThread++)
    {
        if ((++pThread->countTime)>=(pThread->frep))//yanhuan modify 2015/10/28
        {
          pThread->countTime = 0;  
					pThread->flag = 1;
        }
    }
}

/*******************************************************************************
  ������:   Thread_Check
����˵��:   �̺߳˶�
    ����:   fun:     �߳���
            start:  ��ʼ�߳�λ��
            end:    ��ֹ�߳�λ��
  ����ֵ:   0(û��)/(1~THREAD_UNM)�߳�����λ��
*******************************************************************************/
u8 Thread_Check(u32 start, u32 end, void *fun)
{
    u32 i; 
    
    for (i = start; i < end;)
    {
        if (thread.thread[i++].fun == fun)
        {
            return i;
        }
    }
    
    return 0;
}

/*******************************************************************************
�� �� ��:  ThreadState_t Thread_Query(void *fun)
����˵��:  �̲߳�ѯ����
��    ��:  fun���̺߳���ָ��
�� �� ֵ:  NO_EXIST_STATE:��������������;RUNNING_STATE��������������;PENDING_STATE���ڹ���������
*******************************************************************************/
ThreadState_t Thread_Query(void *fun)    //Jay Add 2015.11.17
{
	u8 Taskpos = Thread_Check(0, THREAD_TOTAL, fun);
	if(0 == Taskpos)
	{
		return NO_EXIST_STATE;
	}
	else if(Taskpos < thread.runCnt + 2)
	{
		return RUNNING_STATE;
	}
	else
	{
		return PENDING_STATE;
	}
}

/**************************Copyright BestFu 2014-05-14*************************/

