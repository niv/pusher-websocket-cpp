#include "pushcpp_internal.h"

bool pushcpp::trigger(
	const std::string &channel,
	const std::string &event,
	const std::string &data
)
{
	string myev = event;

	if (myev.compare(0, 7, "client-", 0, 7))
		myev = "client-" + myev;

	send(channel, event, data);
}
