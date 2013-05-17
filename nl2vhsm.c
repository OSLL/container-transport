#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <netlink/netlink.h>
#include <netlink/attr.h>

#include "nlvhsm.h"

static int cb(struct nl_msg *msg, void *arg)
{
		printf("rcv");
        return 0;
}


int main(void)
{
	struct nl_sock *sk;
	struct nl_msg *msg;

	/* Allocate a new socket */
	sk = nl_socket_alloc();

	/*
	* Notifications do not use sequence numbers, disable sequence number
	* checking.
	*/
	nl_socket_disable_seq_check(sk);
	/*
	 * Define a callback function, which will be called for each notification
	 * received
	 */
	nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, cb, NULL);
	
	/* Connect to routing netlink protocol */
	nl_connect(sk, NETLINK_VHSM);
	
		/* Allocate a default sized netlink message */
	if (!(msg = nlmsg_alloc_simple(NLVHSM_MSG_ROUTE, NLM_F_REQUEST | NLM_F_ACK)))
	{		perror("nlmsg_alloc_simple err");
			return 0;
	}
	NLA_PUT_U32(msg, NLVHSM_TO_VEID, 31337);
	
	nl_send_auto(sk,msg);
	
	
	while (1)
        nl_recvmsgs_default(sk);
	return 0;
}