#include "pushcpp_internal.h"

void pushcpp::Execute() {
	if (request_connection_) {
		if (this->m_websocket == NULL ||
			((WebSocket::pointer)this->m_websocket)->getReadyState()
		 		== WebSocket::CLOSED) {
			// logger attempt to connecte
			if (this->m_websocket != NULL)
				delete((WebSocket::pointer) this->m_websocket);
			this->m_websocket = (void*) WebSocket::from_url(m_url);
			return;
		}
		WebSocket::pointer ws = (WebSocket::pointer) this->m_websocket;
		// connected log (subs.. resubs..)
		if (ws->getReadyState() != WebSocket::CLOSED) {
			ws->poll(100);
			ws->dispatch([this, ws](const string &msg) {
				WS_Dispatch(msg);
			});
			if (m_wantDisconnect) {
				std::cout << "pusher-WebSocket: disconnecting." << std::endl;
				ws->close();
				this->m_socketId = "";
				for (auto it = m_channelData.begin(); it != m_channelData.end(); it++) {
					it->second.clear();
				}
				if (m_connectionEventHandler) {
					m_connectionEventHandler(ConnectionEvent::DISCONNECTED, "");
				}
				request_connection_ = false;
			}
			return;
		}
		// add log connection Lost
		this->m_socketId = "";
		for (auto it = m_channelData.begin(); it != m_channelData.end(); it++) {
			it->second.clear();
		}
		if (m_connectionEventHandler) {
			m_connectionEventHandler(ConnectionEvent::DISCONNECTED, "");
		}
		if (m_wantDisconnect) {
			m_wantDisconnect = false;
			return;
		}
	}
}
