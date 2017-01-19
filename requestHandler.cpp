/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "requestHandler.h"
#include "user.h"

#define GET_POSTS "GET" //requests to get posts from a user
#define MAKE_POST "PST" //requests to make a post
#define GET_PM "GPM" //requests to get a PM
#define SEND_PM "SPM" //requests to send a PM to a conversation
#define ADD_FRIEND "ADD" //requests to add a friend
#define DELETE_FRIEND "DEL" //requests to delete a friend
#define REGISTER_USER "REG" //requests to register a user
#define LOGIN_USER "LOG" //requests to login
connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
void handleRequest(string req,char* username, int sd){
	if(req == GET_POSTS){
		getPosts(sd);
	}
	else if(req == MAKE_POST){
		makePost(sd);
	}
	else if(req == GET_PM){
		getPM(sd);
	}
	else if(req == SEND_PM){
		sendPM(sd);
	}
	else if(req == ADD_FRIEND){
	    addFriend(sd);
	}
	else if(req == DELETE_FRIEND){
	    deleteFriend(sd);
	}
	else if(req == REGISTER_USER){
        registerUser(sd);
	}
	else if(req == LOGIN_USER){
	    loginUser(sd);
	}
	else{
		exit(1);
	}
}

int registerUser(int sock){
    work dbAccess(dbConnection);
    char * username, * password;
    result r;
    int x = readInt(sock);
    if(x > 52) {
        sendMessage(sock,"Invalid request, user and or password too long...");
        return 0;
    }
    char buffer[100];
    read(sock,buffer,x);
    char * pch;
    pch = strtok(buffer,":");
    username = strdup(pch);
    pch = strtok(NULL,":");
    password = strdup(pch);
    string command;
    command = "select username from users where username like \'";
    command.append(username);
    command.append("\'");
    r = dbAccess.exec(command);
    if(r.size() > 0) {
        sendMessage(sock, "Error, duplicate user...");
        return 0;
    }
    char passhash[1000];
    passToSHA512(password,passhash);
    command = "INSERT into users (username, passhash) VALUES (\'";
    command.append(username);
    command.append("\', \'");
    command.append(passhash);
    command.append("\');");
    delete[] username;
    delete[] password;
    try{
        dbAccess.exec(command.c_str());
        dbAccess.commit();
    }
    catch(pqxx::pqxx_exception &e){
        cerr << e.base().what() << "\nSQL ERROR\n" << endl;
        return 0;
    }
    catch(const exception &e){
        cerr << e.what() << endl;
        return 0;
    }
    sendMessage(sock,"Register Successful.");
    return 1;
}
int loginUser(int sock){
    work dbAccess(dbConnection);
    char * username, * password,buffer[100],passhash[1000];;
    int x;
    x = readInt(sock);
    if(x > 52) {
        sendMessage(sock,"Invalid request, user and or password too long...");
        return 0;
    }
    read(sock,buffer,x);
    char * pch = strtok(buffer,":");
    username = strdup(pch);
    pch = strtok(NULL,":");
    pch[strlen(pch)]='\0';
    password = strdup(pch);
    passToSHA512(password,passhash);
    string command = "select * from users where username like \'";
    command.append(username);
    command.append("\' and passhash like \'");
    command.append(passhash);
    command.append("\'");
    result r = dbAccess.exec(command);
    delete[] username;
    delete[] password;
    if(r.size() == 1) {
        sendMessage(sock,"Login Successful.");
        return 1;
    }
    else {
        sendMessage(sock,"Login Failed.");
        return 0;
    }
}

int getPosts(int sd){

}

int makePost(int sd){

}

int getPM(int sd){

}

int sendPM(int sd){

}

int addFriend(int sd){

}

int deleteFriend(int sd){

}

