#include "service_discovery.h"
#include "mdns.h"

namespace service_discovery {

std::list<std::pair<std::string,int>> discover(const char* service, const char* proto, int max, int timeout)
{
    mdns_result_t* results = NULL;
    ESP_ERROR_CHECK(mdns_query_ptr(service, proto, timeout, 1,  &results));
    auto r = results;
    std::list<std::pair<std::string,int>> rst;
    while (r && (max < 0 || rst.size() < max)) {
        auto a = r->addr;
        while (a) {
            if (a->addr.type == IPADDR_TYPE_V6 || a->addr.type == IPADDR_TYPE_V4) {
                rst.push_back(std::make_pair(ipaddr_ntoa(&a->addr), r->port));
            }
            a = a->next;
        }
        r = r -> next;
    }

    mdns_query_results_free(results);
    return rst;
}

};
