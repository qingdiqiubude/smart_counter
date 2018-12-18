
#include "amqp_fun.h"
#pragma comment(lib, "rabbitmq.4.lib")

amqp_socket_t *amqp_socket = NULL;
amqp_connection_state_t conn;

amqp_bytes_t queuename;//queue ������
                       //exchange ������ͬ���ӵ�SN���
                       //Ҫ���ӵķ�������Ϣ��counter�ṹ����

int init_amqp()
{
	//��Ҫ��ʼ���������������
	queuename.bytes = counter->queue_name;//��ʼ��queue�����֣����������������ݿ��趨��Ŀǰ��ֵͬ���ӵ�SN����ͬ
									//�ù�����Ϣ��SN�������queue,�˴����ܻ��������ף�Ӧ�����ɷ�����������queue���������ִ洢�����ݿ���
								  //�˴�ֱ�ӽ��а󶨣���������������ע��˴�������queue�����ڷ���������ӷ��������õģ���queue����ͬ
								  //���������õ�queue����
	queuename.len = strlen(counter->queue_name);
	return AMQP_FUN_SUCCESS;
}

//�������ݺ���
int run_listen(void * dummy)
{
	dummy = NULL;
	//amqp_socket_t *socket = NULL;
	//amqp_connection_state_t conn;
	int status;
	conn = amqp_new_connection();//�˴��ǽ���һ��amqp���ӵĽṹ�岢��ʼ���ýṹ���ýṹ����Ҫʹ��amqp_destroy_connection()������Դ�ͷ�
								 //��������һ��amqp_connection_state_t ָ�������ɹ���������null������0�����ʧ��

	amqp_socket = amqp_tcp_socket_new(conn);//�˺����ǽ���tcp�շ��Ĺؼ�����������amqp_new_connection()�������صĽṹ��ָ��
											//�˺������ڲ�������һ��amqp_tcp_socket_t�ṹ�壬�ýṹ����������״̬��sock�������Լ��շ��ȹ����ͺ�����
											//�����й��շ�����Ҫ���ܺ�����ͨ���ڲ�һ��klassָ�룬ָ��amqp_socket_class_t�ṹ�����amqp_tcp_socket_class�ĵ�ַ��ʵ�ֵ�
											//amqp_tcp_socket_class�ýṹ�������Ԫ�ؾ�Ϊ����ָ�룬����6���������ֱ�Ϊ���ա����͡��򿪡��رա���ȡ�����ɾ��
											//���amqp_tcp_socket_new����ͨ��amqp_set_socket������amqp_tcp_socket_t�ṹ���������amqp_new_connection()���صĽṹ��
											//�е�socket���������ҽ�amqp_tcp_socket_t�ṹ���������
											//����������ķ���ֵ��ʵ�Ѿ���conn�����ڵ�conn->socket���ָ����ָ����
	if (!amqp_socket)//
	{
		die("creating TCP socket");
	}

	status = amqp_socket_open(amqp_socket, counter->server_ip , atoi(counter->server_port));//��tcp���Ӻ��������ڲ�ʵ���ϵ�����klassָ��ָ��Ľṹ���ص�open������ʵ�ֵ�
	if (status) 
	{
		die("opening TCP socket");
	}

	//��¼��amqp_login����Ϊ  
	//1�����Ӷ��󣬼�amqp_new_connection�����ķ���ֵ���˴�Ϊconn
	//2�����ӵ��������һ�㶼Ϊ"/"��broker������ĸ����һ������ر����������Ҫ��һ���˽⣬��broker�п����趨��ͬ�ĵ�¼�û�����Ӧ����������ƣ�guest�պ�Ĭ����"/"
	//3������channel��������ֵ�����Ӷ����ʼ����ʱ���趨Ϊ0����Ϊ�����ƣ������65535��channel-0Ϊ��������������Ҳ���趨��ֵ����ʵ��ʹ����ʹ�ý�С��һ����ֵ��Ϊ�޶�
	//4��֡��󳤶ȣ�4096λ��С��2��31�η�-1Ϊ���131072Ϊ128K�������ֵ��ΪĬ��ֵ���
	//5���������ܼ�����趨һ����������Ϊheartbeat�ļ��ʱ�䣬����趨Ϊ0��Ϊ��ʹ��
	//6����¼����������֪���ƣ�SASLΪc/s��һ����֤���ƣ�����amqp�����еģ��ڴ˴��ṩ������֤����
	//		*AMQP_SASL_METHOD_PLAIN ���ô��ַ�ʽ���ڸò�������Ҫ�ṩ�û�����������������������������ʹ�õķ�ʽ
	//		*AMQP_SASL_METHOD_EXTERNAL ���ô��ַ�ʽ���ڸĲ�����Ҫ�ṩһ����֤�ַ�����������ʹ�û���Ҫ���˽�,��֪���Ƿ�Ϊ����Ҫ������û��ף��½��õ�ʱ���Ƿ���Ҫ�����ǿ�ѡ�
	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 360 , AMQP_SASL_METHOD_PLAIN, counter->mq_name, counter->mq_pw), "Logging in");

	amqp_channel_open(conn, atoi(counter->channel));
	/*
	*	amqp_get_rpc_reply�����ܹ���Ӧ�ڴ����AMQPͬ���ķ������䷵��һ��ָ����÷��������ָ�룬�����쳣��ʱ�򣬸ú�������null����ʱ��Ҫͨ��һЩ�����˽⵽��ʲôԭ����ɵ��쳣
	*	�ú�������һ��API���������·���ֵ��ʵ��
	*	�ú���ֻʹ���ڱ����ú���������ִ�н�����ص�ʱ�򣨲�֪���˴������Ƿ���ȷ��
	*	�������ʵ�����Ƿ������Ӷ���conn�е�most_recent_api_result��ֵ����ֵΪһ���ṹ��amqp_rpc_reply_t���ýṹ������һ������ֵ������������ֵ
	*		*reply_typeΪ����ֵ����Ϊ1��AMQP_RESPONSE_NORMAL�������ո�ִ�е�api��������ΪAMQP_RESPONSE_SERVER_EXCEPTION����AMQP_RESPONSE_LIBRARY_EXCEPTION�����ո�ִ�е�api������
	*		*������������Ҫ��������ֵreply��һ���ṹ�壩��library_error��һ��int�ͣ�������ԭ�򣬴˴��ɼ�die_on_amqp_error�����������˷���֮�õ�
	*/
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    //����queue�����²������onenote�ʼ�
	/*  @param [in] state connection state
	* @param [in] channel the channel to do the RPC on //���͸��ĸ�channel
	* @param [in] queue queue //���������Ϊ����Ϊ��ϵͳ�Զ�����
	* @param [in] passive passive //��֪����ʲô����
	* @param [in] durable durable //�־û�����
	* @param [in] exclusive exclusive //��������ֻ����һ������
	* @param [in] auto_delete auto_delete //�����ж��ĸ�queue�����ӶϿ���queueɾ��
	* @param [in] arguments arguments
	* �˺�����ִ�����ݽ�Ϊ���������������뺯���ڲ���amqp_queue_declare_t�ṹ����������ҵ���amqp_simple_rpc_decoded����������������queue�������ȴ��ظ�
	* Ŀǰ��Ҫ�˽�������һ��������������queue��ѡ��һЩ����������
	*/
	amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, atoi(counter->channel), queuename , 0, 0, 0, 1, amqp_empty_table);//�˺�����һЩ��������Ϊ����Ҫ���������
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
	//��queueͬexchange��
	/**
	* amqp_queue_bind
	*
	* @param [in] state connection state
	* @param [in] channel the channel to do the RPC on
	* @param [in] queue queue
	* @param [in] exchange exchange
	* @param [in] routing_key routing_key
	* @param [in] arguments arguments
	* @returns amqp_queue_bind_ok_t
	*/
	amqp_queue_bind(conn, atoi(counter->channel), queuename, amqp_cstring_bytes(counter->exchange_name),amqp_cstring_bytes(counter->routingkey), amqp_empty_table);//�˴���routingkeyҲ��ͬSN�����ͬ
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");
	/*
	* @param [in] state connection state
	* @param [in] channel the channel to do the RPC on
	* @param [in] queue queue
	* @param [in] consumer_tag consumer_tag
	* @param [in] no_local no_local
	* @param [in] no_ack no_ack
	* @param [in] exclusive exclusive
	* @param [in] arguments arguments
	*/
	amqp_basic_consume(conn, atoi(counter->channel), queuename, amqp_empty_bytes, 0, 1, 0,amqp_empty_table);
	die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

	{
		printf("\r\n");
		printf("amqp listen start!! \r\n");
		printf("server ip is : %s \r\n" , counter->server_ip);
		printf("server port is : %s \r\n" , counter->server_port);
		printf("exchange name is : %s \r\n" , counter->exchange_name);
		printf("queue name is : %s \r\n" , queuename.bytes);
		printf("routingkey is : %s \r\n", counter->routingkey);
		printf("\r\n");
		printf("\r\n");
		for (;;) 
		{
			amqp_rpc_reply_t res;
			amqp_envelope_t envelope;

			amqp_maybe_release_buffers(conn);

			//�@�������ĵ����������鳬�r�r�g�������null�t������ʽ������һ����������ʹ�ã�ֱ�Ӟ�0����
			printf("\r\n");
			printf("\r\n");
			printf("Wait for messaage ...\r\n");
			printf("\r\n");
			printf("\r\n");
			res = amqp_consume_message(conn, &envelope, NULL, 0);//�˴��趨��������ʽ����

			if (AMQP_RESPONSE_NORMAL != res.reply_type) //���¼��д������ȷ�Ի���Ҫ���ԣ�������if������������쳣���ڳ����ȴ���Ϣ��֪���Ƿ���ȷ
				                                        //Ҳ���ظ�ִ�иú���Ҳ����ȷ�Ĵ���ʽ
			{
				//���յ����쳣��Ϣ����
				//amqp_destroy_envelope(&envelope);//��֪���ڴ˴�ִ�д˾��Ƿ�������⣬��Ҫ����###
				//continue;
				break;
			}


			//�������յ������ݣ�ע��Ŀǰ���Ե�����������߱���Ҫִ�������д������ݲŻ��ٴν��ܷ�����������������ݣ���˲�����ֵ����߳�
			//���ڴ���һ�������ʱ���ֽ��յ��������һ���������������counter�ṹ���е�isbusyֵ�ڴ˴���δʹ��
			json_parse_fun(envelope.message.body.bytes);
			amqp_destroy_envelope(&envelope);
		}
	}

	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
		"Closing channel");
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
		"Closing connection");
	die_on_error(amqp_destroy_connection(conn), "Ending connection");

	return AMQP_FUN_SUCCESS;
}

//����envelope.message.body.bytes����������ط�δ�����Ը���Ϊ���߳�
//����ʱҪע�⽫amqp��������Ϣ������Ա��ڴ����߳��ͷŸ���Դ��ʱ�򣬴��̲߳��ᱨ��
/*
* ������������Ĺ滮�����������ǹ���ͬ������֮�䣩�����ڴ˴��༭�������Ҫ�γ��ĵ�
* devid : xxxxx �豸���
* cmdid : xxxxx ������
* cmddata: xxxxx �����������ݣ�����Ϊ��
* ����Ϊ��������Ҫ�����ݣ���û��������Ϊ��
*/
static int json_parse_fun(char * amqp_message)
{
	JSON_Value * root_value = json_parse_string(amqp_message);
	//JSON_Value * val = NULL;
	JSON_Object * json_object_buf = NULL;

	if (root_value != NULL)//���Ϊ����������
	{
		//��ʼ�������ݣ���ֵӦ��Ϊһ��object�����򲻴����Ƿ���ʻ�����ԣ����ڽ�������ĵط���id�Ų��Ի����������Ӧ�ÿ�����������ظ�������Ϣ
		if (json_value_get_type(root_value) == JSONObject)
		{
			json_object_buf = json_value_get_object(root_value);
			//��ȡdevid�ַ�����ֵ
			char * devid_buf = json_object_get_string(json_object_buf, "devid");
			if (devid_buf != NULL && (strcmp(counter->sn, devid_buf) == 0) || (strcmp(" broadcast", devid_buf) == 0))
			{
				Check_Cmd(json_object_buf);
			}
			//val = json_object_get_value(json_object_buf, "devid");
			//if (val != NULL && json_value_get_type(val) == JSONString)
			//{
				/*
				*	������͹����������У�devid���ڱ����ӵ�id���ߵ��� broadcast ��˹��ӻ�����������ָ��
				*/
				//if ((strcmp(counter->sn, json_value_get_string(val)) == 0)  || (strcmp(" broadcast", json_value_get_string(val)) == 0))
				//{
					//Check_Cmd(json_object_buf);
				//}
			//}
		}
		else
		{
			//�����������ʹ���
			printf("Received message  not a JSON Object\r\n");
		}
	}
	else
	{
		//�����ַ����д���
		printf("Received message was not a JSON\r\n");
	}

	json_value_free(root_value);

	return AMQP_FUN_SUCCESS;

}

/*�����麯�����ú����Ĳ���json_object���������˳����������ϲ㺯���ͷ���Դ
* ���յ��������Ҫ���ݹ��ӵ�״̬ѡ���Ƿ���������������ھܽӽ��յ�״̬
* ��Ӧ�ô����κη������˷��͹���������
*/

static int Check_Cmd(JSON_Object * json_object)
{

	//JSON_Value * val = json_object_get_value(json_object, "cmdid");
	char * cmd = json_object_get_string(json_object, "cmdid");
	if (cmd != NULL)
	{
		//char * cmd = json_value_get_string(val);
		if (strcmp(cmd, "Shopping") == 0)//ִ��һ������
		{

			printf("ִ��һ������\r\n");
			//char * result_p = Procedure_Sales();

			char * result_p = Procedure_Sales_Ex(json_object);
			if (result_p != NULL)
			{
				Amqp_public_message(conn, "amq.direct", "server", result_p);
				free(result_p);
			}
			
		}
		else if (strcmp(cmd, "Unlock") == 0)
		{
			printf("ִ��һ�ο���\r\n");
			char * result_p = Procedure_Open_Lock();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Unlock_Close") == 0)
		{
			printf("ִ��һ�ο�����\r\n");
			char * result_p = Procedure_Open_Close();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Locker_State") == 0)
		{
			printf("��ȡ�����ŵ�״̬\r\n");
			char * result_p = Procedure_Get_Locker_State();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Basic_Value") == 0)//ָ�����ذ���ִ��ȥƤ
		{
			printf("ִ��һ��ȥƤ\r\n");
			char * result_p = Procedure_Basic_Value_Set(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Curavture_Value") == 0)//ָ�����ذ�������У׼
		{
			printf("ִ��һ������У׼\r\n");
			char * result_p = Procedure_Set_Curavture_Value(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Weight_Value") == 0)//ָ�����ذ��ŵ�������ȡ,����ָ�����
		{
			printf("ִ��һ�γ���\r\n");
			char * result_p = Procedure_Get_Weight_Value(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		else if (strcmp(cmd, "Board_State") == 0)//��ȡ���ذ�״̬
		{
			printf("ִ��һ�λ�ȡ���ذ�״̬\r\n");
			char * result_p = Procedure_Get_Board_State();
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
		}
		//else if (strcmp(cmd, "Basic_Value") == 0)//ִ��ȥƤ
		//{
		//	printf("ִ��һ��ȥƤ\r\n");
		//	char * result_p = Board_Basic_Value_Set_With_ACK();//������ȡ�Ż�
		//	Amqp_public_message(conn, "amq.direct", "server", result_p);
		//	free(result_p);
		//}
		else if (strcmp(cmd, "SQL_Select") == 0)//ִ��sql select��䣬�����Ծ����ѯû�в�ѯ�����Ҳû����
		{
			//ִ�д��������sql��䣬�����ؽ��,�����Ϊ���࣬select�����ɾ�ĵĽ��
			//select����ǲ�ѯ�����ݣ���ɾ�ĵĽ����Ӱ�����������
			printf("ִ��һ��sql select���\r\n");
			char * result_p =  Procedure_SQL_Select(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
			//printf("cmdid is : sql\r\n");
		}
		else if (strcmp(cmd, "SQL_Updata") == 0)//ִ��sql ��ɾ����䣬�˴�Updata����ָsql���updata�����Ǹ��µ���˼��������ָ��ɾ��
		{

			printf("ִ��һ��sql �������\r\n");
			char * result_p = Procedure_SQL_Updata(json_object);
			Amqp_public_message(conn, "amq.direct", "server", result_p);
			free(result_p);
			//Procedure_SQL_Select(json_object);
			//printf("cmdid is : sql\r\n");
		}
		else if (strcmp(cmd, "status") == 0)//��ȡ����״̬
		{
			//�˴��ɷ���
		}
		else
		{
			printf("Cmdid unknown!\n\n");
		}
	}

	return AMQP_FUN_SUCCESS;

}

//�������ݺ������򻯰�
/*
*	��Ϣ���ͺ����򻯰�
*	Ӧ�ð�channel�������Ҳ�ó���,�������Ŀǰֱ�Ӳ������ݿ������趨���
*/
static int Amqp_public_message(amqp_connection_state_t state, char * exchange, char * routingkey, char * message)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */
	die_on_error(amqp_basic_publish(conn, atoi(counter->channel), amqp_cstring_bytes(exchange),\
		         amqp_cstring_bytes(routingkey), 0, 0,&props, amqp_cstring_bytes(message)), "Publishing");

	return AMQP_FUN_SUCCESS;
}
