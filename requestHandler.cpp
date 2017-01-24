/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "requestHandler.h"

#define GET_POSTS "GET" //requests to get posts from a user
#define MAKE_POST "PST" //requests to make a post
#define GET_PM "GPM" //requests to get a PM
#define SEND_PM "SPM" //requests to send a PM to a conversation
#define ADD_FRIEND "ADD" //requests to add a friend
#define DELETE_FRIEND "DEL" //requests to delete a friend
#define REGISTER_USER "REG" //requests to register a user
#define LOGIN_USER "LOG" //requests to login

connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
void handleRequest(string req, char * username, int sd){
	if(req == GET_POSTS){
		getPosts(sd,username);
	}
	else if(req == MAKE_POST){
		makePost(sd,username);
	}
	else if(req == GET_PM){
		getPM(sd,username);
	}
	else if(req == SEND_PM){
		sendPM(sd,username);
	}
	else if(req == ADD_FRIEND){
	    addFriend(sd,username);
	}
	else if(req == DELETE_FRIEND){
	    deleteFriend(sd,username);
	}
	else if(req == REGISTER_USER){
        registerUser(sd);
	}
	else if(req == LOGIN_USER){
	    loginUser(sd,username);
	}
	else{
		exit(1);
	}
}

int registerUser(int sock){
    work dbAccess(dbConnection);
    char * password, *username;
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
    command.append(dbAccess.esc(username));
    command.append("\'");
    r = dbAccess.exec(command);
    if(r.size() > 0) {
        sendMessage(sock, "Error, duplicate user...");
        return 0;
    }
    char passhash[1000];
    passToSHA512(password,passhash);
    command = "INSERT into users (username, passhash) VALUES (\'";
    command.append(dbAccess.esc(username));
    command.append("\', \'");
    command.append(passhash);
    command.append("\');");
    delete[] password;
    delete[] username;
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
int loginUser(int sock,char * username){
    work dbAccess(dbConnection);
    char * password,buffer[100],passhash[1000];
    int x;
    x = readInt(sock);
    if(x > 52) {
        sendMessage(sock,"Invalid request, user and or password too long...");
        return 0;
    }
    read(sock,buffer,x);
    char * pch = strtok(buffer,":");
    strcpy(username,pch);
    username[strlen(pch)] = 0;
    pch = strtok(NULL,":");
    pch[strlen(pch)]='\0';
    password = strdup(pch);
    passToSHA512(password,passhash);
    string command = "select * from users where username like \'";
    command.append(dbAccess.esc(username));
    command.append("\' and passhash like \'");
    command.append(passhash);
    command.append("\'");
    result r = dbAccess.exec(command);
    delete[] password;
    if(r.size() == 1) {
        sendMessage(sock,"Login Successful.");
        return 1;
    }
    else {
        sendMessage(sock,"Login Failed.");
        delete[] username;
        return 0;
    }
}

int getPosts(int sock,char * username){
    work dbAccess(dbConnection);
    char buffer[1000],*index,*count,*pch,*from;
    int bufferSize;
    bufferSize = readInt(sock);
    if(bufferSize < 10) {
        sendMessage(sock,"Bad get request.");
    }
    read(sock,buffer,bufferSize);
    pch = strtok(buffer,":");
    from = strdup(pch);
    pch = strtok(NULL,":");
    index = strdup(pch);
    pch = strtok(NULL,":");
    count = strdup(pch);
    string command;
    if(username == NULL){
        sendMessage(sock,"Error, client not logged in.");
        return 0;
    }
    if(strcmp(username,from) != 0) {
        command = "select * from (select * from posts where author = \'";
        command.append(dbAccess.esc(from));
        command += "\' order by date desc ) as foo OFFSET ";
        command.append(dbAccess.esc(index));
        command += " LIMIT ";
        command.append(count);
        result r = dbAccess.exec(command);
        string postCount = to_string(r.size());
        sendMessage(sock,postCount);
        for(int i = 0; i < r.size(); i++){
            sendMessage(sock,r[i][0].as<string>());
            sendMessage(sock,r[i][1].as<string>());
        }
        return 1;
    }
    else{
        command = "select * from (select author,text,date from posts p ";
        command += "join friends f on p.author = f.username ";
        command += "where friend_username = \'"; command.append(username);
        command += "\' and p.type <= f.type ";
        command += "order by date desc ) recentPosts ";
        command += "OFFSET "; command.append(index);
        command += " LIMIT "; command.append(count);
        result r = dbAccess.exec(command);
        string postCount = to_string(r.size());
        sendMessage(sock,postCount);
        for(int i = 0; i < r.size(); i++){
            sendMessage(sock,r[i][0].as<string>());
            sendMessage(sock,r[i][1].as<string>());
        }
        return 1;
    }
}

int makePost(int sock,char * username){
    work dbAccess(dbConnection);
    char buffer[300], *post,*pch,type[10];
    int bufferSize;
    bufferSize = readInt(sock);
    if(bufferSize > 300) {
        sendMessage(sock,"Invalid request, post too long.");
        return 0;
    }
    read(sock,buffer,bufferSize);
    if(username == NULL) {
        sendMessage(sock, "Error, you are not logged in.");
        return 0;
    }
    int i = 0;
    while(buffer[i]!=':'){
        type[i] = buffer[i];
        i++;
    }
    i++;
    post = new char[bufferSize-i];
    int j = 0;
    while(i < bufferSize-1){
        post[j] = buffer[i];
        i++;
        j++;
    }
    post[j] = 0;
    if(atoi(type) < 0 || atoi(type) > 3) {
        sendMessage(sock,"Invalid friend type...");
    }
    string command = "INSERT INTO public.posts(author, type, text, date) VALUES (\'";
    command.append(dbAccess.esc(username));
    command += "\',";
    command.append(type);
    command +=", \'";
    command.append(dbAccess.esc(post));

    command +="\', current_timestamp)";
    try{
        dbAccess.exec(command.c_str());
        dbAccess.commit();
    }
    catch(pqxx::pqxx_exception &e){
        cerr << e.base().what() << "\nSQL ERROR\n" << endl;
        sendMessage(sock,"Post Failed.");
        return 0;
    }
    catch(const exception &e){
        cerr << e.what() << endl;
        sendMessage(sock,"Post Failed");
        return 0;
    }
    sendMessage(sock,"Post Successful.");
    return 1;
}

int getPM(int sock,char * username){

}

int sendPM(int sock,char * username){

}

int addFriend(int sock,char * username){
    work dbAccess(dbConnection);

    //Getting and parsing the package sent from the client

    char buffer[1000], *pch,befriend[26],type[2];
    int bufferSize;
    bufferSize = readInt(sock);
    read(sock,buffer,bufferSize);
    pch = strtok(buffer,":");
    strcpy(type,pch);
    pch = strtok(NULL,":");
    strcpy(befriend,pch);

    //Check if the user to be added as a friend exists or not

    string command = "select username from users where username like \'";
    command.append(dbAccess.esc(befriend));
    command.append("\'");
    result r = dbAccess.exec(command);
    if(r.size() == 0) {
        sendMessage(sock, "Error, invalid user...");
        return 0;
    }

    //Getting ready to add friend relationship to the DB

    command = "INSERT INTO public.friends(username, friend_username, type) VALUES (\'";
    command+= dbAccess.esc(username) + "\', \'" + dbAccess.esc(befriend) + "\', " + dbAccess.esc(type) +")";
    try{
        dbAccess.exec(command.c_str());
        dbAccess.commit();
    }
    catch(pqxx::pqxx_exception &e){
        cerr << e.base().what() << "\nSQL ERROR\n" << endl;
        sendMessage(sock,"Friend Add Failed.");
        return 0;
    }
    catch(const exception &e){
        cerr << e.what() << endl;
        sendMessage(sock,"Friend Add Failed");
        return 0;
    }
    sendMessage(sock,"Friend Added Successful.");
    return 1;
}

int deleteFriend(int sock,char * username){
    work dbAccess(dbConnection);
}

