#include "http.h"

const char *http_strmsg(int code)
{
	static const char **msgxxx[] = {
		0, http_msg1xx, http_msg2xx, http_msg3xx, http_msg4xx, http_msg5xx
	};
	static int msgcount[] = {
		0,
		sizeof http_msg1xx / sizeof *http_msg1xx,
		sizeof http_msg2xx / sizeof *http_msg2xx,
		sizeof http_msg3xx / sizeof *http_msg3xx,
		sizeof http_msg4xx / sizeof *http_msg4xx,
		sizeof http_msg5xx / sizeof *http_msg5xx
	};

	int type = code / 100;
	int idx = code % 100;

	if(type < 1 || type >= sizeof msgxxx / sizeof *msgxxx) {
		return "Invalid HTTP Status";
	}

	if(idx < 0 || idx >= msgcount[type]) {
		return "Unknown HTTP Status";
	}

	return msgxxx[type][idx];
}
