#pragma once

#include <string>
#include <set>
#include <unordered_map>
#include <vector>
#include <utility>
#include <functional>

class pushcpp
{
public:
	enum class ConnectionEvent {
		CONNECTED       = 0,
		DISCONNECTED    = 1
	};
	enum class PingEvent {
		PING = 1,
	};
	typedef void (*ChannelEventHandler)(
		const std::string &channel,
		const std::string &event,
		const std::string &eventData);

	typedef void (*ChannelPresenceHandler)(
		const std::string &channel
	);

	/**
	 * Return this for authentication requests.
	 */
	struct ChannelAuthentication {
		std::string auth;
		std::string channelData;
	};

	/**
	 * Called when we want authentication data.
	 * You need to do a pusher API request here as described on
	 * https://pusher.com/docs/auth_signatures. This probably involves
	 * making a HTTP request to your custom backend, and as such it is
	 * left as an exercise to the reader!
	 *
	 * Blocking inside this function will block either the subscribe()
	 * method call that originates this, or the pusher event thread.
	 * This might change in the future.
	 */
	typedef ChannelAuthentication(*ChannelAuthHandler)(
		const std::string &socketId,
		const std::string &channel
	);

	struct ChannelData {
		bool subscribed = false;
		ChannelAuthHandler authHandler;
		std::vector<std::function<void(const std::string&,
									   const std::string&,
									   const std::string&)>> eventHandlers;
		std::set<std::string> presenceMemberIds;

		void clear()
		{
			subscribed = false;
			presenceMemberIds.clear();
		}
	};

	/**
	 * Sets up this client and configures it.
	 * This does not connect yet.
	 */
	pushcpp(
		const std::string &appKey,
		std::function<void(const ConnectionEvent ce, const std::string&)> ch = nullptr,
		std::function<void(const int, const std::string&)> eh = nullptr,
		std::function<void(const PingEvent p)> pe = nullptr,
		const std::string &cluster = ""
	);

	/**
	 * Connects to the configured remote URL.
	 *
	 * This needs to be called once, and it will reconnect
	 * transparently on socket failure. This starts a new thread
	 * in the background.
	 */
	void connect();

	/**
	 * Disconnects from Pusher, stopping the event thread.
	 *
	 * This does NOT invalidate subscriptions; on connect(), they will
	 * be re-subscribed.
	 *
	 * Setting wait to true will wait until the socket is closed.
	 */
	void disconnect(bool wait = false);

	/**
	 * Subscribe to a channel. The given event handler
	 * will be called when a channel receives a message.
	 *
	 * You can call this at any time, no matter the connection state.
	 * Channels will be automatically resubscribed to on reconnect.
	 *
	 * Repeatedly subscribing to the same channel will do nothing.
	 *
	 * Will return true if the subscription was sent out immediately (since
	 * we were connected), false otherwise.
	 */
	bool subscribe(
		const std::string &channel,
		/**
		 * This handler receives all channel events, including
		 * internal ones, in case you want to act on them.
		 */
		 std::function<void(const std::string&,
 						    const std::string&,
 						    const std::string&)> event,
		/**
		 * This is called for authentication requests as described
		 * above. presence- and private- channels require this.
		 */
		ChannelAuthHandler auth = NULL
	);

	/**
	 * Unsubscribes you from this channel.
	 * This will immediately destroy all associated data.
	 */
	void unsubscribe(const std::string &channel);

	/**
	 * Triggers a client event on the given channel.
	 * The library adds "client-" in front of the event name if it's missing,
	 * so you can leave it off if you are lazy.
	 *
	 * Please note that as per the pusher spec, channel
	 * needs to be a "private-" or "presence-" channel.
	 */
	bool trigger(
		const std::string &channel,
		const std::string &event,
		const std::string &data
	);

	/**
	 * Sends a event object to Pusher. data can be any binary data; it will
	 * be transparently en/decoded.
	 * Channel and data can be "".
	 * This is mostly for internal use.
	 * Will return true if the message was sent, false otherwise. */
	bool send(
		const std::string &channel,
		const std::string &event,
		const std::string &data
	);

	/**
	 * Send some raw data over the websocket. For pusher, this needs
	 * to be valid json - see https://pusher.com/docs/pusher_protocol
	 * This is mostly for internal use.
	 * Will return true if the message was sent, false otherwise. */
	bool sendRaw(const std::string &raw);

	/**
	 * Returns true if we are currently connected.
	 */
	bool connected() const;

	/**
	 * Returns a set of our currently subscribed-to channels.
	 * If you only want channels we received confirmation to on,
	 * pass confirmedOnly = true.
	 */
	std::unordered_map<std::string, ChannelData>
	subscriptions(bool confirmedOnly = true) const;

	const std::string & socketId() const
	{
		return m_socketId;
	}
	void Execute();

private:
	// Dont allow copying.
	// No point in opening a second connection, is there?
	pushcpp(const pushcpp&);
	pushcpp& operator=(const pushcpp&);

	std::string m_url;

	bool m_wantDisconnect = false;
	std::string m_socketId;
	// The current connection, if any!
	void *m_websocket = NULL;

	void WS_Dispatch(const std::string & message);

	/* Send a subscription request to Pusher. You do not need to call this
	 * yourself; use subscribe() instead!
	 * Will return true if the message was sent, false otherwise. */
	bool sendSubscription(
		bool subscribe,
		const std::string &channel
	);
	std::function<void(const ConnectionEvent ce, const std::string&)> m_connectionEventHandler;
	std::function<void(const int, const std::string&)> m_errorEventHandler;
	std::function<void(const PingEvent p)> m_pingEventHandler;

	// The complete list of channels we (want to) subscribe to.
	// This includes channels we were rejected from.
	std::unordered_map<std::string, ChannelData> m_channelData;

	bool request_connection_;
};
