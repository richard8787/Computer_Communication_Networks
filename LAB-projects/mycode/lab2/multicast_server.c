/* Send Multicast Datagram code example. */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char databuf[20480] = "";
int datalen = sizeof(databuf);
int n;
FILE *fp;

int main(int argc, char *argv[])
{
	/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else
		printf("Opening the datagram socket...OK.\n");

	/* Initialize the group sockaddr structure with a */
	/* group address of 225.1.1.1 and port 5555. */
	memset((char *)&groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(4321);

	/* Disable loopback so you do not receive your own datagrams.
	{
	char loopch = 0;
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
	{
	perror("Setting IP_MULTICAST_LOOP error");
	close(sd);
	exit(1);
	}
	else
	printf("Disabling the loopback...OK.\n");
	}
	*/

	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr("127.0.0.1");
	if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
		perror("Setting local interface error");
		exit(1);
	}
	else
		printf("Setting the local interface...OK\n");
	/* Send a message to the multicast group specified by the*/
	/* groupSock sockaddr structure. */
	/*int datalen = 1024;*/
	//if (argc == 2)
	fp = fopen(argv[1], "rb"); //read file
	struct stat buf;
	stat(argv[1], &buf);

	while (1)
	{
		bzero(databuf, 20480);							  //initialize the buffer
		fread(databuf, sizeof(unsigned char), 20480, fp); //read file by 512 byte
		//printf("%s", databuf);
		n = sendto(sd, databuf, 20480, 0, (struct sockaddr *)&groupSock, sizeof(groupSock));
		if (feof(fp))
			break;
		if (n < 0)
		{
			perror("Sending datagram message error");
		}
	}
	printf("Sending datagram message...OK\n");
	sendto(sd, "finish", 6, 0, (struct sockaddr *)&groupSock, sizeof(groupSock));
	/* Try the re-read from the socket if the loopback is not disable
	if(read(sd, databuf, datalen) < 0)
	{
	perror("Reading datagram message error\n");
	close(sd);
	exit(1);
	}
	else
	{
	printf("Reading datagram message from client...OK\n");
	printf("The message is: %s\n", databuf);
	}
	*/
	printf("file size: %.3fKB\n", buf.st_size / 1000.0);
	return 0;
}