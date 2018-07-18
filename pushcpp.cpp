#include "pushcpp_internal.h"

pushcpp::pushcpp(const string &appKey,
				 std::function<void(const ConnectionEvent ce, const std::string&)> ch,
			     std::function<void(const int, const std::string&)> eh,
				 std::function<void(const PingEvent p)> pe,
			 	 const std::string &cluster)
				 : request_connection_(false) {
	this->m_connectionEventHandler = ch;
	this->m_errorEventHandler = eh;
	this->m_pingEventHandler = pe;
	stringstream str;
	str << "ws://ws";
	if (!cluster.empty()) {
		str << "-";
		str << cluster;
	}
	str << ".pusher.com:80/app/";
	str << appKey;
	str << "?client=pushcpp&version=1.0&protocol=5";
	m_url = str.str();
}
void pushcpp::connect()
{
	request_connection_ = true;
	m_wantDisconnect = false;
}

bool pushcpp::connected() const
{
	return
		this->m_websocket != NULL && (
			((WebSocket::pointer) this->m_websocket)->
			getReadyState() == WebSocket::OPEN
		);
}

void pushcpp::disconnect(bool wait)
{
	m_wantDisconnect = true;
}

bool pushcpp::sendRaw(const string &raw)
{
	WebSocket::pointer ws = (WebSocket::pointer) this->m_websocket;

	if (ws != NULL && ws->getReadyState() == WebSocket::OPEN) {
		DEBUG("send: %s", raw.c_str());
		ws->send(raw);
		return true;
	}

	return false;
}

bool pushcpp::send(
	const string &channel,
	const string &event,
	const string &data
)
{
	json_t *json = json_object();
	json_object_set_new(json, "event", json_string(event.c_str()));

	if (channel != "")
		json_object_set_new(json, "channel", json_string(channel.c_str()));

	json_object_set_new(json, "data", json_string(data.c_str()));
	char *dumped = json_dumps(json, 0);
	assert(dumped);
	json_decref(json);

	bool ret = sendRaw(dumped);
	free(dumped);
	return ret;
}
