../Virtualsoc_Server.out: main.cpp requestHandler.cpp user.cpp
	g++ -std=gnu++11 main.cpp requestHandler.cpp user.cpp -o Virtualsoc_Server.out -lpthread -lpqxx -lpq -lcrypto
	
