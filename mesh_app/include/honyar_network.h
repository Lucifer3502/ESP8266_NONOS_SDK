
#ifndef _HONYAR_NETWORK_H_
#define _HONYAR_NETWORK_H_

#include "c_types.h"

#define NET_IP_ADDR_LEN  16
#define NET_TCP_SERVER_CLI_NUM  5
#define NET_NETWORK_WRITE_LEN  1024

typedef enum {
    NET_TCP_CLIENT_T,
    NET_TCP_SERVER_T,
    NET_UDP_T,
}network_type_t;




#endif

