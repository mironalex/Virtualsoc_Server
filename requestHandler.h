/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   requestHandler.h
 * Author: fox
 *
 * Created on January 9, 2017, 10:47 AM
 */

#include "user.h"

using namespace std;
using namespace pqxx;

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

/**
 * This is a request handler that checks the type of a request then calls the appropriate function
 * @param req the type of request
 * @param username the username of the client
 * @param sd a socket descriptor to the client
 * @return returns 1 when it gets a proper request and 0 otherwise
 */
int handleRequest(string req,char * username,int sd);

/**
 * Function that handles a login request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int loginUser(int sock,char * username);

/**
 * Function that handles a register request
 * @param sock a socket to the client
 * @return 1 on success 0 otherwise
 */
int registerUser(int sock);

/**
 * Function that handles a get posts request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getPosts(int sock,char* username);

/**
 * Function that handles a make post request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int makePost(int sock,char* username);

/**
 * Function that handles a get PMs request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getPM(int sock,char* username);

/**
 * Function that handles a send PM request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int sendPM(int sock,char* username);

/**
 * Function that handles a add friend request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int addFriend(int sock,char* username);

/**
 * Function that handles a delete friend request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int deleteFriend(int sock,char* username);

/**
 * Function that handles a get friends request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getFriends(int sock,char* username);

/**
 * Function that handles a get friend requests request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getFriendRequests(int sock, char* username);

/**
 * Function that handles a get group messages request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getGroupMessages(int sock, char* username);

/**
 * Function that handles a send group message request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int sendGroupMessage(int sock,char* username);


/**
 * Function that handles a leave group request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int leaveGroup(int sock, char* username);

/**
 * Function that handles a invite to group request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int inviteToGroup(int sock,char* username);

/**
 * Function that handles a get group participants request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getGroupParticipants(int sock, char* username);

/**
 * Function that handles a get group list request
 * @param sock a socket to the client
 * @param username describes the user of the client
 * @return 1 on success 0 otherwise
 */
int getGroupList(int sock, char* username);

#endif /* REQUESTHANDLER_H */

