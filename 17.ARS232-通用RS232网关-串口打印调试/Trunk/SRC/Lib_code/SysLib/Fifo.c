/***************************Copyright BestFu 2014-05-14*************************
��	����	Fifo.c
˵	����	ͨ�ö��д���ԭ����
��	�룺	Keil uVision4 V4.54.0.0
��	����	v1.0
��	д��	Unarty
��	�ڣ�	2013-09-03
��  ��:     ����
*******************************************************************************/
#include "fifo.h"
#include "BestFuLib.h"

static u32 fifo_surplusSize(FIFO_t *head); //����ʣ��ռ��ж�
static u8 fifo_full(FIFO_t *head);         //�����ж�

/*******************************************************************************
�� �� ����  fifo_Init
����˵����  ���г�ʼ��
��	  ����  *head:  ����ͷ
            *data:  ���ݴ洢�׵�ַ
            len:    ���ݴ洢���򳤶�
�� �� ֵ��  ��ʼ����� TRUE/FALSE
*******************************************************************************/
u8 fifo_Init(FIFO_t *head, u8 *buf, u32 len)
{
    ERRR(head == NULL, return FALSE);
    head->data = buf;
    head->size = len;
    head->front = head->rear = 0;

    return TRUE;
}

/*******************************************************************************
�� �� ����  fifo_Clr
����˵����  �������
��	  ����  *head:  ����ͷ
�� �� ֵ��  ��
*******************************************************************************/
void fifo_Rst(FIFO_t *head)
{
    ERRR(head == NULL, return);
    head->front = 0;
	head->rear = 0;
}
/*******************************************************************************
�� �� ����  fifo_empty
����˵����  �ж϶����Ƿ�Ϊ��
��	  ����  *head:  ����ͷ
�� �� ֵ��  TRUE(����Ϊ��)/FALSE
*******************************************************************************/
u8 fifo_empty(FIFO_t *head)
{
    
    return ((head->front == head->rear) ? TRUE : FALSE);
}

/*******************************************************************************
�� �� ����  fifo_full
����˵����  �ж϶����Ƿ�����
��	  ����  *head:  ����ͷ
�� �� ֵ��  TRUE(��������)/FALSE
*******************************************************************************/
static u8 fifo_full(FIFO_t *head)
{
    
    return ((head->front == ((head->rear+1)%head->size)) ? TRUE : FALSE);
}

/*******************************************************************************
�� �� ����  fifo_surplusSize
����˵����  �ж϶���ʣ��ռ��С 
��	  ����  *head:  ����ͷ
�� �� ֵ��  ʣ��ռ��С(�ַ�Ϊ��λ)
*******************************************************************************/
static u32 fifo_surplusSize(FIFO_t *head)
{

    return ((head->front > head->rear)
            ? (head->front - head->rear - 1)
            : (head->size + head->front - head->rear - 1));
}

/*******************************************************************************
�� �� ����  fifo_validSize
����˵����  ��ѯ������Ч�ռ��С
��	  ����  *head:  ����ͷ
�� �� ֵ��  ʣ��ռ��С(�ַ�Ϊ��λ)
*******************************************************************************/
u32 fifo_validSize(FIFO_t *head)
{

	return ((head->rear < head->front)
			? (head->rear + head->size - head->front)
			: (head->rear - head->front));
}

/*******************************************************************************
�� �� ����  kfifo_puts
����˵����  ���
��	  ����  *head:  ����ͷ
            *data:  �������
            len:    ���ݳ���
�� �� ֵ��  ��ӽ�� TRUE/FALSE
*******************************************************************************/
u8 fifo_puts(FIFO_t *head, u8 *data, u32 len)
{  
	u32 size;

    ERRR(head == NULL, return FALSE);
    ERRR(len > fifo_surplusSize(head), return FALSE); //�ж϶������Ƿ��д洢�ռ�
    
	size = MIN(len, head->size - head->rear);
	memcpy(head->data + head->rear, data, size);
	memcpy(head->data, data + size, len - size);

	head->rear = (head->rear + len)%head->size;

    return TRUE;   
}

/*******************************************************************************
�� �� ����  kfifo_gets
����˵����  ����
��	  ����  *head:  ����ͷ
            *data:  ��������
            len:    �������ݳ���
�� �� ֵ��  ���ӽ�� TRUE/FALSE
*******************************************************************************/
u8 fifo_gets(FIFO_t *head, u8 *data, u32 len)
{
	u32 size;

    ERRR(head == NULL, return FALSE);
    ERRR(fifo_empty(head) == TRUE, return FALSE); //����Ϊ��
    ERRR(len > fifo_validSize(head), return FALSE); //�洢����С��Ҫ��ȡ����

	size = MIN(len, head->size - head->front);
	memcpy(data, head->data + head->front, size);
	memcpy(data+size, head->data, len - size);

	head->front = (head->front + len)%head->size;

    return TRUE;   
}

/*******************************************************************************
�� �� ����  fifo_putc
����˵����  ���һ���ַ�
��	  ����  *head:  ����ͷ
            data:   Ҫ��ӵ�����
�� �� ֵ��  ��ӽ�� TRUE/FALSE
*******************************************************************************/
u8 fifo_putc(FIFO_t *head, u8 data)
{
    ERRR(head == NULL, return FALSE);
    ERRR(fifo_full(head) == TRUE, return FALSE); //�ж϶������Ƿ��д洢�ռ�

    head->data[head->rear] = data;

    head->rear = (++head->rear)%head->size;

    return TRUE;   
}

/*******************************************************************************
�� �� ����  kfifo_getc
����˵����  ����һ���ַ�
��	  ����  *head:  ����ͷ
            data:  ��������
�� �� ֵ��  ���ӽ�� TRUE/FALSE
*******************************************************************************/
u8 fifo_getc(FIFO_t *head, u8 *data)
{
    ERRR(head == NULL, return FALSE);
    ERRR(fifo_empty(head) == TRUE, return FALSE); //����Ϊ��

    *data = head->data[head->front];
    head->front = (++head->front)%head->size;

    return TRUE;   
}

/*******************************************************************************
�� �� ����  fifo_Find
����˵����  �������ݲ���
��	  ����  *head:  ����ͷ
            *data:  ����ƥ������
            len:    ƥ�����ݳ���
�� �� ֵ��  ����ƥ��ָ�룬���ҵ���λ�� 
*******************************************************************************/
u32 fifo_find(FIFO_t *head, const u8 *data, u32 len)
{
	u32 i, n;
	
	ERRR((NULL == head)||(NULL == data)||(0 == len), return 0);
	
	for (i = head->front, n = 0; i != head->rear; i = (++i)%head->size)
	{
		if (head->data[i] == data[n])	//�����������
		{
			if (++n == len)	//ƥ�䳤�����
			{
				n--;
				break;
			}
		}
		else
		{
			n = 0;
		}
	}
	
	return ((i+head->size - n)%head->size);
}

/*******************************************************************************
�� �� ����  fifo_cmp
����˵����  �������ݱȽ�
��	  ����  fifo:	�Ƚ�Դ
            seat:   λ��fifo�Ŀ�ʼ�Ƚ�λ��
			cmp��	���Ƚ���
			cmpsize:�Ƚϳ���
�� �� ֵ��  TRUE/FALSE
*******************************************************************************/
u8 fifo_cmp(const FIFO_t *fifo, u32 seat, const u8 *cmp, u32 cmpsize)
{
	u32 i;

	ERRR((fifo->data == NULL)||(cmp == NULL), return FALSE);

	for (i = 0; i < cmpsize; i++)	//�������в���
	{
		if (fifo->data[(seat+i)%fifo->size] != *cmp++)
		{
            return FALSE;
		}
	}
    
    return TRUE;
}	


/**************************Copyright BestFu 2014-05-14*************************/

