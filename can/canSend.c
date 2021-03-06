#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "/home/pi/joy/lib.h"

void print_usage(char *prg)
{
	fprintf(stderr, "%s - send CAN-frames via CAN_RAW sockets.\n", prg);
	fprintf(stderr, "\nUsage: %s <device> <can_frame>.\n", prg);
	fprintf(stderr, "\n<can_frame>:\n");
	fprintf(stderr, " <can_id>#{data}          for 'classic' CAN 2.0 data frames\n");
	fprintf(stderr, " <can_id>#R{len}          for 'classic' CAN 2.0 data frames\n");
	fprintf(stderr, " <can_id>##<flags>{data}  for CAN FD frames\n\n");
	fprintf(stderr, "<can_id>:\n"
	        " 3 (SFF) or 8 (EFF) hex chars\n");
	fprintf(stderr, "{data}:\n"
	        " 0..8 (0..64 CAN FD) ASCII hex-values (optionally separated by '.')\n");
	fprintf(stderr, "{len}:\n"
		 " an optional 0..8 value as RTR frames can contain a valid dlc field\n");
	fprintf(stderr, "<flags>:\n"
	        " a single ASCII Hex value (0 .. F) which defines canfd_frame.flags\n\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  5A1#11.2233.44556677.88 / 123#DEADBEEF / 5AA# / 123##1 / 213##311223344 /\n"
		 "  1F334455#1122334455667788 / 123#R / 00000123#R3\n\n");
}



int canSend(char *port, char *data)
{
	int s; /* can raw socket */ 
	int required_mtu;
	int mtu;
	int enable_canfd = 1;
	struct sockaddr_can addr;
	struct canfd_frame frame;
	struct ifreq ifr;

	/* check command line options
	if (argc != 3) {
		print_usage(argv[0]);
		return 1;
	}
	*/
	/* parse CAN frame */
	required_mtu = parse_canframe(data, &frame);
	if (!required_mtu){
		fprintf(stderr, "\nWrong CAN-frame format!\n\n");
		print_usage("argv[0]");
		return 1;
	}

	/* open socket */
	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return 1;
	}

	strncpy(ifr.ifr_name, port, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
	if (!ifr.ifr_ifindex) {
		perror("if_nametoindex");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (required_mtu > CAN_MTU) {

		/* check if the frame fits into the CAN netdevice */
		if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
			perror("SIOCGIFMTU");
			return 1;
		}
		mtu = ifr.ifr_mtu;

		if (mtu != CANFD_MTU) {
			printf("CAN interface is not CAN FD capable - sorry.\n");
			return 1;
		}

		/* interface is ok - try to switch the socket into CAN FD mode */
		if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
			       &enable_canfd, sizeof(enable_canfd))){
			printf("error when enabling CAN FD support\n");
			return 1;
		}

		/* ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64 */
		frame.len = can_dlc2len(can_len2dlc(frame.len));
	}

	/* disable default receive filter on this RAW socket */
	/* This is obsolete as we do not read from the socket at all, but for */
	/* this reason we can remove the receive list in the Kernel to save a */
	/* little (really a very little!) CPU usage.                          */
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	/* send frame */
	if (write(s, &frame, required_mtu) != required_mtu) {
		perror("write");
		return 1;
	}

	close(s);

	return 0;
}