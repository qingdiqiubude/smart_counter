
#include "stdio.h"
#include <process.h>
#include "database_fun.h"//mysqlΪ64λ�汾�������Ҫ��x64ģʽ�µ���
#include "amqp_fun.h"
#include "sys.h"
//�������ݿ����ȫ�ֱ���

//mq���ȫ�ֱ���
int main()
{
	//���ӱ��س�ʼ������Ҫִ�����������ݿ⣬��ʼ�������ṹ�����ݣ�Ȼ��򿪴��ڣ��������ڽ����̣߳������������ݷ����̣߳����Գ��ذ�ʵ������
	//�Ƿ�ͬ���ݿ����趨һ�£����Ƿ�Ϊ�ر�״̬���ڳ�ʼ���������ݵ�ʱ�򣬹��Ӳ�δ�������磬��ʱ��������κ����緢�͹���������
	Init_System();
	//unsigned char aa = 100;
	_beginthread(ReceiveChar, 0, &hCom_C);
	_beginthread(ReceiveCharT, 0, &hCom_Tem);
	_beginthread(Parse_Usart_Data_Run, 0, NULL);
	Board_Ready();
	Locker_Get_Stat();
	//��ʼ��������������
	init_amqp();
	_beginthread(run_listen, 0, NULL);
	Sleep(200);
	Init_Tem();
	_beginthread(Counter_Get_Tem_Ex, 0, NULL);
	//Get_Scheme("scheme1");
	//Board_Basic_Value_Set(2);
	//Sleep(2000);
	//Get_Boards_Items_Weight();
	//printf("ִ��У׼\r\n");
	//getchar(1);
	//Board_Curavture_Value_Set(board_info->board_items_weight_all, 2, 1);//У׼�ͱ���
	//printf("ִ��һ�����ѣ�\r\n");
	//getchar(1);
	//Procedure_Sales();
	//Locker_Open();
	//Sleep(5000);
	//Board_Get_Weight();
	//Send_CMD(board_info->id, BOARD_CMD_BASIC_VALUE, NULL, 0, 1);
	while (1)
	{
		Sleep(5000);
	}
	getchar();
}








