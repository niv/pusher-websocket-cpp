# Pusher C++ WebSocket Client

This is a very rudimentary pusher.com C++ *client* (not http/REST),
using the pusher.com websocket API.

It is very much a work-in-progress project that arose out of a need
to have a C++ client for a very specific application (wintershards.net).

I have tried to keep the API as simple as possible. Please feel free
to use it for your own projects, but *evaluate* it first. I make no claims
for it to be bug-free or functionally competent.

## It currently supports:

* Connecting to Pusher and keeping the connection up in a background thread
* Callback for connection state change events
* Callback for errors
* Keeping track of subscribed channels, automatic resubscription
* Support for private- channels
* Support for presence- channels, including tracking members

## It does not support:

* SSL
* Keeping track of presence userInfo data
* ?? All other things not mentioned.


## Dependencies

* C++11
* libjansson for json parsing/generating (```libjansson-dev``` on debianlikes).

Internally, it's currently using https://github.com/dhbaird/easywsclient,
because that allowed me to get started quickly. :) However, because of that,
there is no SSL support.

easywsclient is ```Copyright (c) 2012, 2013 <dhbaird@gmail.com>```
under the MIT licence.

Please see the linked repository for more information.

## Usage

Please see ```pushcpp.h```, and the included ```example.cpp```.
