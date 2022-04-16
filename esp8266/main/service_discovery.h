#ifndef __SERVICE_DISCOVERY_H__
#define __SERVICE_DISCOVERY_H__

#include <list>
#include <string>

namespace service_discovery {

std::list<std::pair<std::string,int>> discover(const char* service, const char* proto, int max = -1, int timeout = 5000);

};

#endif // __SERVICE_DISCOVERY_H__