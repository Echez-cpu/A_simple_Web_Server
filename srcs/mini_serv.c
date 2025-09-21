#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h> //important

int fixed_fd[65536]; // change to id
char *msg[65536];



char send_buff[1024], recv_buff[1024];

int highest_fd = 0, global_fd = 0;

fd_set write_me, read_me, master_set;

void error_msg(char *str)
{
    if (str)
    {
        write(2, str, strlen(str));
    }

    else
    {
        write(2, "Fatal error", 11);
    }

    write(2, "\n", 1);

    exit(1);
}

void notify_all(int except, char *str)
{
     for (int fd = 0; fd <= highest_fd; fd++)
     {
         if(FD_ISSET(fd, &write_me) && fd != except)
         {
            if(send(fd, str, strlen(str), 0) == -1)
            {
                error_msg(NULL);
            }
               
         }
     }
}


char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}


void	send_msg(int fd)
{
	char *msgs;

	while (extract_message(&(msg[fd]), &msgs))
	{
		sprintf(send_buff, "client %d: ", fixed_fd[fd]);
		notify_all(fd, send_buff);
		notify_all(fd, msgs);
		free(msgs);
	}
}



int main (int argc, char **argv)
{
    
    if (argc != 2)
    {
        error_msg("Wrong number of arguments");
    }
    
    
    struct sockaddr_in server_add, cli;

    socklen_t  len;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1)
    {
        error_msg(NULL);
    }

    highest_fd = sock_fd;

     FD_ZERO(&master_set);
     FD_SET(sock_fd, &master_set);
     bzero(&server_add, sizeof(server_add));
     bzero(&cli, sizeof(cli));

     // assign IP, PORT 
	server_add.sin_family = AF_INET; 
	server_add.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	server_add.sin_port = htons(atoi(argv[1])); 


    if (bind(sock_fd, (const struct sockaddr *)&server_add, sizeof(server_add)) == -1 || listen(sock_fd, 10) == -1)
    {
        error_msg(NULL);
    }



    while(1)
    {
        write_me = read_me = master_set;

        if (select(highest_fd + 1, &read_me, &write_me, NULL, NULL) == -1)
        {
            error_msg(NULL);
        }

        for (int fd = 0; fd <= highest_fd; fd++)
        {
            if(!FD_ISSET(fd, &read_me))
            {
                continue;
            }

            if (sock_fd == fd)
            {
                len = sizeof(cli);
               int new_client = accept(sock_fd, (struct sockaddr *)&cli, &len);
               if (new_client == -1) continue;
               if(new_client > highest_fd) highest_fd = new_client;
                fixed_fd[new_client] = global_fd++;
                msg[new_client] = NULL;  // be careful
                FD_SET(new_client, &master_set);
                sprintf(send_buff, "server: client %d just arrived\n", fixed_fd[new_client]);
                notify_all(new_client, send_buff);
                break;

            }

            else
            {
                int bytes_recv = recv(fd, recv_buff, 1000, 0);

                if(bytes_recv <= 0)
                {
                    sprintf(send_buff, "server: client %d just left\n", fixed_fd[fd]);
                    notify_all(fd, send_buff);
                    FD_CLR(fd, &master_set);
                    free(msg[fd]);
                    close(fd);
                    break;

                }

                recv_buff[bytes_recv] = '\0';

                msg[fd] = str_join(msg[fd], recv_buff);

                send_msg(fd);
            }

        }
    }
	return(0);
}
