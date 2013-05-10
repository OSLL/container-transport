#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libmnl/libmnl.h>

#include "nlexample.h"

int main(void)
{
        char buf[getpagesize()];
        struct nlmsghdr *nlh;
        struct mnl_socket *nl;
        int ret;
        unsigned int seq, portid;

        nlh = mnl_nlmsg_put_header(buf);
        nlh->nlmsg_type = NLEX_MSG_UPD;
        nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
        nlh->nlmsg_seq = seq = time(NULL);
		nlh->nlmsg_pid = getpid();

        mnl_attr_put_u32(nlh, NLE_MYVAR, 31337);

        nl = mnl_socket_open(NETLINK_EXAMPLE);
        if (nl == NULL) {
                perror("mnl_socket_open");
                exit(EXIT_FAILURE);
        }
        if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
                perror("mnl_socket_bind");
                exit(EXIT_FAILURE);
        }
        portid = mnl_socket_get_portid(nl);

        if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
                perror("mnl_socket_sendto");
                exit(EXIT_FAILURE);
        }

        ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
        if (ret == -1) {
                perror("mnl_cb_callback");
                exit(EXIT_FAILURE);
        }
        mnl_socket_close(nl);

        return 0;
}
