
#include <utility>
#include "httpserver.h"
#include "utils.h"
#include "logger.h"

std::string HttpServer::s_web_dir = "";
//mg_serve_http_opts HttpServer::s_server_option;
std::unordered_map<std::string, ReqHandler> HttpServer::s_handler_map;

std::string C_APIPATH[API_NUM] = {
	"/api/v1/login",  //0
	"/api/v1/logout",
	"/api/v1/config/setPassword",
	"/api/v1/config/getBoxId",
	"/api/v1/config/setBoxId",
	"/api/v1/config/getSysTime",  //5
	"/api/v1/config/setSysTime",
	"/api/v1/reboot",
	"/api/v1/config/getBasicInfo",
	"/api/v1/config/chunkUpload",
	"/api/v1/config/getNetwork",  //10
	"/api/v1/config/setNetwork",
	"/api/v1/config/getMqtt",
	"/api/v1/config/setMqtt",
	"/api/v1/config/getStatus",  
	"/api/v1/config/getCameras", //15
	"/api/v1/config/addCamera",
	"/api/v1/config/delCamera",
	"/api/v1/config/getRois",
	"/api/v1/config/setRois",  
	"/api/v1/config/getSnapshot", //20
	"/api/v1/config/getArmRules",
	"/api/v1/config/setArmRules",
	"/api/v1/config/getAlgoRules",
	"/api/v1/config/setAlgoRules",  
	"/api/v1/alarms/count", //25
	"/api/v1/alarms/query"
};

void HttpServer::Init(const std::string &port)
{
	m_port = port;
	//s_server_option.enable_directory_listing = "yes";
	//s_server_option.document_root = s_web_dir.c_str();

	m_blRunning = false;
	// 其他http设置

	// 开启 CORS，本项只针对主页加载有效
	// s_server_option.extra_headers = "Access-Control-Allow-Origin: *";
}

bool HttpServer::startsvr()
{
	if (m_blRunning)
		return true;
	m_blRunning = true;
	m_thread = std::thread(&HttpServer::Start, this);
	//setThreadName(m_thread.native_handle(), "HttpServer");

	//log_print(ZH_LOG_INFO, "HttpServer::start\n");
	return true;
}

void HttpServer::stopsvr(bool blExit)
{
	m_blRunning = false;

	std::this_thread::sleep_for(std::chrono::milliseconds(800));
	//log_print(ZH_LOG_INFO, "HttpServer::stop \n");
}

static const char *s_debug_level = "2";
static const char *s_root_dir = ".";
static const char *s_enable_hexdump = "no";
static const char *s_ssi_pattern = "#.html";

static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) 
{
	if (ev == MG_EV_HTTP_MSG) {
		struct mg_http_message *hm = (mg_http_message *)ev_data, tmp = { 0 };
		struct mg_str unknown = mg_str_n("?", 1), *cl;
		struct mg_http_serve_opts opts = { 0 };
		opts.root_dir = s_root_dir;
		opts.ssi_pattern = s_ssi_pattern;
		mg_http_serve_dir(c, hm, &opts);
		mg_http_parse((char *)c->send.buf, c->send.len, &tmp);
		cl = mg_http_get_header(&tmp, "Content-Length");
		if (cl == NULL) cl = &unknown;
		MG_INFO(("%.*s %.*s %.*s %.*s", (int)hm->method.len, hm->method.ptr,
			(int)hm->uri.len, hm->uri.ptr, (int)tmp.uri.len, tmp.uri.ptr,
			(int)cl->len, cl->ptr));
	}
	(void)fn_data;
}

bool HttpServer::Start()
{
	mg_mgr_init(&m_mgr);

	printf("HttpServer Start %s\n", m_listening_address.c_str());
	m_listening_address = "http://0.0.0.0:" + m_port;
	//mg_connection *connection = mg_http_listen(&m_mgr, m_listening_address.c_str(), cb, &m_mgr);
	mg_connection *connection = mg_http_listen(&m_mgr, m_listening_address.c_str(), HttpServer::HandleHttpEvent, &m_mgr);
	
	if (connection == NULL)
		return false;

	printf("HttpServer Start loop");
	// for both http and websocket
	//mg_set_protocol_http_websocket(connection);

	//log_print(ZH_LOG_INFO, "starting http server at port: %s\n", m_port.c_str());
	// loop
	while (m_blRunning)
		mg_mgr_poll(&m_mgr, 500); // ms

	return true;
}

void HttpServer::OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data)
{
	//// 区分http和websocket
	//if (event_type == MG_EV_HTTP_REQUEST)
	//{
	//	http_message *http_req = (http_message *)event_data;
	//	HandleHttpEvent(connection, http_req);
	//}
	//else if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE ||
	//	event_type == MG_EV_WEBSOCKET_FRAME ||
	//	event_type == MG_EV_CLOSE)
	//{
	//	websocket_message *ws_message = (struct websocket_message *)event_data;
	//	HandleWebsocketMessage(connection, event_type, ws_message);
	//}
}

// ---- simple http ---- //
static bool route_check(mg_http_message *http_msg, char *route_prefix)
{
	if (mg_vcmp(&http_msg->uri, route_prefix) == 0)
		return true;
	else
		return false;

	// TODO: 还可以判断 GET, POST, PUT, DELTE等方法
	if (mg_vcmp(&http_msg->method, "GET") >= 0 || mg_vcmp(&http_msg->method, "POST") >= 0 || mg_vcmp(&http_msg->method, "PUT") >= 0)
		return true;
	else
		return false;
	//mg_vcmp(&http_msg->method, "PUT");
	//mg_vcmp(&http_msg->method, "DELETE");
}

void HttpServer::AddHandler(const std::string &url, ReqHandler req_handler)
{
	if (s_handler_map.find(url) != s_handler_map.end())
		return;
	printf("AddHandler %s handle=%p\n", url.c_str(), req_handler);
	s_handler_map.insert(std::make_pair(url, req_handler));
}

void HttpServer::RemoveHandler(const std::string &url)
{
	auto it = s_handler_map.find(url);
	if (it != s_handler_map.end())
		s_handler_map.erase(it);
}

void HttpServer::SendHttpRsp200(mg_connection *connection, std::string rsp)
{
	// --- 未开启CORS
	// 必须先发送header, 暂时还不能用HTTP/2.0
	//mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
	// --- 开启CORS
	mg_printf(connection, "HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\n"
		"Cache-Control: no-cache\n"
		"Content-Length: %d\n"
		"Access-Control-Allow-Origin: *\n\n"
		"%s\n", rsp.length(), rsp.c_str());

	// 以json形式返回
	mg_http_printf_chunk(connection, rsp.c_str());
	// 发送空白字符快，结束当前响应
	mg_http_printf_chunk(connection, "", 0);
}

void HttpServer::SendHttpRsp405(mg_connection *connection)
{
	// --- 未开启CORS
	// 必须先发送header, 暂时还不能用HTTP/2.0
	mg_printf(connection, "%s", "HTTP/1.1 405 Method not allowed\r\n Content-Length: 0\r\n\r\n");

	mg_http_printf_chunk(connection, "", 0);

	// --- 开启CORS
	/*mg_printf(connection, "HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\n"
	"Cache-Control: no-cache\n"
	"Content-Length: %d\n"
	"Access-Control-Allow-Origin: *\n\n"
	"%s\n", rsp.length(), rsp.c_str()); */
}

void HttpServer::HandleHttpEvent(mg_connection *connection, int ev, void * ev_data, void *fn_data)
{
	if (ev != MG_EV_HTTP_MSG) 
	{
		return;
	}
	//mg_http_serve_dir(connection, http_req, &opts);

	mg_http_message *http_req = (mg_http_message *)ev_data;
	std::string req_str = std::string(http_req->message.ptr, http_req->message.len);
	//log_print(ZH_LOG_TRACE, "got request: %s\n", req_str.c_str());
	std::string sztoken = "";

	// 先过滤是否已注册的函数回调
	std::string url = std::string(http_req->uri.ptr, http_req->uri.len);

	for (int i = 0; http_req->headers[i].name.len > 0; i++)
	{
		if ((strcmp(http_req->headers[i].value.ptr, "token") == 0) || (strcmp(http_req->headers[i].value.ptr, "Token") == 0))
		{
			sztoken = http_req->headers[i].value.ptr;
		}
	}

	printf("get token", sztoken.c_str());

	std::string method = std::string(http_req->method.ptr, http_req->method.len);
	std::string body = std::string(http_req->body.ptr, http_req->body.len);
	std::string query_string = std::string(http_req->query.ptr, http_req->query.len);

	if (route_check(http_req, (char*)"/")) // index page
	{
		//serve_http(connection, http_req, s_server_option);
		return;
	}
	else if (route_check(http_req, (char*)"/apitest/echo"))
	{
		// 直接回传
		SendHttpRsp200(connection, "welcome to api ");
		return;
		//log_print(ZH_LOG_TRACE, "respond request: welcome to httpserver \n");
	}

	int nFlag = 0;
	for (int i = 0; i < API_NUM; i++)
	{
		if (route_check(http_req, (char*)C_APIPATH[i].c_str()))
		{
			nFlag = 1;
			break;
		}
	}

	if (0 == nFlag)
	{
		SendHttpRsp405(connection);
		return;
		//res = "HTTP/1.1 405 Method not allowed\r\n Content-Length: 0\r\n\r\n";
	}

	auto it = s_handler_map.find(url);
	printf("get request: %s\n", url.c_str());
	if (it != s_handler_map.end())
	{
		ReqHandler handle_func = it->second;
		std::string sz;
		int nres = 0;

		printf("found request: %s\n", url.c_str());
		//handle_func(url, body, connection, &HttpServer::SendHttpRsp);
		uint32_t nip = *(uint32_t*)&(connection->rem.ip);
		handle_func(query_string, nip, sztoken, body, sz);

		//if (bl == true)
			SendHttpRsp200(connection, sz);

			printf("respond request: %s\n", sz.c_str());
		//else
		//	SendHttpRsp405(connection);

			//reboot!!!! net && reboot ...
			system("sh ./reboot.sh");
		//log_print(ZH_LOG_TRACE, "respond request: %s\n", sz.c_str());
		return;
	}

	// 其他请求

	//else if (route_check(http_req, "/api/lpr"))
	//{
	//	//// 简单post请求，加法运算测试
	//	//char n1[100], n2[100];
	//	//double result;

	//	///* Get form variables */
	//	//mg_get_http_var(&http_req->body, "n1", n1, sizeof(n1));
	//	//mg_get_http_var(&http_req->body, "n2", n2, sizeof(n2));

	//	///* Compute the result and send it back as a JSON object */
	//	//result = strtod(n1, NULL) + strtod(n2, NULL);

	//	std::string result = "";
	//	SendHttpRsp(connection, result);
	//}
	else
	{
		mg_printf(
			connection,
			"%s",
			"HTTP/1.1 501 Not Implemented\r\n"
			"Content-Length: 0\r\n\r\n");
		//log_print(ZH_LOG_WARN, "request URL invalid!\n");
	}
}
//
//// ---- websocket ---- //
//int HttpServer::isWebsocket(const mg_connection *connection)
//{
//	//return connection->flags & MG_F_IS_WEBSOCKET;
//}
//
//void HttpServer::HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg)
//{
//	if (event_type == MG_EV_WEBSOCKET_HANDSHAKE_DONE)
//	{
//		printf("client websocket connected\n");
//		// 获取连接客户端的IP和端口
//		char addr[32];
//		mg_sock_addr_to_str(&connection->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
//		printf("client addr: %s\n", addr);
//
//		// 添加 session
//		s_websocket_session_set.insert(connection);
//
//		SendWebsocketMsg(connection, "client websocket connected");
//	}
//	else if (event_type == MG_EV_WEBSOCKET_FRAME)
//	{
//		mg_str received_msg = {
//			(char *)ws_msg->data, ws_msg->size
//		};
//
//		char buff[1024] = { 0 };
//		strncpy(buff, received_msg.p, received_msg.len); // must use strncpy, specifiy memory pointer and length
//
//														 // do sth to process request
//		printf("received msg: %s\n", buff);
//		SendWebsocketMsg(connection, "send your msg back: " + std::string(buff));
//		//BroadcastWebsocketMsg("broadcast msg: " + std::string(buff));
//	}
//	else if (event_type == MG_EV_CLOSE)
//	{
//		if (isWebsocket(connection))
//		{
//			printf("client websocket closed\n");
//			// 移除session
//			if (s_websocket_session_set.find(connection) != s_websocket_session_set.end())
//				s_websocket_session_set.erase(connection);
//		}
//	}
//}
//
//void HttpServer::SendWebsocketMsg(mg_connection *connection, std::string msg)
//{
//	//mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
//}
//
//void HttpServer::BroadcastWebsocketMsg(std::string msg)
//{
//	//for (mg_connection *connection : s_websocket_session_set)
//	//	mg_send_websocket_frame(connection, WEBSOCKET_OP_TEXT, msg.c_str(), strlen(msg.c_str()));
//}

bool HttpServer::Close()
{
	mg_mgr_free(&m_mgr);
	return true;
}
