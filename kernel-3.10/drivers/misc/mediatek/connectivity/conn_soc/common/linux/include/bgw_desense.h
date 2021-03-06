
#ifndef __BGW_DESENSE_H_
#define __BGW_DESENSE_H_

#ifdef MSG
#undef MSG
#endif

#ifdef ERR
#undef ERR
#endif

#define MSG(fmt, arg ...) printk(KERN_INFO "[BGW] %s: " fmt, __FUNCTION__ ,##arg)
#define ERR(fmt, arg ...) printk(KERN_ERR "[BGW] %s: " fmt, __FUNCTION__ ,##arg)

#ifdef NETLINK_TEST
#undef NETLINK_TEST
#endif

#define NETLINK_TEST 17

#ifdef MAX_NL_MSG_LEN
#undef MAX_NL_MSG_LEN
#endif

#define MAX_NL_MSG_LEN 1024


#ifdef ON
#undef ON
#endif
#ifdef OFF
#undef OFF
#endif
#ifdef ACK
#undef ACK
#endif

#define ON 1
#define OFF 0
#define ACK 2

extern void send_command_to_daemon(const int command);

extern int bgw_init_socket(void);

extern void bgw_destory_netlink_kernel(void);

#endif
