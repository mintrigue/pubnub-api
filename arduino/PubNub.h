#ifndef PubNub_h
#define PubNub_h

#include <stdint.h>
#include <Ethernet.h>


/* Some notes:
 *
 * (i) There is no SSL support on Arduino, it is unfeasible with
 * Arduino Uno or even Arduino Mega's computing power and memory limits.
 * All the traffic goes on the wire unencrypted and unsigned.
 *
 * (ii) We re-resolve the origin server IP address before each request.
 * This means some slow-down for intensive communication, but we rather
 * expect light traffic and very long-running sketches (days, months),
 * where refreshing the IP address is quite desirable.
 *
 * (iii) We let the users read replies at their leisure instead of
 * returning an already preloaded string so that (a) they can do that
 * in loop() code while taking care of other things as well (b) we don't
 * waste precious RAM by pre-allocating buffers that are never needed.
 *
 * (iv) If you are having problems connecting, maybe you have hit
 * a bug in Debian's version of Arduino pertaining the DNS code. Try using
 * an IP address as origin and/or upgrading your Arduino package.
 *
 * (v) We assume that server replies always use Transfer-encoding: chunked;
 * adding auto-detection would be straightforward if that ever changes.
 * Adding support for multiple chunks is going to be possible, not so
 * trivial though if we are to shield the user application from chunked
 * encoding. Note that /history still uses non-chunked encoding.
 */


/* This class is a thin EthernetClient wrapper whose goal is to
 * automatically acquire time token information when reading
 * subscribe call response.
 *
 * (i) The user application sees only the JSON body, not the timetoken.
 * As soon as the body ends, PubSubclient reads the rest of HTTP reply
 * itself and disconnects. The stored timetoken is used in the next call
 * to the PubSub::subscribe method then. */
class PubSubClient : public EthernetClient {
public:
	PubSubClient() :
		EthernetClient(), json_enabled(false)
	{
		strcpy(timetoken, "0");
	}

	/* Customized functions that make reading stop as soon as we
	 * have hit ',' outside of braces and string, which indicates
	 * end of JSON body. */
	virtual int read();
	virtual int read(uint8_t *buf, size_t size);
	virtual void stop();

	/* Enable the JSON state machine. */
	void start_body();

	inline char *server_timetoken() { return timetoken; }

private:
	void _state_input(uint8_t ch, uint8_t *nextbuf, size_t nextsize);
	void _grab_timetoken(uint8_t *nextbuf, size_t nextsize);

	/* JSON state machine context */
	bool json_enabled:1;
	bool in_string:1;
	bool after_backslash:1;
	int braces_depth;

	/* Time token acquired during the last subscribe request. */
	char timetoken[22];
};


class PubNub {
public:
	/* Init the Pubnub Client API
	 *
	 * This should be called after Ethernet.begin().
	 * Note that the string parameters are not copied; do not
	 * overwrite or free the memory where you stored the keys!
	 * (If you are passing string literals, don't worry about it.)
	 * Note that you should run only a single publish at once.
	 *
	 * @param string publish_key required key to send messages.
	 * @param string subscribe_key required key to receive messages.
	 * @param string origin optional setting for cloud origin.
	 * @return boolean whether begin() was successful. */
	bool begin(char *publish_key, char *subscribe_key, char *origin = "pubsub.pubnub.com");

	/* Publish
	 *
	 * Send a message (assumed to be well-formed JSON) to a given channel.
	 *
	 * Note that the reply can be obtained using code like:
	     client = publish("demo", "\"lala\"");
	     if (!client) return; // error
	     while (client->connected()) {
	       while (client->connected() && !client->available()) ; // wait
	       char c = client->read();
	       Serial.print(c);
	     }
	     client->stop();
	 * You will get content right away, the header has been already
	 * skipped inside the function. If you do not care about
	 * the reply, just call client->stop(); immediately.
	 *
	 * @param string channel required channel name.
	 * @param string message required message string in JSON format.
	 * @return string Stream-ish object with reply message or NULL on error. */
	EthernetClient *publish(char *channel, char *message);

	/**
	 * Subscribe
	 *
	 * Listen for a message on a given channel. The function will block
	 * and return when a message arrives. Typically, you will run this
	 * function from loop() function to keep listening for messages
	 * indefinitely.
	 *
	 * As a reply, you will get a JSON array with messages, e.g.:
	 * 	["msg1",{msg2:"x"}]
	 * and so on. Empty reply [] is also normal and your code must be
	 * able to handle that. Note that the reply specifically does not
	 * include the time token present in the raw reply.
	 *
	 * @param string channel required channel name.
	 * @return string Stream-ish object with reply message or NULL on error. */
	PubSubClient *subscribe(char *channel);

	/**
	 * History
	 *
	 * Receive list of the last N messages on the given channel.
	 *
	 * @param string channel required channel name.
	 * @param int limit optional number of messages to retrieve.
	 * @return string Stream-ish object with reply message or NULL on error. */
	EthernetClient *history(char *channel, int limit = 10);

private:
	bool _request_bh(EthernetClient &client, bool chunked = true);

	char *publish_key, *subscribe_key;
	char *origin;

	EthernetClient publish_client, history_client;
	PubSubClient subscribe_client;
};

extern class PubNub PubNub;

#endif
