/* networking-only header */
#ifdef NETWORKING
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#endif

#include "includes.h"
#include "game/update.h"
#include "gui/gui.h"
#include "network/net_sprite.h"
#include "network/network.h"
#include "sprite/sprite.h"

#define SERVER_PORT 1500
#define SOCK_ERROR -1
#define BUF_SIZE 1024

int bufsize = BUF_SIZE;

struct {
	short int last_angle_sent;
} net_info;

#ifdef NETWORKING
struct hostent *h = NULL;
struct sockaddr_in localAddr, servAddr;
socklen_t serv_len = sizeof(servAddr);
int tcp_socket, udp_socket, rc;
#endif
unsigned char networking_on = 0;

char callsign[] = "Epiar Client";

int authenticate(int socket);
void request_ship_info(int about_who);

void network_to(char *address) {
#ifdef NETWORKING
	int dontblock = 1;
	int rc; /* return value for setting nonblocking */
	u_long server_ip;
	socklen_t addrLen = sizeof(struct sockaddr_in);

	server_ip = inet_addr(address);
	if ((signed)server_ip == SOCK_ERROR) {
		struct hostent *server_host = gethostbyname(address);

		if (server_host == NULL) {
			printf("unknown host \"%s\"\n", address);
			return;
		}
		memcpy((char *)&server_ip, server_host->h_addr_list[0], server_host->h_length);
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = server_ip;
	servAddr.sin_port = htons(SERVER_PORT);

	/* create socket */
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("cannot open tcp socket ");
		exit(1);
	}

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) {
		perror("cannot open udp socket");
		exit(1);
	}

	/* bind any port number */
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(0);

	rc = bind(tcp_socket, (struct sockaddr *) &localAddr, addrLen);
	if (rc < 0) {
		printf("cannot bind port TCP %u\n", SERVER_PORT);
		perror("error ");
		exit(1);
	}

	rc = getsockname(tcp_socket, (struct sockaddr *) &localAddr, &addrLen);
	if (rc < 0) {
		perror("cannot get TCP port ");
		exit(1);
	}

	rc = bind(udp_socket, (struct sockaddr *) &localAddr, addrLen);
	if (rc < 0) {
		printf("cannot bind port UDP %u\n", SERVER_PORT);
		perror("error ");
		exit(1);
	}

	/* connect to server */
	rc = connect(tcp_socket, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if (rc < 0) {
		perror("cannot connect ");
		exit(1);
	}

	rc = connect(udp_socket, (struct sockaddr *) &servAddr, sizeof(servAddr));
	if (rc < 0) {
		perror("cannot connect ");
		exit(1);
	}

	rc = ioctl(tcp_socket, FIONBIO, &dontblock);
	rc = ioctl(udp_socket, FIONBIO, &dontblock);

	/* authenticate client */
	//if (authenticate(tcp_socket) != 0)
		//return;

	/* set some info. */
	net_info.last_angle_sent = 0;

	init_net_ships();

	networking_on = 1;
#endif
}

void send_status(void) {
#ifdef NETWORKING
	char buffer[BUF_SIZE] = {0};
	static Uint32 last_time = 0;
	int rc;
	struct sockaddr_in remote_addr;
	socklen_t remote_len = sizeof(remote_addr);

	if (current_time > (last_time + 35)) {
		last_time = current_time;

		if (networking_on) {
			int pos = 0;
			char msg[BUF_SIZE] = {0};
			int n;
			union {
				int ix;
				char cx[4];
			} x;
			union {
				int iy;
				char cy[4];
			} y;

			/* sign the beginning as a standard ship update packet */
			buffer[pos] = 'E';
			pos += 1;
			buffer[pos] = 'N';
			pos += 1;
			/* send the x and y coords */
			x.ix = player.ship->world_x;
			y.iy = player.ship->world_y;
			buffer[pos] = x.cx[0]; /* set the x coord */
			pos += 1;
			buffer[pos] = x.cx[1];
			pos += 1;
			buffer[pos] = x.cx[2];
			pos += 1;
			buffer[pos] = x.cx[3];
			pos += 1;
			buffer[pos] = y.cy[0]; /* set the y coord */
			pos += 1;
			buffer[pos] = y.cy[1];
			pos += 1;
			buffer[pos] = y.cy[2];
			pos += 1;
			buffer[pos] = y.cy[3];
			pos += 1;
			/* see if we need to update the angle */
			if (net_info.last_angle_sent != player.ship->angle) {
				union {
					short int ia;
					char ca[2];
				} angle;
				buffer[pos] = 1; /* angle indicator */
				pos += 1;
				angle.ia = player.ship->angle;
				buffer[pos] = angle.ca[0];
				pos += 1;
				buffer[pos] = angle.ca[1];
				pos += 1;
				net_info.last_angle_sent = player.ship->angle;
			}

			/* send the server some data */
			/* dont need to say pos + 1 'cause pos is already one over */
			rc = sendto(udp_socket, buffer, pos, 0, 0, 0);
			printf("sending returned %d\n", rc);
			if ((rc == -1) && (errno != EWOULDBLOCK)) {
				perror("cannot send data ");
				close(udp_socket);
				close(tcp_socket);
				gui_alert("Lost connection to server.");
				networking_on = 0;
				//exit(1);
			}

			/* receieve some data */
			n = recvfrom(udp_socket, msg, BUF_SIZE, 0, (struct sockaddr *)&remote_addr, &remote_len); /* wait for data MSG_DONTWAIT commented out */

			if (memcmp(&servAddr, &remote_addr, sizeof(struct sockaddr_in)) != 0) {
				printf("received udp data but it wasnt from server\n");
			}

			printf("servAddr.sin_port           = %d\n", servAddr.sin_port);
			printf("servAddr.sin_addr.s_addr    = %d\n", servAddr.sin_addr.s_addr);
			printf("servAddr.sin_family         = %d\n", servAddr.sin_family);
			printf("remote_addr.sin_port        = %d\n", remote_addr.sin_port);
			printf("remote_addr.sin_addr.s_addr = %d\n", remote_addr.sin_addr.s_addr);
			printf("remote_addr.sin_family      = %d\n", remote_addr.sin_family);

			if (n == -1) {
				/* this is okay if it's just a SOCEWOULDBLOCK message */
				if (errno != EWOULDBLOCK) {
					perror("data transmission error");
				}
			} else if (n == 0) {
				printf("connection terminated\n");
				close(udp_socket);
				close(tcp_socket);
				gui_alert("Lost connection to server.");
				networking_on = 0;
			} else {
				/* interpert the package */
				union {
					int ix;
					char cx[4];
				} x;
				union {
					int iy;
					char cy[4];
				} y;
				union {
					short int ia;
					char ca[2];
				} angle;
				union {
					int iw;
					char cw[4];
				} who;

				/* only interpert package if it is signed correctly */
				if ((msg[0] == 'E') && (msg[1] == 'N')) {
					/* find out who it is */
					who.cw[0] = msg[2];
					who.cw[1] = msg[3];
					who.cw[2] = msg[4];
					who.cw[3] = msg[5];

					/* if we know who this is, set their information. if we dont, ask server who it is and ignore info. until server tells us */
					if (is_ship_known(who.iw) != 0) {
						request_ship_info(who.iw);
					} else {
						/* get the x */
						x.cx[0] = msg[6];
						x.cx[1] = msg[7];
						x.cx[2] = msg[8];
						x.cx[3] = msg[9];

						net_ships[who.iw].x = x.ix;

						/* get the y */
						y.cy[0] = msg[10];
						y.cy[1] = msg[11];
						y.cy[2] = msg[12];
						y.cy[3] = msg[13];

						net_ships[who.iw].y = y.iy;

						/* get the angle */
						angle.ca[0] = msg[14];
						angle.ca[1] = msg[15];

						net_ships[who.iw].angle = angle.ia;

						net_ships[who.iw].last_update = current_time;
					}
				} else if ((msg[0] == 'S') && (msg[1] == 'I')) {
					union {
						int ia;
						char ca[4];
					} who;
					union {
						short int it;
						char ct[2];
					} ship_type;
					union {
						int ix;
						char cx[4];
					} x;
					union {
						int iy;
						char cy[4];
					} y;
					union {
						short int ia;
						char ca[2];
					} angle;
					char callsign[20] = {0};
					short int callsign_len;
					int i;

					who.ca[0] = msg[2];
					who.ca[1] = msg[3];
					who.ca[2] = msg[4];
					who.ca[3] = msg[5];
					printf("Received info. about ship #%d\n", who.ia);
					ship_type.ct[0] = msg[6];
					ship_type.ct[1] = msg[7];
					printf("        Ship type: %d\n", ship_type.it);
					x.cx[0] = msg[8];
					x.cx[1] = msg[9];
					x.cx[2] = msg[10];
					x.cx[3] = msg[11];
					y.cy[0] = msg[12];
					y.cy[1] = msg[13];
					y.cy[2] = msg[14];
					y.cy[3] = msg[15];
					printf("        Coordinates: (%d, %d)\n", x.ix, y.iy);
					/* pass x,y */
					angle.ca[0] = msg[16];
					angle.ca[1] = msg[17];
					printf("        Angle: %d\n", angle.ia);
					callsign_len = (short int)msg[18];
					for (i = 0; i < callsign_len; i++) {
						callsign[i] = msg[19 + i];
					}
					callsign[i] = 0;
					printf("        Callsign: \"%s\"\n", callsign);
					new_net_ship(who.ia, ship_type.it, x.ix, y.iy, angle.ia, callsign);
				} else {
					printf("bad packet recived: \"%s\"\n", msg);
				}
			}
		}
	}
#endif
}

int is_networking_enabled(void) {
	return (networking_on);
}

/* sends needed info to server. returns zero on success */
int authenticate(int socket) {
#ifdef NETWORKING
	char msg[80] = {0};
	unsigned char done = 0;
	union {
		short int iv;
		char cv[2];
	} version;

	strcpy(msg, "Epiar");
	/* send the major version */
	version.iv = 0;
	msg[5] = version.cv[0];
	msg[6] = version.cv[1];
	/* send the minor version */
	version.iv = 3;
	msg[7] = version.cv[0];
	msg[8] = version.cv[1];
	/* send the micro version */
	version.iv = 0;
	msg[9] = version.cv[0];
	msg[10] = version.cv[1];

	/* send the length of the callsign, then the callsign itself (reuse version since it isnt needed) */
	version.iv = strlen(callsign);
	msg[11] = version.cv[0];
	msg[12] = version.cv[1];
	/* now send the callsign */
	strncpy(&msg[13], callsign, strlen(callsign));

	/* more authentication needed here later */

	if (send(socket, msg, 12 + strlen(callsign) + 1, 0) < 0) {
		printf("Could not authenticate.\n");
		return (-1);
	}

	printf("Authentication information sent. Awaiting server response ...\n");

	memset(msg, 0, sizeof(char) * 80);

	while(!done) {
		int n;

		n = recv(socket, msg, 80, 0); /* wait for data MSG_DONTWAIT commented out */

		if (n == 0) {
			printf("Connect to server lost.\n");
			return (-1);
		} else if (n != -1) {
			printf("Server replied ... ");
			if ((msg[0] == 'O') && (msg[1] == 'K')) {
				printf("OK\n");
				return (0);
			} else {
				printf("failed.\n");
				return (-1);
			}
		}
	}
#endif
	return (-1);
}

/* sends a request to the server about finding out who the ship is */
void request_ship_info(int about_who) {
#ifdef NETWORKING
	char buffer[40] = {0};
	int pos = 0;
	union {
		int iw;
		char cw[4];
	} who;

	who.iw = about_who;

	/* sign as ship info request packet */
	buffer[pos] = 'S';
	pos += 1;
	buffer[pos] = 'I';
	pos += 1;
	buffer[pos] = who.cw[0];
	pos += 1;
	buffer[pos] = who.cw[1];
	pos += 1;
	buffer[pos] = who.cw[2];
	pos += 1;
	buffer[pos] = who.cw[3];
	pos += 1;

	/* send the request */
	/* dont need to say pos + 1 'cause pos is already one over */
	if (sendto(udp_socket, buffer, pos, 0, (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in)) < 0)
		printf("warning, request for ship info not sent successfully\n");
#endif
}

/* makes request to server to fire */
void request_fire(int x, int y, int weapon) {

}
