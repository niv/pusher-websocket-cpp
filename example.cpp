#include "pushcpp.h"
#include <iostream>

using namespace std;

void cn_ev(const pushcpp::ConnectionEvent ev);
void er_ev(const int code, const std::string &msg);

pushcpp pp("my-api-key", cn_ev, er_ev);

void cn_ev(const pushcpp::ConnectionEvent ev)
{
	printf("ConnectEvent: %d\n", ev);
	pp.send("channel", "pusher:subscribe", "lol");
}

void er_ev(const int code, const std::string &msg)
{
	cout << "Error from pusher: " << code << " "  << msg << endl;
}

void sub_ev(
	const string &channel,
	const string &event,
	const string &data
)
{
	// cout << pp.subscriptions().size() << endl;

	printf("Received event %s on channel %s: %s\n",
		   event.c_str(), channel.c_str(), data.c_str());

	auto cd = pp.subscriptions();

	cout << "Subs: " << endl;
	for (
		auto it = cd.begin();
		it != cd.end();
		it++
	) {
		cout << "Subscription: " << it->first <<
			 " status: " << it->second.subscribed <<
			 " members: " << it->second.presenceMemberIds.size() <<
			 endl;
	}

	// for (auto it = ChannelData

	pp.unsubscribe(channel);

	cd = pp.subscriptions();

	cout << "Subs: " << endl;
	for (
		auto it = cd.begin();
		it != cd.end();
		it++
	) {
		cout << "Subscription: " << it->first <<
			 " status: " << it->second.subscribed <<
			 " members: " << it->second.presenceMemberIds.size() <<
			 endl;
	}

	// cout << pp.subscriptions().size() << endl;
}

int main()
{
	pp.subscribe("my_channel", sub_ev);

	pp.connect();

	pp.join();
	return 0;
}
