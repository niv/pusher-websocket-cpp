#include "pushcpp_internal.h"

void pushcpp::EventThread()
{
	while (true) {
		/* attempt to connect */
		DEBUG("polling thread started");

		while (
			this->m_websocket == NULL ||
			((WebSocket::pointer)this->m_websocket)->
			getReadyState() == WebSocket::CLOSED
		) {
			DEBUG("Attempting to connect!");

			if (this->m_websocket != NULL)
				delete((WebSocket::pointer) this->m_websocket);

			this->m_websocket = (void*) WebSocket::from_url(m_url);
		}

		WebSocket::pointer ws = (WebSocket::pointer) this->m_websocket;

		DEBUG("connected, (re)subscribing to channels");

		while (ws->getReadyState() != WebSocket::CLOSED) {
			ws->poll(100);
			ws->dispatch([this, ws](const string & msg) {
				WS_Dispatch(msg);
			});

			if (m_wantDisconnect)
				ws->close();
		}

		DEBUG("Lost connection, readyState: %d", ws->getReadyState());
		this->m_socketId = "";

		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++)
			it->second.clear();

		if (m_connectionEventHandler)
			m_connectionEventHandler(ConnectionEvent::DISCONNECTED);

		if (m_wantDisconnect)
			break;
	}

	m_wantDisconnect = false;

	DEBUG("thread was stopped");
}
