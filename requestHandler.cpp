/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "requestHandler.h"

#define GET_POSTS "GET" /**<macro for the get posts request. */
#define MAKE_POST "PST" /**<macro for the make post request. */
#define GET_PM "GPM" /**<macro for the get PMs request. */
#define GET_FRIENDS "GFL" /**<macro for the get friends request. */
#define GET_FRIEND_REQUESTS "GFR" /**<macro for the get friend requests request. */
#define SEND_PM "SPM" /**<macro for the send PM request. */
#define ADD_FRIEND "ADD" /**<macro for the add friend request. */
#define DELETE_FRIEND "DEL" /**<macro for the delete friend request. */
#define REGISTER_USER "REG" /**<macro for the register user request. */
#define LOGIN_USER "LOG" /**<macro for the login request. */
#define SEND_GROUP_MESSAGE "SGM" /**<macro for the send group message request. */
#define GET_GROUP_MESSAGES "GGM" /**<macro for the get group messages request. */
#define LEAVE_GROUP "LGR" /**<macro for the leave group request. */
#define INVITE_TO_GROUP "ITG" /**<macro for the invite to group request. */
#define GET_GROUP_PARTICIPANTS "GGP" /**<macro for the get group participants request. */
#define GET_GROUP_LIST "GGL" /// /**<macro for the get group list request. */
#define DB_SPECS "dbname=Virtualsoc_DB user=Virtualsoc_Admin password=admin hostaddr=127.0.0.1 port=5432"
int handleRequest(string req, char * username, int sd){
	if(req == GET_POSTS){
		getPosts(sd,username);
	}
	else if(req == MAKE_POST){
        if(username == NULL) return 0;
		makePost(sd,username);
	}
	else if(req == GET_PM){
        if(username == NULL) return 0;
		getPM(sd,username);
	}
	else if(req == SEND_PM){
        if(username == NULL) return 0;
		sendPM(sd,username);
	}
	else if(req == ADD_FRIEND){
        if(username == NULL) return 0;
	    addFriend(sd,username);
	}
	else if(req == DELETE_FRIEND){
        if(username == NULL) return 0;
	    deleteFriend(sd,username);
	}
	else if(req == REGISTER_USER){
        registerUser(sd);
	}
	else if(req == LOGIN_USER){
	    loginUser(sd,username);
	}
    else if(req == GET_FRIENDS){
        if(username == NULL) return 0;
        getFriends(sd,username);
    }
    else if(req == GET_FRIEND_REQUESTS) {
        if(username == NULL) return 0;
        getFriendRequests(sd, username);
    }
    else if(req == SEND_GROUP_MESSAGE){
        if(username == NULL) return 0;
        sendGroupMessage(sd,username);
    }
    else if(req == GET_GROUP_MESSAGES){
        if(username == NULL) return 0;
        getGroupMessages(sd,username);
    }
    else if(req == LEAVE_GROUP){
        if(username == NULL) return 0;
        leaveGroup(sd,username);
    }
    else if(req == INVITE_TO_GROUP){
        if(username == NULL) return 0;
        inviteToGroup(sd,username);
    }
    else if(req == GET_GROUP_PARTICIPANTS){
        if(username == NULL) return 0;
        getGroupParticipants(sd,username);
    }
    else if(req == GET_GROUP_LIST){
        if(username == NULL) return 0;
        getGroupList(sd,username);
    }
	else{
        return 0;
    }
    return 1;
}

int registerUser(int sock){
    connection dbConnection(DB_SPECS);
    work dbAccess(dbConnection);
    char * password, *username;

    int usernameSize = readInt(sock);
    if(usernameSize > 25){
        sendMessage(sock,"Error, user too long.");
    }
    username = new char[usernameSize+1];
    read(sock,username,usernameSize);
    username[usernameSize] = 0;

    int passwordSize = readInt(sock);
    password = new char[passwordSize+1];
    read(sock,password,passwordSize);
    password[passwordSize] = 0;

    string command;
    command = "select username from users where username like \'";
    command.append(dbAccess.esc(username));
    command.append("\'");
    result r = dbAccess.exec(command);
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
    connection dbConnection(DB_SPECS);
    work dbAccess(dbConnection);
    char * password,passhash[1000];

    int usernameSize = readInt(sock);
    if(usernameSize > 25) {
        sendMessage(sock,"Error, user too long");
        return 0;
    }

    read(sock,username,usernameSize);
    username[usernameSize] = 0;

    int passwordSize = readInt(sock);
    password = new char[usernameSize+1];
    read(sock,password,passwordSize);
    password[passwordSize] = 0;

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
    }\
    else {
        sendMessage(sock,"Login Failed.");
        return 0;
    }
}

int getPosts(int sock,char * username){
    connection dbConnection(DB_SPECS);
    work dbAccess(dbConnection);
    char buffer[1000],*index,*count,*pch,*from;
    int bufferSize;
    bufferSize = readInt(sock);
    if(bufferSize < 10) {
        sendMessage(sock,"Bad get request.");
        return 0;
    }
    read(sock,buffer,bufferSize);
    pch = strtok(buffer,":");
    from = strdup(pch);
    pch = strtok(NULL,":");
    index = strdup(pch);
    pch = strtok(NULL,":");
    count = strdup(pch);
    string command;
    if(strcmp(from,"GUEST")==0){
        command = "select author,text,to_char(date,'dd-mm-yyyy HH24:mm') from posts where type = 0 order by date desc";
        result r = dbAccess.exec(command);
        sendMessage(sock,to_string(r.size()));
        for(int i = 0; i < r.size(); i++){
            sendMessage(sock,r[i][0].c_str());
            sendMessage(sock,r[i][1].c_str());
            sendMessage(sock,r[i][2].c_str());
        }
        return 1;
    }
    if(username == NULL) {
        sendMessage(sock, "Error, client not logged in.");
        return 0;
    }
    if(strcmp(username,from) != 0) {
        command = "select author,text,to_char(date,'dd-mm-yyyy HH24:mm')  from posts join friends f on f.friend_username = author where (author = '";
        command.append(dbAccess.esc(from));
        command += "' and ((posts.type <= f.type and username = '"; command.append(dbAccess.esc(username));
        command+="') or posts.type = 0)) order by date desc";
        /*
        command+=" OFFSET ";
        command.append(dbAccess.esc(index));
        command += " LIMIT ";
        command.append(dbAccess.esc(count));
         */
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
    else{
        command = "select author,text,to_char(date,'dd-mm-yyyy HH24:mm') from posts join friends f1 on f1.friend_username = author where author in (select f.username from friends f where f.friend_username = '";
        command.append(username);
        command += "' and f.username in(select f2.friend_username from friends f2 where f2.username = '";
        command.append(username);
        command += "')) and f1.username = '"; command.append(username); command+="' and f1.type >= posts.type ";
        command +="order by date desc ";
        /*
        command += "OFFSET "; command.append(index);
        command += " LIMIT "; command.append(count);
         */
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
    connection dbConnection(DB_SPECS);
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

    string command ="select sender,message,to_char(date,'dd-mm-yyyy HH24:mm') from pms where (sender = '"; command.append(username);
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

    string command = "select * from friends where (username ='"; command.append(dbAccess.esc(username));
    command+= "' and friend_username='"; command.append(dbAccess.esc(reciever));
    command+= "') or (username ='"; command.append(dbAccess.esc(reciever));
    command+="' and friend_username='"; command.append(dbAccess.esc(username));
    command+="')";

    result r = dbAccess.exec(command);
    if(r.size()!=2){
        return 0;
    }

    command = "INSERT INTO public.pms(sender, reciever, message, date) VALUES ( '"; command.append(dbAccess.esc(username));
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
    connection dbConnection(DB_SPECS);
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



    //Check if the group already exists

    string command = "select * from groups where groupname ='"; command.append(dbAccess.esc(groupName));
    command+= "'";

    result r1 = dbAccess.exec(command);

    command+= " and username = '"; command.append(dbAccess.esc(username)); command+="'";

    result r2 = dbAccess.exec(command);

    if(r1.size() > 0 && r2.size() == 0){
        //The group already exists and the user that invited someone is not in it.
        return 0;
    }

    //Check if the one invited isn't already in the group.
    command = "select * from groups where groupname ='"; command.append(dbAccess.esc(groupName));
    command+= "' and username = '"; command.append(dbAccess.esc(user)); command+="'";

    r1 = dbAccess.exec(command);
    if(r1.size() != 0){
        //The one invited is already in the group
        return 0;
    }

    //Check if the one invited exists as a user

    command = "select * from users where username ='"; command.append(dbAccess.esc(user)); command+="'";

    r1 = dbAccess.exec(command);
    if(r1.size() == 0){
        //User does not exist
        return 0;
    }

    command = "INSERT INTO public.groups(username, groupname) VALUES ('";
    command.append(dbAccess.esc(user)); command+="', '"; command.append(dbAccess.esc(groupName)); command+="')";

    dbAccess.exec(command);
    dbAccess.commit();
    delete[] user;
    delete[] groupName;
    return 1;
}

int leaveGroup(int sock, char* username){
    connection dbConnection(DB_SPECS);
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
    connection dbConnection(DB_SPECS);
    char * groupName, * message;
    work dbAccess(dbConnection);
    int groupSize, messageSize;

    groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    messageSize = readInt(sock);
    message = new char[messageSize+1];
    read(sock,message,messageSize);
    message[messageSize] = 0;

    string command = "select * from groups where groupname ='"; command.append(dbAccess.esc(groupName));
    command+= "'";

    result r1 = dbAccess.exec(command);
    if(r1.size() == 0){
        //The group to which the message is sent does not exist
        return 0;
    }

    command+= " and username = '"; command.append(dbAccess.esc(username)); command+="'";

    r1 = dbAccess.exec(command);
    if(r1.size() == 0){
        //The author of the message is not in the group
        return 0;
    }

    command = "INSERT INTO public.group_pms(username, groupname, message, date) VALUES ('";
    command.append(dbAccess.esc(username)); command+= "', '";
    command.append(dbAccess.esc(groupName)); command+= "', '";
    command.append(dbAccess.esc(message)); command+="', current_timestamp)";
    dbAccess.exec(command);
    dbAccess.commit();
    return 1;
}

int getGroupMessages(int sock, char* username){
    connection dbConnection(DB_SPECS);
    work dbAccess(dbConnection);
    char * groupName;
    int groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    //Check if the user that requested the messages is in the group

    string command = "select * from groups where groupname ='"; command.append(dbAccess.esc(groupName));
    command+= "' and username = '"; command.append(dbAccess.esc(username)); command+="'";
    result r = dbAccess.exec(command);

    if(r.size() == 0){
        //user is not in the group
        return 0;
    }


    command = "SELECT username,message,to_char(date,'dd-mm-yyyy HH24:mm') from group_pms where groupname = '";
    command.append(dbAccess.esc(groupName)); command+="'";

    r = dbAccess.exec(command);
    sendMessage(sock,to_string(r.size()));
    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
        sendMessage(sock,r[i][1].c_str());
        sendMessage(sock,r[i][2].c_str());
    }
    return 1;
}

int getGroupParticipants(int sock, char* username){
    connection dbConnection(DB_SPECS);
    work dbAccess(dbConnection);

    char * groupName;
    int groupSize = readInt(sock);
    groupName = new char[groupSize+1];
    read(sock,groupName,groupSize);
    groupName[groupSize] = 0;

    //Check if the user that requested the messages is in the group

    string command = "select * from groups where groupname ='"; command.append(dbAccess.esc(groupName));
    command+= "' and username = '"; command.append(dbAccess.esc(username)); command+="'";
    result r = dbAccess.exec(command);

    if(r.size() == 0){
        //user is not in the group
        return 0;
    }

    command = "select username from groups where groupname = '"; command.append(dbAccess.esc(groupName));
    command+="'";

    r = dbAccess.exec(command);
    sendMessage(sock,to_string(r.size()));

    for(int i = 0; i < r.size(); i++){
        sendMessage(sock,r[i][0].c_str());
    }
    return 1;
}

int getGroupList(int sock, char* username){
    connection dbConnection(DB_SPECS);
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
