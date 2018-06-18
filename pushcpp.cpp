#include "pushcpp_internal.h"

pushcpp::pushcpp(
	const string &appKey,
	ConnectionEventHandler ch,
	ErrorEventHandler eh
)
{
	this->m_connectionEventHandler = ch;
	this->m_errorEventHandler = eh;
	stringstream str;
	str << "ws://ws-us2.pusher.com:80/app/";
	str << appKey;
	str << "?client=pushcpp&version=1.0&protocol=5";
	m_url = str.str();
}
void pushcpp::connect()
{
	DEBUG("Connecting.");
	assert(!this->m_eventThread);
	m_eventThread = new thread(&pushcpp::EventThread, this);
	assert(this->m_eventThread);
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
	if (wait && m_eventThread)
		m_eventThread->join();
}

void pushcpp::join()
{
	assert(this->m_eventThread);
	DEBUG("joining!");
	m_eventThread->join();
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
