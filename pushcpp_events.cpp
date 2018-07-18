#include "pushcpp_internal.h"

void pushcpp::WS_Dispatch(const string & message)
{
	DEBUG("<- %s", message.c_str());

	json_error_t error;
	json_t *json = json_loadb((const char*) message.data(),
							  message.size(), 0, &error);

	if (!json)
		ERROR("Error parsing json data from server: %s\ndata was: %s",
			  error.text, message.c_str());

	const string event = json_string_value(json_object_get(json, "event"));
	const string channel = json_object_get(json, "channel") ?
						   json_string_value(json_object_get(json, "channel")) : "";
	const string sdata = json_object_get(json, "data") ?
						 (json_is_string(json_object_get(json, "data")) ?
						  json_string_value(json_object_get(json, "data")) : "")
						 : "";

	if (event == "pusher:connection_established") {
		json_t *jdata = json_loadb(sdata.data(), sdata.size(), 0, &error);
		assert(jdata); // should never fail unless pusher backend breaks
		json_t *jdata_conn = json_object_get(json, "data");
		string data_conn = "";

		if (json_is_string(jdata_conn))
			data_conn = json_string_value(json_object_get(json, "data"));
		this->m_socketId = json_string_value(json_object_get(jdata, "socket_id"));
		DEBUG("our socket id is: %s", this->m_socketId.c_str());

		// Re/subscribe to all our channels!
		for (
			auto it = m_channelData.begin();
			it != m_channelData.end();
			it++
		) {
			sendSubscription(true, it->first);
		}

		if (m_connectionEventHandler)
			m_connectionEventHandler(ConnectionEvent::CONNECTED, data_conn);

		json_decref(json);
		json_decref(jdata);
		return;
	}

	// { event, channel, data:str => {
	// 	ids:array[str], hash:hash=>{id=>user_data..}, count:int }}
	if (event == "pusher_internal:subscription_succeeded") {
		m_channelData[channel].subscribed = true;

		json_t *jdata = json_loadb(sdata.data(), sdata.size(), 0, &error);
		assert(jdata); // should never fail unless pusher backend breaks

		json_t *arr = json_object_get(json_object_get(jdata, "presence"), "ids");

		for (size_t i = 0; i < json_array_size(arr); i++)
			m_channelData[channel].presenceMemberIds.insert(
				json_string_value(json_array_get(arr, i))
			);

		json_decref(jdata);
		// json_decref(json);
		// return;
	}

	// { event, channel, data:str => { user_id:str, user_info: hashornull }}
	if (event == "pusher_internal:member_added") {
		json_t *jdata = json_loadb(sdata.data(), sdata.size(), 0, &error);
		assert(jdata); // should never fail unless pusher backend breaks

		if (m_channelData.find(channel) != m_channelData.end())
			m_channelData[channel].presenceMemberIds.insert(
				json_string_value(json_object_get(jdata, "user_id"))
			);

		json_decref(jdata);
		// json_decref(json);
		// return;
	}

	// { event, channel, data:str => { user_id: str }}
	if (event == "pusher_internal:member_removed") {
		json_t *jdata = json_loadb(sdata.data(), sdata.size(), 0, &error);
		assert(jdata); // should never fail unless pusher backend breaks

		if (m_channelData.find(channel) != m_channelData.end())
			m_channelData[channel].presenceMemberIds.erase(
				json_string_value(json_object_get(jdata, "user_id"))
			);

		json_decref(jdata);
		// json_decref(json);
		// return;
	}

	if (event == "pusher:pong") {
		//send("", "pusher:pong", "");
		if (m_pingEventHandler)
			m_pingEventHandler(PingEvent::PING);
		json_decref(json);
		return;
	}

	if (event == "pusher:error") {
		if (m_errorEventHandler) {
			json_t *jdata = json_object_get(json, "data");
			int code = json_integer_value(json_object_get(jdata, "code"));
			string message = json_string_value(json_object_get(jdata, "message"));

			m_errorEventHandler(code, message);
		}

		json_decref(json);
		return;
	}

	bool is_pusher = !event.compare(0, 7, "pusher:", 0, 7);
	bool is_pusher_internal = !event.compare(0, 16, "pusher_internal:", 0, 16);

	if (
		// Optional: Don't show pusher_internal messages to callbacks.
		// !is_pusher && !is_pusher_internal &&
		NULL != json_object_get(json, "channel")
	) {
		json_t *jdata = json_object_get(json, "data");
		string data = "";

		if (json_is_string(jdata))
			data = json_string_value(json_object_get(json, "data"));
		else {
			// We .. just re-encode as json here since we can't pass jansson
			// objects to the client. Boo.
			char *err = json_dumps(json, 0);
			data = err;
			free(err);
		}

		auto it = m_channelData.find(channel);

		if (it != m_channelData.end())
			for (
				auto it2 = it->second.eventHandlers.begin();
				it2 != it->second.eventHandlers.end();
				it2++
			)
				(*it2)(channel, event, data);
	}

	json_decref(json);
}
