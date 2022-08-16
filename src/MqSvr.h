#pragma once

#if __WINDOWS_OS__
#pragma warning (push)
#pragma warning (disable: 4251 4244)
#else
/////gunc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#include "utils.h"
#include "MQTTClient.h"

#include "defs.h"

class MqSvr {
public:
	MqSvr();
	~MqSvr();

	int Init(std::string addr, std::string clientid, std::string topic);
	bool PublishMsg(std::string msg);
	
private:
	MQTTClient m_client;
	MQTTClient_connectOptions m_conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_message m_pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken m_token;

	bool m_blInit;

	std::string m_addr;
	std::string m_clientid;
	std::string m_topic;
};
