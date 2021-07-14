#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    time_t now; //get time

    int sockfd, newsockfd, portno, logperfive, log = 0, logcount = 0; // sockfd,newsockfd,portn,clilen
    socklen_t clilen;                                                 // logperfive , log , logcount for passing record

    unsigned char sendbuf[512]; //the buffer
    unsigned char recvbuf[512];
    unsigned char buffer[512];

    struct sockaddr_in serv_addr, cli_addr; //build connection
    struct hostent *server;

    FILE *fp; //read file

    if (!strcmp(argv[1], "tcp")) //check tcp
    {
        if (!strcmp(argv[2], "send")) //check the send
        {
            int request = 0, byte_num = 0, file_len, byte = 0, n; // request to test have recieve or not
                                                                  // byte_num to count
            sockfd = socket(AF_INET, SOCK_STREAM, 0);             //do socket
            if (sockfd < 0)
                error("ERROR opening socket\n"); // check opening or not

            bzero((char *)&serv_addr, sizeof(serv_addr)); //set bind
            portno = atoi(argv[4]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(portno);
            if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR on binding\n"); // check port

            listen(sockfd, 5);

            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); //to accept
            if (newsockfd < 0)
                error("ERROR on accept\n"); //accept check

            bzero(buffer, 512);

            if (n < 0)
                error("ERROR reading from socket\n");

            file_len = strlen(argv[5]) + 1;               //count the length
            n = write(newsockfd, &file_len, sizeof(int)); //pass the length
            if (n < 0)
                error("ERROR writing to socket\n");
            n = write(newsockfd, argv[5], file_len * sizeof(char)); //pass the name
            if (n < 0)
                error("ERROR writing to socket\n");
            fp = fopen(argv[5], "rb"); //read file
            if (NULL == fp)
            {
                error("Error\n");
            }

            while (1) //read file per byte
            {
                fread(buffer, sizeof(unsigned char), 1, fp);
                if (feof(fp))
                    break;
                byte_num++;
            }

            logperfive = byte_num * 0.25; //count 25%

            n = write(newsockfd, &byte_num, sizeof(int)); //pass the size
            if (n < 0)
                error("ERROR writing to socket\n");

            rewind(fp);
            clock_t start_t, end_t; //count time
            start_t = clock();
            time(&now);
            printf("%d%% %s", log, ctime(&now));
            while (1)
            {
                bzero(buffer, 512);                            //initialize the buffer
                fread(buffer, sizeof(unsigned char), 512, fp); //read file by 512 byte
                n = write(newsockfd, buffer, 512);
                if (feof(fp))
                    break;
                if (n < 0)
                    error("ERROR writing to socket\n");

                logcount += 512; //check 25%

                if (logcount >= logperfive && log != 100)
                {
                    logcount = 0;                              //initialize the log count
                    time(&now);                                //get time
                    printf("%d%% %s", log += 25, ctime(&now)); // print 25%
                }
            }
            end_t = clock();
            printf("%d%% %s", log += 25, ctime(&now));
            printf("Total trans time : %dms\n", (int)((double)(end_t - start_t) / 1000)); //print the time
            printf("file size : %dMB\n", byte_num / 1000000);                             //print the size
            close(newsockfd);
            close(sockfd);
        }
        else if (!strcmp(argv[2], "recv")) //is recive
        {
            int request = 1, byte = 0, byte_num, file_len, loc, n; // byte_num to count byte

            char file_name[512], file_name_rev[512], ch[9] = "GET", ch2[50]; //storage the file name
            portno = atoi(argv[4]);
            sockfd = socket(AF_INET, SOCK_STREAM, 0); //do socket
            if (sockfd < 0)
                error("ERROR opening socket\n"); //check socket

            server = gethostbyname(argv[3]); //set port and ip
            if (server == NULL)
            {
                fprintf(stderr, "ERROR, no such host\n");
                exit(0);
            }

            bzero((char *)&serv_addr, sizeof(serv_addr)); //set connection
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(portno);
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR connecting\n"); //connect and check port

            bzero(buffer, 512); //initialize the buffer

            if (n < 0)
                error("ERROR writing to socket\n");

            n = read(sockfd, &file_len, sizeof(int)); //pass length
            if (n < 0)
                error("ERROR reading from socket\n");

            n = read(sockfd, file_name, file_len * sizeof(char)); //pass name
            if (n < 0)
                error("ERROR reading from socket\n");

            memset(file_name_rev, '\0', 512); //modify the file name
            loc = strchr(file_name, '.') - file_name;
            strncpy(file_name_rev, file_name, loc);
            strcat(file_name_rev, ch);
            memset(ch2, '\0', 50);
            memcpy(ch2, file_name + loc, strlen(file_name) - loc);
            strcat(file_name_rev, ch2);

            fp = fopen(file_name_rev, "wb"); //write file
            if (NULL == fp)
            {
                error("Error\n");
            }

            n = read(sockfd, &byte_num, sizeof(int)); //get size
            if (n < 0)
                error("ERROR reading from socket\n");

            int file_tail = 0;
            int flag = 0;
            //keep get the data
            while (byte <= byte_num)
            {

                n = read(sockfd, buffer, 512);
                if (n < 0)
                    error("ERROR reading from socket\n");

                for (int i = 0; i < 512; i++)
                {
                    if (!buffer[i])
                    {
                        fwrite(buffer, sizeof(unsigned char), i, fp);
                        flag = 1;
                        break;
                    }
                }
                if (flag)
                    break;

                fwrite(buffer, sizeof(unsigned char), 512, fp); //write into the file
                byte += 512;
            }
            printf("finish!\n");
            close(sockfd);
        }
    }
    else if (!strcmp(argv[1], "udp")) //is udp
    {
        if (!strcmp(argv[2], "send")) //is send
        {
            int request = 0, byte_num = 0, file_len, byte = 0; // byte_num to count byte
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                error("socket error"); //do socket

            bzero((char *)&serv_addr, sizeof(serv_addr)); //set bind
            portno = atoi(argv[4]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(portno);
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("bind error"); //bind and check

            clilen = sizeof(cli_addr);
            recvfrom(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&cli_addr, &clilen); //get request

            if (request)
            {
                file_len = strlen(argv[5]) + 1; //count length

                sendto(sockfd, &file_len, sizeof(int), 0, (struct sockaddr *)&cli_addr, clilen);           //pass length
                sendto(sockfd, argv[5], file_len * sizeof(char), 0, (struct sockaddr *)&cli_addr, clilen); //pass name

                fp = fopen(argv[5], "rb"); //readfile
                if (NULL == fp)
                {
                    error("Error\n");
                }

                while (1)
                {
                    fread(sendbuf, sizeof(unsigned char), 1, fp); //read per byte
                    if (feof(fp))
                        break;
                    byte_num++;
                }

                logperfive = byte_num * 0.25; //count 25%

                sendto(sockfd, &byte_num, sizeof(int), 0, (struct sockaddr *)&cli_addr, clilen); //pass size

                rewind(fp);

                clock_t start_t, end_t;
                start_t = clock();
                time(&now);
                printf("%d%% %s", log, ctime(&now));
                while (1)
                {
                    bzero(sendbuf, 512);
                    fread(sendbuf, sizeof(unsigned char), 511, fp); //read 511 byte
                    sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&cli_addr, clilen);

                    if (feof(fp))
                        break;

                    byte++; //count byte
                    logcount += 511;
                    if (logcount >= logperfive && log != 100) //count 25%
                    {
                        logcount = 0; //initialize count
                        time(&now);
                        printf("%d%% %s", log += 25, ctime(&now)); //print the status
                    }
                }
                printf("%d%% %s", log += 25, ctime(&now));
                end_t = clock();
                printf("Total trans time : %dms\n", (int)((double)(end_t - start_t) / 1000));
                printf("file size : %dMB\n", byte_num / 1000000);
            }
            else
            {
                printf("No client request file.\n");
            }
            close(sockfd);
        }
        else if (!strcmp(argv[2], "recv")) //recieve
        {
            int request = 1, byte = 0, byte_num, file_len, loc, ret, lose = 0;
            //lose to count lose packet

            struct timeval timeout = {0, 1};

            char file_name[512], file_name_rev[512], ch[9] = "GET", ch2[50];

            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) // do socket
                error("socket");

            bzero((char *)&serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]); //set udp
            server = gethostbyname(argv[3]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(portno);
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

            bzero(recvbuf, 512); //initialize the buffer

            sendto(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)); //sent the request
            recvfrom(sockfd, &file_len, sizeof(int), 0, NULL, NULL);                                    // pass length
            recvfrom(sockfd, file_name, file_len * sizeof(char), 0, NULL, NULL);                        // pass name

            memset(file_name_rev, '\0', 512); //rename the file
            loc = strchr(file_name, '.') - file_name;
            strncpy(file_name_rev, file_name, loc);
            strcat(file_name_rev, ch);
            memset(ch2, '\0', 50);
            memcpy(ch2, file_name + loc, strlen(file_name) - loc);
            strcat(file_name_rev, ch2);

            fp = fopen(file_name_rev, "wb"); //write to file
            if (NULL == fp)
            {
                error("Error\n");
            }

            recvfrom(sockfd, &byte_num, sizeof(int), 0, NULL, NULL); //get size

            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); //check time

            while (byte <= byte_num) //utile recive end
            {
                bzero(recvbuf, 512);
                ret = recvfrom(sockfd, recvbuf, sizeof(recvbuf) - 1, 0, NULL, NULL); //keep get
                if (ret == -1)                                                       //check lose
                {
                    lose++; //count lose packet
                    if (lose > 100)
                        break;
                }
                else
                {
                    fwrite(recvbuf, sizeof(unsigned char), strlen(recvbuf), fp); //write to file
                    byte += strlen(recvbuf);                                     //check byte
                }
            }

            if (!lose)
                printf("packet loss rate : 0%%\n"); //no lose anything
            else
                printf("packet loss rate : %lf%%\n", (double)(byte_num - byte) / (double)byte_num * 100); //print lose rate
            printf("finish!\n");
            close(sockfd);
        }
    }
    return 0;
}
