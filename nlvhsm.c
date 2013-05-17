#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <libmnl/libmnl.h>
#include <pthread.h>

#include "nlvhsm.h"

struct mnl_socket *nl;
pthread_mutex_t nl_mutex;


static int data_cb(const struct nlmsghdr *nlh, void *data)
{
	mnl_nlmsg_fprintf(stdout,nlh, nlh->nlmsg_len,0);
	return MNL_CB_OK;
}

void *listener(void *arg)
{
     int ret;
     char *buf = (char*)malloc(MNL_SOCKET_BUFFER_SIZE);
     unsigned int portid = mnl_socket_get_portid(nl);



    while (1) {
         ret = mnl_socket_recvfrom(nl, buf, MNL_SOCKET_BUFFER_SIZE);
         if (ret == -1) {
             perror("mnl_socket_recvfrom");
             exit(EXIT_FAILURE);
         }

         ret = mnl_cb_run(buf, ret, 0, 0, data_cb, NULL);
         if (ret == -1) {
             perror("mnl_cb_run");
             exit(EXIT_FAILURE);
         }
     }

}

int send_msg(int to_veid, size_t size, void * data)
{
	struct nlmsghdr *nlh;
	
	char *buf = (char*)malloc(MNL_SOCKET_BUFFER_SIZE);
	
	nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = NLVHSM_MSG_ROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = time(NULL);
	mnl_attr_put_u32(nlh, NLVHSM_TO_VEID, to_veid);
	//mnl_attr_put(nlh,NLVHSM_DATA,size,data);
	
	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	free(buf);
}


int main(void)
{
	char str [80];
	int i;
	pthread_t thread;
	
	nl = mnl_socket_open(NETLINK_VHSM);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	
	pthread_mutex_init(&nl_mutex, NULL);
	
	pthread_create(&thread,NULL,&listener,NULL);

	printf("Enter veid and message(veid==-1 => exit):");
	scanf ("%d %s",&i,str);
	while (i>=0)
	{
		send_msg(i,strlen(str),str);
		
		printf("Enter veid and message(veid==-1 => exit):");
		scanf ("%d %s",&i,str);
	}
  	mnl_socket_close(nl);

}