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

void handleRequest(string req,char * username, int sd);

int loginUser(int sd);

int registerUser(int sd);

int getPosts(int sd);

int makePost(int sd);

int getPM(int sd);

int sendPM(int sd);

int addFriend(int sd);

int deleteFriend(int sd);

#endif /* REQUESTHANDLER_H */

