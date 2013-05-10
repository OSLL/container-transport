#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ve.h>
#include <linux/netlink.h>
#include <linux/flex_array.h>


#ifndef NETLINK_EXAMPLE
#define NETLINK_EXAMPLE 21
#endif
#define NLEX_GRP_MAX    0

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

static struct sock *nlsk[3];

struct flex_array *net_ns;


static const struct nla_policy nle_info_policy[NLE_MAX+1] = {
    [NLE_MYVAR] = { .type = NLA_U32 },
};

static int
nl_rcv_skb(struct sk_buff *skb,
        struct nlmsghdr *nlh)
{
   int err;
   struct nlattr *cda[NLE_MAX+1];
   struct nlattr *attr = NLMSG_DATA(nlh);
   int attrlen = nlh->nlmsg_len - NLMSG_SPACE(0);
   struct net *net=skb->sk->sk_net;

   if (nlh->nlmsg_len < NLMSG_SPACE(0))
      return -EINVAL;

   err = nla_parse(cda, NLE_MAX,
           attr, attrlen, nle_info_policy);
   if (err < 0)
      return err;

    printk(KERN_INFO "Netlink received msg payload: %d\n",nla_get_u32(cda[NLE_MYVAR]));
    printk(KERN_INFO " From ve id %u\n",net->owner_ve->veid);
    return 0;
}

static void
nl_callback(struct sk_buff *skb)
{
    netlink_rcv_skb(skb, &nl_rcv_skb);
}
static int __net_init net_init(struct net *net)
{
    envid_t id=net->owner_ve->veid;

   nlsk[id] = netlink_kernel_create(net, NETLINK_EXAMPLE,NLEX_GRP_MAX,nl_callback, NULL, THIS_MODULE);
   printk(KERN_INFO " ve id %u\n",net->owner_ve->veid);
    if (!nlsk[id])
    {
        printk(KERN_INFO "Create error");
        return -ENOMEM;
    }
    return 0;
}
static void __net_exit net_exit(struct net *net)
{
  envid_t id=net->owner_ve->veid;
  netlink_kernel_release(nlsk[id]);

}
static struct pernet_operations net_ops = {
   .init = net_init,
   .exit = net_exit,
};
static int __init nlexample_init(void)
{
   register_pernet_subsys(&net_ops);
  return 0;
}

void __exit nlexample_exit(void)
{
    unregister_pernet_subsys(&net_ops);
}

module_init(nlexample_init);
module_exit(nlexample_exit);

MODULE_AUTHOR("Ilya Averyanov <averyanovin@gmail.com>");
MODULE_LICENSE("GPL");