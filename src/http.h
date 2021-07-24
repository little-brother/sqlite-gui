#ifndef __HTTP_H__
#define __HTTP_H__

#include "sqlite3.h"

namespace http {
	bool start(int port, sqlite3* db, bool debug = false);
	bool stop();
}

#endif
