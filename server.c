/**
 * Skeleton file for server.c
 * 
 * You are free to modify this file to implement the server specifications
 * as detailed in Assignment 3 handout.
 * 
 * As a matter of good programming habit, you should break up your imple-
 * mentation into functions. All these functions should contained in this
 * file as you are only allowed to submit this file.
 */ 

#include <stdio.h>
// Include necessary header files
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <stdbool.h>
#include <ctype.h>
/**
 * The main function should be able to accept a command-line argument
 * argv[0]: program name
 * argv[1]: port number
 * 
 * Read the assignment handout for more details about the server program
 * design specifications.
 */ 

/**
 *Prints error message and exits
 * @param msg String message sent to server
 */
void error(const char * msg){
    printf("Error: %s\n", msg);
    exit(1);
}

/**
 * This creates the socket
 */ 
int create_socket(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){error("Error creating socket");}
    printf("Socket created\n");
    return fd;
}

/**
 * This creates the server address
 * @param argc The number of arguments in command line
 * @param serveraddr The structure which holds the information of the servers address
 * @param PORT_NUM The port number
 */
void create_server_addr(int argc, struct sockaddr_in *serveraddr, int PORT_NUM){
    if(argc == 2){
        serveraddr -> sin_family = AF_INET;
        serveraddr -> sin_port = htons(PORT_NUM);
        serveraddr -> sin_addr.s_addr = INADDR_ANY;
    }else{
        error("Need and Address");
    }
    printf("Address created\n");
}

/**
 * Binding the socket to an address
 * @param fd The socket
 * @param serveraddr The structure which holds the information of the servers address
 */ 
void binding(int fd, struct sockaddr_in serveraddr){
    int bd;
    bd = bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(bd < 0){error("Error Binding");}
    printf("Bind successful\n");
}

/**
 * Listens for connections
 * @param fd The socket
 */ 
void listening(int fd){
    int l = listen(fd,SOMAXCONN);
    if(l < 0){error("Listening Error");}
    printf("Listen successful\n");
}

/**
 * Accepts connections
 * @param fd The socket
 */
int accept_connection(int fd){
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);
    int c_fd = accept(fd, (struct sockaddr *)&clientaddr, (socklen_t*)&addrlen);
    if(c_fd < 0){error("Error Accepting Client");}
    printf("Accept successful\n");
    return c_fd;
}

/**
 * Writes to the client
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client
 */
void writes(int client_fd, char buffer[]){
    int s = write(client_fd, buffer,300);
    if(s<0){error("Error writing to socket");}
    printf("Write success\n");
    memset(buffer,0,300);
}

/**
 * Gets the file name to be opened from the buffer
 * reads through the file 
 * sends contents of file to the client line by line
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client
 */ 
void get_file_name(int client_fd, char buffer[]){
    int ctr = 0;
    char file[100];
    for(int i = 4; i<strlen(buffer)-1; ++i){
        file[ctr++] = buffer[i];
    }
    FILE *fp1;
    fp1 = fopen(file,"r");
    if(fp1 == NULL){
        memset(buffer,0,300);
        strncpy(buffer, "SERVER 404 Not Found\n", strlen("SERVER 404 Not Found\n"));
        writes(client_fd, buffer);
    }else{
        char temp[1028];
        strncpy(buffer, "SERVER 200 OK\n\n", strlen("SERVER 200 OK\n\n"));
        writes(client_fd, buffer);
        while(fgets(temp, sizeof(temp), fp1) != NULL){
            memset(buffer,0,300);
            strncpy(buffer, temp, strlen(temp));
            writes(client_fd, buffer);
        }
        strncpy(buffer, "\n\n", strlen("\n\n"));
    }
    fclose(fp1);
    memset(file,0,100);
    
}

/**
 * Checks if the filename ends with .txt
 * @param s Contains the string inside the buffer
 * @param suffix This is hat should be the end part 
 */ 
int str_ends_with(const char *s, const char *suffix) {
    size_t slen = strlen(s);
    size_t suffix_len = strlen(suffix);

    return suffix_len <= slen && !strcmp(s + slen - suffix_len, suffix);
}

/**
 * If client types GET command to get a certain file
 * checks if it contains a file name otherwise
 * sends error message to client
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client
 */ 
void get_command(int client_fd, char buffer[]){
    if(str_ends_with(buffer, ".txt\n") == 0 ||strcmp(buffer, "GET") == 10){
        memset(buffer,0,300);
        strncpy(buffer, "SERVER 500 Get Error\n", strlen("SERVER 500 Get Error\n"));
    }else{
        get_file_name(client_fd, buffer);
    }
}

/**
 * Reads the inputs of the client and stores it in the buffer
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client 
 */
void reading(int client_fd, char buffer[]){
    memset(buffer,0,300);
    int r = read(client_fd, buffer,300);
    if(r<0){error("Error reading from client socket");}
    printf("Read success. Msg received: %s\n",buffer);
}

/**
 * Gets the file name to be opened from the buffer
 * reads through each line that the client has put until "\n\n" and puts in a file
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client
 */
void put_file_name(int client_fd, char buffer[]){
    int ctr = 0;
    char file[100];
    for(int i = 4; i<strlen(buffer)-1; ++i){
        file[ctr++] = buffer[i];
    }
    FILE *fp1;
    fp1 = fopen(file,"w");
    if(fp1 == NULL){
        memset(buffer,0,300);
        strncpy(buffer, "SERVER 501 Put Error\n", strlen("SERVER 501 Put Error\n"));
        writes(client_fd, buffer);
    }else{
        char temp[1028];
        while(!strstr(temp, "\n\n")){
            reading(client_fd, buffer);
            strncat(temp, buffer, strlen(buffer));
            if(strstr(temp, "\n\n")){
                memset(buffer,0,300);
            }else{
                char temp2[300];
                memset(temp2,0,300);
                strncpy(temp2, buffer, strlen(buffer)-1);
                fprintf(fp1, "%s\n", temp2);
            }
        }
        fclose(fp1);
        memset(file,0,100);
        strncpy(buffer, "SERVER 201 Created\n", strlen("SERVER 201 Created\n"));
    }
}


/**
 * If client types PUT command to get a certain file
 * writes in file
 * empties file if it exists so can write in the file
 * checks if it contains a file name otherwise (not just PUT)
 * sends error message to client
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client
 */ 
void put_command(int client_fd, char buffer[]){
    if(str_ends_with(buffer, ".txt\n") == 0 || strcmp(buffer, "PUT") == 10){
        memset(buffer,0,300);
        strncpy(buffer, "SERVER 501 Put Error\n", strlen("SERVER 500 Put Error\n"));
    }else{
        put_file_name(client_fd, buffer);
    }
}

/**
 * Turns the command into all uppercase
 * @param buffer Used stores strings that are sent/recieved from the client
 */ 
void command(char buffer[]){
    for(int i = 0; i<3; i++){
        buffer[i] = toupper(buffer[i]);
    }
}

/**
 * Checks for which command the client entered and returns the corresponding response/action
 * @param client_fd The socket of the client
 * @param buffer Used stores strings that are sent/recieved from the client 
 */
void writing(int client_fd, char buffer[]){
    if(strlen(buffer) == 0){
        memset(buffer,0,300);
        strncpy(buffer, "HELLO\n", strlen("HELLO\n"));
    }else{
        command(buffer);
        if(strstr(buffer, "GET")){
            get_command(client_fd, buffer);
        }else if(strstr(buffer, "PUT")){
            put_command(client_fd, buffer);
        }else{
            strncpy(buffer, "SERVER 502 Command Error\n", strlen("SERVER 502 Command Error\n"));
        }
    }
    writes(client_fd, buffer);
}

/**
 * The main function
 * Creates the server side
 * @param argc The number of arguments in commandline
 * @param argv The array of argumnts that are on the command line
 */
int main(int argc, char *argv[])
{
    int PORT_NUM;
    if(argc < 2){return -1;}
    sscanf(argv[1], "%d",&PORT_NUM);
    if(PORT_NUM <1024){return -1;}
    
    int fd = create_socket();

    struct sockaddr_in serveraddr;
    create_server_addr(argc, &serveraddr, PORT_NUM);

    binding(fd, serveraddr);
    listening(fd);
    bool b = true;
    while(b){
    int client_fd = accept_connection(fd);

    char buffer[300];
        while(strcmp(buffer, "BYE") != 10){
            writing(client_fd, buffer);
            reading(client_fd, buffer);
        }
        memset(buffer, 0,300);
        close(client_fd);
    }
}