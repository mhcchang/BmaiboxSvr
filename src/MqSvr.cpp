#include "MqSvr.h"
#include <string>

#define ADDRESS     "tcp://mqtt.eclipseprojects.io:1883"
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

//#include "Log4j.h"
#include "logger.h"

MqSvr::MqSvr()
{
	m_blInit = false;
}

MqSvr::~MqSvr()
{
	if (!m_blInit)
		return;
	int rc;
	if ((rc = MQTTClient_disconnect(m_client, 10000)) != MQTTCLIENT_SUCCESS)
		MHC_DEBUG("Failed to disconnect, return code {}\n", rc);
	MQTTClient_destroy(&m_client);
}

bool MqSvr::PublishMsg(std::string msg)
{
	int rc;
	m_pubmsg.payload = (void*)msg.c_str();
	m_pubmsg.payloadlen = msg.length();
	m_pubmsg.qos = QOS;
	m_pubmsg.retained = 0;
	rc = MQTTClient_publishMessage(m_client, m_topic.c_str(), &m_pubmsg, &m_token);
	if (rc != MQTTCLIENT_SUCCESS)
	{
		MHC_DEBUG("Failed to publish message, return code {}\n", rc);
		return false;
		//exit(EXIT_FAILURE);
	}

	MHC_DEBUG("Waiting for up to %d seconds for publication of {}\n"
		"on topic %s for client with ClientID: {}\n",
		(int)(TIMEOUT / 1000), PAYLOAD, TOPIC, CLIENTID);
	rc = MQTTClient_waitForCompletion(m_client, m_token, TIMEOUT);
	MHC_DEBUG("Message with delivery token {} delivered\n", m_token);

	return MQTTCLIENT_SUCCESS == rc;
}

int MqSvr::Init(std::string addr, std::string clientid, std::string topic)
{
	int rc;
	m_addr = addr;
	m_clientid = clientid;
	m_topic = topic;
	if ((rc = MQTTClient_create(&m_client, m_addr.c_str(), m_clientid.c_str(),
		MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
	{
		MHC_ERROR("Failed to create client, return code {}\n", rc);
		//exit(EXIT_FAILURE);
	}

	m_conn_opts.keepAliveInterval = 20;
	m_conn_opts.cleansession = 1;
	if ((rc = MQTTClient_connect(m_client, &m_conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		MHC_ERROR("mq Failed to connect, return code {}\n", rc);
		//exit(EXIT_FAILURE);
	}
	m_blInit = MQTTCLIENT_SUCCESS == rc;
	return rc;
}
