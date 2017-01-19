//
// Created by fox on 1/20/17.
//

#include "user.h"

int readInt (int fd){
    int x = 0;
    ssize_t size;
    size = read(fd, &x, 4);
    if(size != 4) return -1;
    return x;
}

void sendMessage(int fd, string message){
    unsigned long messageSize = message.size();
    if( write(fd, (char *)&messageSize, 4) != 4) throw("Failed to write message size");
    dprintf(fd,"%s",message.c_str());
}

int getUser(char *input, int n, FILE * input_stream){
    fgets(input, n, stdin);
    if(input[strlen(input)-1] == '\n') input[strlen(input)-1] = '\0';
    return strlen(input);
}

int getPassword(char *input, int n, FILE * input_stream){
    struct termios old_term, new_term;
    if(tcgetattr(fileno(input_stream), &old_term) != 0) return -1;
    new_term = old_term;
    new_term.c_lflag &= ~ECHO;
    if(tcsetattr(fileno(input_stream), TCSAFLUSH, &new_term) != 0) return -1;
    fgets(input, n, stdin);
    if(input[strlen(input)-1] == '\n') input[strlen(input)-1] = '\0';
    (void) tcsetattr(fileno(input_stream), TCSAFLUSH, &old_term);
    return strlen(input);
}

void passToSHA512(const char *pass,char hexHash[1000]){
    unsigned char pass_digest[SHA512_DIGEST_LENGTH*2+1];
    SHA512((unsigned char *)pass,strlen(pass),(unsigned char *)&pass_digest);

    for(int i = 0; i < 128/2; i++){
        sprintf(&hexHash[i*2], "%02x", (unsigned int)pass_digest[i]);
    }
    hexHash[128] = '\0';
}



