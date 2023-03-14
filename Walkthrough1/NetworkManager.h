#pragma once
#pragma comment (lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <string>

using namespace std;

class NetworkManager
{
public:
	static NetworkManager* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new NetworkManager();
		}
		return instance;
	}

	void Init();
	void Shutdown();
	void CreateUDPSockets();
	void CreateTCPSockets();
	void BindUDP();
	void BindTCP(u_short numPort);
	//Listen TCP vs UDP
	//TCP is like a pipeline and needs a conection to listen, while UDP just listen to the socket
	void SetRemoteData();
	void SendDataUDP(const char* message);
	int ReceiveDataUDP(char* message);
	void ListenTCP();
	void ConnectTCP(u_short numPort, string ip4Address);
	void AcceptConnectionsTCP();
	void SendDataTCP(const char* message);
	int ReceiveDataTCP(char* message);

	int GetNumConnections() { return numConnections; }

	static const int MAX_MESSAGE = 1500;
	static const int MAX_USERS = 4;

	//Function to tell the program if it is a server
	bool GetServer() { return isServer; }
	void SetServer() { isServer = true; }

	int GetUserID() { return indexClient; }
	
private:
	NetworkManager();
	~NetworkManager();

	SOCKET UDPSocketIn;
	SOCKET UDPSocketOut;

	SOCKET TCPSocketIn; //In is just FOR LISTENING
	SOCKET TCPSocketOut[MAX_USERS]; 

	SOCKADDR_IN UDPOutAddr;
	SOCKADDR_IN UDPinAddr;

	SOCKADDR_IN TCPOutAddr; //USED Only to storeinforation (STRUCT)
	SOCKADDR_IN TCPinAddr;

	int numConnections = 0;

	bool isServer = false;
	//Variable for the message sent by the current CLIENT
	int indexClient = -1;
	//Singleton instance
	static NetworkManager* instance;

};

