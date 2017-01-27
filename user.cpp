//
// Created by fox on 1/20/17.
//

#include "user.h"

/**
 * A function that reads an integer from a socket
 * @param sock the socket descriptor from where the value is read
 * @return returns the integer that was read
 */
int readInt (int sock){
    int value = 0;
    ssize_t size;
    size = read(sock, &value, 4);
    if(size != 4){
		return -1;
	}
    return value;
}

/**
 * A function that sends a message to a socket
 * @param sock socket to which the message is sent
 * @param message string that contains the message to be sent
 */
void sendMessage(int sock, string message){
    unsigned long messageSize = message.size();
    write(sock, (char *)&messageSize, 4);
    dprintf(sock,"%s",message.c_str());
}

/**
 * Function that reads from the command line a user
 * @param input the user will be saved here
 * @param n length of the user
 * @param input_stream FILE pointer from where the reading will be done
 * @return length of the user
 */

int getUser(char *input, int n, FILE * input_stream){
    fgets(input, n, stdin);
    if(input[strlen(input)-1] == '\n') input[strlen(input)-1] = '\0';
    return strlen(input);
}

/**
 * function that securely reads a password from a command line
 * @param input the password will be saved here
 * @param n length of the password
 * @param input_stream FILE pointer from where the reading will be done
 * @return length of the username
 */

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

/**
 * Function that takes a password as input and then stores its SHA512 hash into the 2nd argument
 * @param pass the input password
 * @param hexHash the output SHA512 hash
 */

void passToSHA512(const char *pass,char hexHash[1000]){
    unsigned char pass_digest[SHA512_DIGEST_LENGTH*2+1];
    SHA512((unsigned char *)pass,strlen(pass),(unsigned char *)&pass_digest);

    for(int i = 0; i < 128/2; i++){
        sprintf(&hexHash[i*2], "%02x", (unsigned int)pass_digest[i]);
    }
    hexHash[128] = '\0';
}



