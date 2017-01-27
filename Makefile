server.out: main.cpp requestHandler.cpp user.cpp
	g++ -std=gnu++11 main.cpp requestHandler.cpp user.cpp -o server.out -lpthread -lpqxx -lpq -lcrypto
	
