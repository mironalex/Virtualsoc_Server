/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   user.h
 * Author: fox
 *
 * Created on December 20, 2016, 9:14 PM
 */

#include <cstdio>
#include <termios.h>
#include <pqxx/pqxx>
#include <openssl/sha.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;
using namespace pqxx;

#ifndef USER_H
#define USER_H

int readInt (int sock);

void sendInt (int sock,int value);

void sendMessage(int sock, string message);

int getUser(char *input, int n, FILE * input_stream);

int getPassword(char *input, int n, FILE * input_stream);

void passToSHA512(const char *pass,char hexHash[1000]);




#endif /* USER_H */

