#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080

// Helper function to read file contents
char* read_file(const char* filename, size_t* length) 
{
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(*length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *length, file);
    buffer[*length] = '\0'; // Null-terminate
    fclose(file);
    return buffer;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    // Set address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    // Server loop
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        // Read HTTP request
        char buffer[30000] = {0};
        valread = read(new_socket, buffer, 30000);
        printf("%s\n", buffer);

        // Check if it's a GET /index.html request
        if (strncmp(buffer, "GET /index.html", 15) == 0) {
            size_t file_length;
            char* file_content = read_file("index.html", &file_length);

            if (file_content) {
                char header[1024];
                snprintf(header, sizeof(header),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Content-Length: %zu\r\n"
                         "Connection: close\r\n"
                         "\r\n", file_length);

                write(new_socket, header, strlen(header));
                write(new_socket, file_content, file_length);
                free(file_content);
            } else {
                char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
                write(new_socket, not_found, strlen(not_found));
            }
        } else {
            // Fallback for other paths
            char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
            write(new_socket, not_found, strlen(not_found));
        }

        close(new_socket);
        printf("------------------Response sent-------------------\n");
    }

    return 0;
}





// #include <netinet/in.h>
// #include <string.h>
// #include <stdio.h>
// #include <sys/socket.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <arpa/inet.h>



// #define PORT 8080


// int main(int argc, char const *argv[])
// {
//     int server_fd, new_socket; long valread;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);
    
//     // Only this line has been changed. Everything is same.
//     char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    
//     // Creating socket file descriptor
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
//     {
//         perror("In socket");
//         exit(EXIT_FAILURE);
//     }
    

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons( PORT );
    
//     memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
//     {
//         perror("In bind");
//         exit(EXIT_FAILURE);
//     }
//     if (listen(server_fd, 10) < 0)
//     {
//         perror("In listen");
//         exit(EXIT_FAILURE);
//     }
//     while(1)
//     {
//         printf("\n+++++++ Waiting for new connection ++++++++\n\n");
//         if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
//         {
//             perror("In accept");
//             exit(EXIT_FAILURE);
//         }
        
//         char buffer[30000] = {0};
//         valread = read( new_socket , buffer, 30000);
//         printf("%s\n",buffer );
//         write(new_socket , hello , strlen(hello));
//         printf("------------------Hello message sent-------------------\n");
//         close(new_socket);
//     }
//     return 0;
// }

// THE commented CODE ABOVE WAS MADE FOR A CLIENT (That is not a web browser)











