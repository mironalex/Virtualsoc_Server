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

void handleRequest(string req,char * username,int sd);

int loginUser(int sock,char * username);

int registerUser(int sock);

int getPosts(int sock,char* username);

int makePost(int sock,char* username);

int getPM(int sock,char* username);

int sendPM(int sock,char* username);

int addFriend(int sock,char* username);

int deleteFriend(int sock,char* username);

#endif /* REQUESTHANDLER_H */

