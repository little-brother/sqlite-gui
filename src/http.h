#ifndef __HTTP_H__
#define __HTTP_H__


namespace http {
	bool start(int port, sqlite3* db, bool debug = false);
	bool stop();
}

#endif
