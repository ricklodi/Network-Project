#include "NetworkManager.h"
#include <iostream>
#include <WS2tcpip.h> // needed for inet_pton

using namespace std;

//Creating our singleton instance initialization
NetworkManager* NetworkManager::instance = nullptr;

NetworkManager::NetworkManager()
{
	UDPSocketIn = INVALID_SOCKET;
	UDPSocketOut = INVALID_SOCKET;

	TCPSocketIn = INVALID_SOCKET;
	for (int i = 0; i < MAX_USERS; i++)
	{
		TCPSocketOut[i] = INVALID_SOCKET;
	}

	UDPOutAddr = { 0 };
	UDPinAddr = { 0 };
	TCPOutAddr = { 0 };
	TCPinAddr = { 0 };
}

NetworkManager::~NetworkManager()
{
	//It doesn't need the delete keyword since we want this instance to be alive for the whole program
	//And it is only a singleton
}

void NetworkManager::Init()
{
	cout << "NetworkManager::Init() called" << endl;

	WSADATA lpWASData;	//The WSAStartup function initiates use of the Winsock DLL by a process.
	int error = WSAStartup(MAKEWORD(2, 2), &lpWASData);	//MAKEWORD is specifying the version number of winsock to use

	//Error will return 0 in case it doesn't have any issues
	if (error != 0)
	{
		cout << "WSAData failed with error code:" << WSAGetLastError() << endl;
	}
}

void NetworkManager::Shutdown()
{
	cout << "NetworkManager::Shutdown() called" << endl;
	int error = WSAGetLastError();
	if (error != 0)
	{
		cout << "WSAData failed with error code:" << error << endl;
	}
	//Safety check for UDP SOCKET IN
	if (UDPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketIn) != 0)
		{
			cout << "[ERROR] closing UDP in" << endl;
		}
	}
	//Safety check for UDP SOCKET OUT
	if (UDPSocketOut != INVALID_SOCKET)
	{
		if (closesocket(UDPSocketOut) != 0)
		{
			cout << "[ERROR] closing UDP out" << endl;
		}
	}
	//Safety check for TCP SOCKET IN

	if (TCPSocketIn != INVALID_SOCKET)
	{
		if (closesocket(TCPSocketIn) != 0)
		{
			cout << "[ERROR] closing TCP in" << endl;
		}
	}

	for (int i = 0; i < numConnections; i++)
	{
		//Safety check for TCP SOCKET OUT
		if (TCPSocketOut[i] != INVALID_SOCKET)
		{
			if (closesocket(TCPSocketOut[i]) != 0)
			{
				cout << "[ERROR] closing TCP out" << endl;
			}
		}
	}
	//Clean up memory established in the INIT
	WSACleanup();
	exit(0);
}

void NetworkManager::BindUDP()
{
	// using IPv4
	UDPinAddr.sin_family = AF_INET;
	//Port 8889
	UDPinAddr.sin_port = htons(8889); //host to network short
	//INADDR_ANY: Use any available address
	UDPinAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int bindError = bind(UDPSocketIn, reinterpret_cast<SOCKADDR*>(&UDPinAddr), sizeof(UDPinAddr));

	if (bindError == SOCKET_ERROR)
	{
		cout << "[ERROR] binding failed..." << endl;

		Shutdown();
	}
}

void NetworkManager::BindTCP(u_short numPort)
{
	TCPinAddr.sin_family = AF_INET;
	TCPinAddr.sin_port = htons(numPort);
	TCPinAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bindError = bind(TCPSocketIn, reinterpret_cast<SOCKADDR*>(&TCPinAddr), sizeof(TCPinAddr));

	if (bindError == SOCKET_ERROR)
	{
		cout << "[ERROR] binding failed..." << endl;

		Shutdown();
	}
}

void NetworkManager::ListenTCP()
{
	//SOMAXCONN - limit of users, very large number

	listen(TCPSocketIn, SOMAXCONN);
	
}

void NetworkManager::ConnectTCP(u_short numPort, string ip4Address)
{
	TCPOutAddr.sin_family = AF_INET;
	TCPOutAddr.sin_port = htons(numPort);

	//inet_pton(AF_INET, "127.0.0.1", &TCPOutAddr.sin_addr);
	inet_pton(AF_INET, ip4Address.c_str(), &TCPOutAddr.sin_addr);

	int connectStatus = connect(TCPSocketOut[numConnections], reinterpret_cast<sockaddr*>(&TCPOutAddr), sizeof(TCPOutAddr));

	if (connectStatus == SOCKET_ERROR)
	{
		cout << "[ERROR] Connection through TCP failed." << endl;
		Shutdown();
	}

	numConnections++;

	u_long unblocking = 1;
	ioctlsocket(TCPSocketOut[numConnections-1], FIONBIO, &unblocking);
	
}
//**TCP has a socket for just ACCEPTING new clients ONLY
//Server -> bind -> Listen -> Connect with Client
//The client cannot send info without the Server accepting the connection
//A socket will be created specialy for this client
//To accept multiple clients, it needs an array
void NetworkManager::AcceptConnectionsTCP()
{
	u_long unblocking = 1;
	int clientSize = sizeof(TCPOutAddr);
	//Once it accepts TCPOutAddr populates
	//It is using numConnections instead of for because it would overwrite the already accepted connections
	TCPSocketOut[numConnections] = accept(TCPSocketIn,
		reinterpret_cast<sockaddr*>(&TCPOutAddr), &clientSize);

	if (TCPSocketOut[numConnections] != INVALID_SOCKET)
	{
		char ipConnected[32];
		//Reverse Function - pulling out IP information
		inet_ntop(AF_INET, &TCPOutAddr.sin_addr, ipConnected, 32);
		cout << "User with ip: " << ipConnected << " just connected" << endl;
		numConnections++; //is going to be used in the lab to know how many users connected
		//numUsers++;
	}	
	ioctlsocket(TCPSocketOut[numConnections-1], FIONBIO, &unblocking);
	
	ioctlsocket(TCPSocketIn, FIONBIO, &unblocking); //It would block after we go to listen
}

void NetworkManager::SetRemoteData()
{
	//Setup where/how to send the data
	//IPv4
	UDPOutAddr.sin_family = AF_INET;
	//use port 8889
	//htons convert from host style to network byte order
	UDPOutAddr.sin_port = htons(8889);
	//inet_pton sets up our destination, protocol and IP address and
	//outputs to the substruct of sockaddr within SOCKADDER_IN
	inet_pton(AF_INET, "127.0.0.1", &UDPOutAddr.sin_addr);
}

void NetworkManager::CreateUDPSockets()
{
	cout << "NetworkManager::CreateUDPSockets() called" << endl;
	//AF_INET means we are using IPv4

	UDPSocketIn = socket(AF_INET, SOCK_DGRAM,  IPPROTO_UDP);
	if (UDPSocketIn == INVALID_SOCKET)
	{
		cout << "Udp socket in failed to create." << endl;
		Shutdown();
	}

	UDPSocketOut = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPSocketOut == INVALID_SOCKET)
	{
		cout << "Udp socket out failed to create." << endl;
		Shutdown();
	}
}

void NetworkManager::CreateTCPSockets()
{
	cout << "NetworkManager::CreateTCPSockets() called" << endl;
	//AF_INET means we are using IPv4

	TCPSocketIn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (TCPSocketIn == INVALID_SOCKET)
	{
		cout << "TCP socket in failed to create." << endl;
		Shutdown();
	}

	for (int i = 0; i < MAX_USERS; i++)
	{
		TCPSocketOut[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (TCPSocketOut[i] == INVALID_SOCKET)
		{
			cout << "TCP socket out failed to create." << endl;
			Shutdown();
		}
	}
	u_long iMode = 1;
	ioctlsocket(TCPSocketIn, FIONBIO, &iMode);
}

void NetworkManager::SendDataUDP(const char* message)
{
	//New API call sendto
	int totalByteSent = sendto(UDPSocketOut, message, strlen(message)+1, 0,
		reinterpret_cast<SOCKADDR*>(&UDPOutAddr), sizeof(UDPOutAddr));

	//check for error
	if (totalByteSent == SOCKET_ERROR)
	{
		Shutdown();
	}

	cout << " sent : " << totalByteSent << " of data" << endl;
}

void NetworkManager::SendDataTCP(const char* message)
{
	for (int i = 0; i < numConnections; i++)
	{
		if (i != indexClient)
		{
			int totalByteSent = send(TCPSocketOut[i], message, strlen(message) + 1, 0);

			//check for error
			if (totalByteSent == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK)
				{
					cout << " sent : " << totalByteSent << " of data" << endl;
				}
				else
				{
					cout << "[ERROR] TCP Send Failed" << endl;
					Shutdown();
				}
			}
		}
	}
	//Reset the sender "ID"
	indexClient = -1;
}

//recv vs recvfrom - sendto
//TCP keeps track of it in the TCPOutSocket when creating a connection so it doesn't need those parameters passed in
int NetworkManager::ReceiveDataTCP(char* message)
{
	int byteR = 0;
	for (int i = 0; i < numConnections; i++)
	{
		int bytesReceived = recv(TCPSocketOut[i], message, MAX_MESSAGE, 0);

		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				cout << "[ERROR] error receiving from TCP" << endl;
				Shutdown();
			}
		}
		if (bytesReceived > 0)
		{
			byteR = bytesReceived;
			if (isServer == true)
				indexClient = i;
		}
	}
	return byteR;
}

int NetworkManager::ReceiveDataUDP(char* message)
{
	int ByteReceived = 0;
	int inAddrSize = sizeof(UDPinAddr);

	ByteReceived = recvfrom(UDPSocketIn, message, MAX_MESSAGE, 0, 
		reinterpret_cast<SOCKADDR*>(&UDPinAddr), &inAddrSize);
	//Check for error
	if (ByteReceived == SOCKET_ERROR)
	{
		Shutdown();
	}
	return ByteReceived;
}