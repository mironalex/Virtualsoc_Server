/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "requestHandler.h"

#define GET_POSTS "GET" //requests to get posts from a user
#define MAKE_POST "PST" //requests to make a post
#define GET_PM "GPM" //requests to get a PM
#define GET_FRIENDS "GFL" //requests to get the friend list of a user
#define GET_FRIEND_REQUESTS "GFR" //requests to get all of the friend requests of a user
#define SEND_PM "SPM" //requests to send a PM to a conversation
#define ADD_FRIEND "ADD" //requests to add a friend
#define DELETE_FRIEND "DEL" //requests to delete a friend
#define REGISTER_USER "REG" //requests to register a user
#define LOGIN_USER "LOG" //requests to login
#define SEND_GROUP_MESSAGE "SGM" // requests to send a group message
#define GET_GROUP_MESSAGES "GGM" // requests to get all of the group messages from a specific group
#define LEAVE_GROUP "LGR" // requests to leave a group
#define INVITE_TO_GROUP "ITG" // requests to invite someone to a group
#define GET_GROUP_PARTICIPANTS "GGP" // requests a list of all participants
#define GET_GROUP_LIST "GGL" // requests a list of conversations

int handleRequest(string req, char * username, int sd){
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
    else if(req == GET_FRIENDS){
        getFriends(sd,username);
    }
    else if(req == GET_FRIEND_REQUESTS) {
        getFriendRequests(sd, username);
    }
    else if(req == SEND_GROUP_MESSAGE){
        sendGroupMessage(sd,username);
    }
    else if(req == GET_GROUP_MESSAGES){
        getGroupMessages(sd,username);
    }
    else if(req == LEAVE_GROUP){
        leaveGroup(sd,username);
    }
    else if(req == INVITE_TO_GROUP){
        inviteToGroup(sd,username);
    }
    else if(req == GET_GROUP_PARTICIPANTS){
        getGroupParticipants(sd,username);
    }
    else if(req == GET_GROUP_LIST){
        getGroupList(sd,username);
    }
	else{
        return 0;
    }
    return 1;
}

int registerUser(int sock){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
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
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
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
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
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
        command = "select author,text,to_char(date,'dd-mm-yyyy HH:mm') from posts join friends f1 on f1.friend_username = author where author in (select f.username from friends f where f.friend_username = '";
        command.append(username);
        command += "' and f.username in(select f2.friend_username from friends f2 where f2.username = '";
        command.append(username);
        command += "')) and f1.username = '"; command.append(username); command+="' and f1.type >= posts.type ";
        command +="order by date desc ";
        command += "OFFSET "; command.append(index);
        command += " LIMIT "; command.append(count);
        result r = dbAccess.exec(command);
        string postCount = to_string(r.size());
        sendMessage(sock,postCount);
        for(int i = 0; i < r.size(); i++){
            sendMessage(sock,r[i][0].as<string>());
            sendMessage(sock,r[i][1].as<string>());
            sendMessage(sock,r[i][2].as<string>());
        }
        return 1;
    }
}

int makePost(int sock,char * username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
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
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * partner;
    int partnerSize = readInt(sock);
    partner = new char[partnerSize+1];
    partner[partnerSize] = 0;
    read(sock,partner,partnerSize);

    string command ="select sender,message,to_char(date,'dd-mm-yyyy HH:mm') from pms where (sender = '"; command.append(username);
    command+= "' and reciever = '"; command.append(partner);
    command+= "') or (sender ='"; command.append(partner);
    command+= "' and reciever = '"; command.append(username); command+="') ORDER BY date asc";

    result r = dbAccess.exec(command);

    sendMessage(sock,to_string(r.size()));

    for(int i = 0 ; i < r.size(); i++){
        sendMessage(sock,r[i][0].as<string>());
        sendMessage(sock,r[i][1].as<string>());
        sendMessage(sock,r[i][2].as<string>());
    }
}

int sendPM(int sock,char * username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * reciever, *message;
    int recieverSize, messageSize;

    recieverSize = readInt(sock);
    reciever = new char[recieverSize+1];
    read(sock,reciever,recieverSize);
    reciever[recieverSize] = 0;

    messageSize = readInt(sock);
    message = new char[messageSize+1];
    read(sock,message,messageSize);
    message[messageSize] = 0;

    string command = "INSERT INTO public.pms(sender, reciever, message, date) VALUES ( '"; command.append(dbAccess.esc(username));
    command += "', '"; command.append(dbAccess.esc(reciever));
    command += "', '"; command.append(dbAccess.esc(message));
    command += "', current_timestamp )";
    dbAccess.exec(command);
    dbAccess.commit();
}

int addFriend(int sock,char * username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
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

    //Check if the user to be added as a friend isn't already added as a friend

    command = "select * from friends where username = '"; command.append(dbAccess.esc(username));
    command+= "' and friend_username = '"; command.append(dbAccess.esc(befriend));
    command+="'";
    r = dbAccess.exec(command);
    if(r.size() != 0){
        sendMessage(sock, "Error, user is already your friend.");
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
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * exFriend;
    int exFriendSize;
    exFriendSize = readInt(sock);
    exFriend = new char[exFriendSize+1];
    read(sock,exFriend,exFriendSize);
    exFriend[exFriendSize] = 0;
    string command = "DELETE FROM public.friends WHERE (username = '"; command.append(dbAccess.esc(username));
    command+= "' and friend_username = '"; command.append(dbAccess.esc(exFriend));
    command+= "') or (username = '"; command.append(dbAccess.esc(exFriend));
    command+= "' and friend_username = '"; command.append(dbAccess.esc(username));
    command+="')";
    dbAccess.exec(command);
    dbAccess.commit();
}

int getFriends(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    string command = "select * from friends f where f.friend_username = '"; command.append(dbAccess.esc(username));
    command+="' and f.username in(select f2.friend_username from friends f2 where f2.username = '"; command.append(dbAccess.esc(username));
    command+="')";
    result r = dbAccess.exec(command);
    string postCount = to_string(r.size());
    sendMessage(sock,postCount);
    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
    }
}

int getFriendRequests(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    string command = "select * from friends f where f.friend_username = '"; command.append(dbAccess.esc(username));
    command+="' and f.username not in(select f2.friend_username from friends f2 where f2.username = '"; command.append(dbAccess.esc(username));
    command+="')";
    result r = dbAccess.exec(command);
    string postCount = to_string(r.size());
    sendMessage(sock,postCount);
    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
    }
}

int inviteToGroup(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char *user,*groupName;
    int userSize,groupSize;

    userSize = readInt(sock);
    user = new char[userSize+1];
    read(sock,user,userSize);
    user[userSize] = 0;

    groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    string command = "INSERT INTO public.groups(username, groupname) VALUES ('";
    command.append(dbAccess.esc(user)); command+="', '"; command.append(dbAccess.esc(groupName)); command+="')";

    dbAccess.exec(command);
    dbAccess.commit();
    delete[] user;
    delete[] groupName;
    return 1;
}

int leaveGroup(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * groupName;
    int groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;
    string command = "DELETE FROM public.groups WHERE groupname='"; command.append(dbAccess.esc(groupName));
    command+="' and username ='"; command.append(dbAccess.esc(username)); command+="'";
    dbAccess.exec(command);
    dbAccess.commit();

}

int sendGroupMessage(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * groupName, * message;
    int groupSize, messageSize;

    groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    messageSize = readInt(sock);
    message = new char[messageSize+1];
    read(sock,message,messageSize);
    message[messageSize] = 0;

    string command = "INSERT INTO public.group_pms(username, groupname, message, date) VALUES ('";
    command.append(dbAccess.esc(username)); command+= "', '";
    command.append(dbAccess.esc(groupName)); command+= "', '";
    command.append(dbAccess.esc(message)); command+="', current_timestamp)";
    dbAccess.exec(command);
    dbAccess.commit();
    return 1;
}

int getGroupMessages(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    char * groupName;
    int groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    string command = "SELECT username,message,to_char(date,'dd-mm-yyyy HH:mm') from group_pms where groupname = '";
    command.append(dbAccess.esc(groupName)); command+="'";

    result r = dbAccess.exec(command);
    sendMessage(sock,to_string(r.size()));
    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
        sendMessage(sock,r[i][1].c_str());
        sendMessage(sock,r[i][2].c_str());
    }
    return 1;
}

int getGroupParticipants(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);

    char * groupName;
    int groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    string command = "select username from groups where groupname = '"; command.append(dbAccess.esc(groupName));
    command+="'";

    result r = dbAccess.exec(command);
    sendMessage(sock,to_string(r.size()));

    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
    }
    return 1;
}

int getGroupList(int sock, char* username){
    connection dbConnection("dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432");
    work dbAccess(dbConnection);
    string command = "select groupname from groups where username ='"; command.append(dbAccess.esc(username));
    command+="'";
    result r = dbAccess.exec(command);
    string groupCount = to_string(r.size());
    sendMessage(sock,groupCount);
    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
    }
    return 1;
}