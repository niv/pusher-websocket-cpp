#pragma once

#include "pushcpp.h"

#include <stdint.h>
#include <assert.h>

#include <sstream>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <jansson.h>
#include <queue>

#define DEBUG(f,...) { \
	if (getenv("PUSHCPP_DEBUG")) \
		fprintf(stderr, "pushcpp: " f "\n", ##__VA_ARGS__); \
}

#define ERROR(f,...) { \
	fprintf(stderr, "pushcpp (library/server problem): " f "\n", ##__VA_ARGS__); \
	fprintf(stderr, "This is a library problem and cannot be fixed automatically.\n"); \
	fprintf(stderr, "Must abort, sorry!\n"); \
	abort(); \
}

#include "easywsclient.hpp"

using easywsclient::WebSocket;

using namespace std;
