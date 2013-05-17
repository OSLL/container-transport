#ifndef _NLVHSM_H_
#define _NLVHSM_H_

#ifndef NETLINK_VHSM
#define NETLINK_VHSM 21
#endif

 enum nlvhsm_msg_types {
    NLVHSM_MSG_BASE = NLMSG_MIN_TYPE,
    NLVHSM_MSG_ROUTE = NLVHSM_MSG_BASE,
    NLVHSM_MSG_MAX
 };

 enum nlvhsm_attr {
    NLVHSM_TO_VEID = 1,
    NLVHSM_SIZE,
    NLVHSM_DATA,
    __NLVHSM_MAX,
 };
 #define NLVHSM_MAX (__NLVHSM_MAX)

 #define NLVHSM_ROUTE_TO 1

#endif