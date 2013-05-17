#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/ve.h>
#include <linux/netlink.h>

#include "nlvhsm.h"

LIST_HEAD(vhsm_sokets);

struct nlvhsm_socket 
{
	struct list_head  list;
	envid_t veid;
	struct sock *socket;
};

static const struct nla_policy nlvhsm_route_policy[NLVHSM_MAX] = {
	[NLVHSM_TO_VEID] = { .type = NLA_U32 },
};

void free_soket(envid_t veid)
{
	struct nlvhsm_socket *s,*tmp;
	
	list_for_each_entry_safe(s, tmp, &vhsm_sokets, list){
		if (s->veid == veid)
		{
			netlink_kernel_release(s->socket);
			list_del(&s->list);
			kfree(s);
			return;
		}

	}
}

static struct sock * get_soket(envid_t veid)
{
	struct nlvhsm_socket *s;
	list_for_each_entry(s, &vhsm_sokets, list) {
		if (s->veid == veid)
			return s->socket;
	}
	return NULL;
}
static int nlvhsm_send(int pid, int id,envid_t veid)
{
   struct sk_buff *skb;
   struct nlmsghdr *nlh;

   skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
   if (skb == NULL)
      return -ENOMEM;

   nlh = nlmsg_put(skb, pid, 0, NLVHSM_ROUTE_TO, 0, 0);
   if (nlh == NULL)
      goto nlmsg_failure;

   NLA_PUT_U32(skb, NLVHSM_TO_VEID, id);
   NLA_PUT_U32(skb, NLVHSM_SIZE, 31337);
   nlmsg_end(skb, nlh);
   
   print_hex_dump(KERN_DEBUG, "raw data: ", DUMP_PREFIX_ADDRESS,16, 1,skb->data  ,skb->len, true);
   
   return nlmsg_unicast(get_soket(veid), skb, pid);
nlmsg_failure:
nla_put_failure:
   nlmsg_cancel(skb, nlh);
   kfree_skb(skb);
   return -1;
}
static int nl_rcv_skb(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	int err;
	struct nlattr *cda[NLVHSM_MAX];
	struct nlattr *attr = NLMSG_DATA(nlh);
	int attrlen = nlh->nlmsg_len - NLMSG_SPACE(0);
	struct net *net=skb->sk->sk_net;

	if (nlh->nlmsg_len < NLMSG_SPACE(0))
		return -EINVAL;

	err = nla_parse(cda, NLVHSM_MAX - 1, attr, attrlen,  nlvhsm_route_policy);
	if (err < 0)
		return err;
	print_hex_dump(KERN_DEBUG, "raw data: ", DUMP_PREFIX_ADDRESS,16, 1,  NLMSG_DATA(nlh), attrlen, true);
	printk(KERN_INFO "Netlink received msg payload: %d\n",nla_get_u32(cda[NLVHSM_TO_VEID]));
	printk(KERN_INFO " From ve id %u\n",net->owner_ve->veid);
	printk(KERN_INFO "Send...\n");
	nlvhsm_send(nlh->nlmsg_pid,nla_get_u32(cda[NLVHSM_TO_VEID]),net->owner_ve->veid);
	printk(KERN_INFO "Send done\n");
	
	return 0;
}

static void nl_callback(struct sk_buff *skb)
{
	netlink_rcv_skb(skb, &nl_rcv_skb);
}
static int __net_init net_init(struct net *net)
{
	struct sock *s;
	struct nlvhsm_socket* socket_list;
	
	printk(KERN_INFO " ve id %u ",net->owner_ve->veid);
	
	s = netlink_kernel_create(net, NETLINK_VHSM, NLVHSM_ROUTE_TO, nl_callback, NULL, THIS_MODULE);
	printk(KERN_INFO " sock addr %p\n",s);
	if(s)
	{
		socket_list = kmalloc(sizeof(*socket_list), GFP_KERNEL);
		socket_list->veid = net->owner_ve->veid;
		socket_list->socket = s;
		INIT_LIST_HEAD(&socket_list->list);
		list_add(&socket_list->list, &vhsm_sokets);
	}
	else
	{
		printk(KERN_INFO "Create error");
		return -ENOMEM;
	}
	return 0;
}
static void __net_exit net_exit(struct net *net)
{
	free_soket(net->owner_ve->veid);
}

static struct pernet_operations net_ops = {
	.init = net_init,
	.exit = net_exit,
};
static int __init nlvhsm_init(void)
{
	register_pernet_subsys(&net_ops);
	return 0;
}

void __exit nlvhsm_exit(void)
{
	unregister_pernet_subsys(&net_ops);
}

module_init(nlvhsm_init);
module_exit(nlvhsm_exit);

MODULE_AUTHOR("Ilya Averyanov <averyanovin@gmail.com>");
MODULE_LICENSE("GPL");