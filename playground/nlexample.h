#ifndef _NLEXAMPLE_H_
#define _NLEXAMPLE_H_

#ifndef NETLINK_EXAMPLE
#define NETLINK_EXAMPLE 21
#endif

enum nlexample_msg_types {
   NLEX_MSG_BASE = NLMSG_MIN_TYPE,
   NLEX_MSG_UPD = NLEX_MSG_BASE,
   NLEX_MSG_GET,
   NLEX_MSG_MAX
};

enum nlexample_attr {
   NLE_UNSPEC,
   NLE_MYVAR,
   __NLE_MAX,
};
#define NLE_MAX (__NLE_MAX - 1)

#define NLEX_GRP_MYVAR 1

#endif
