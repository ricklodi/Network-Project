//Ricardo Lodi Monteiro

#include <iostream>
#include "NetworkManager.h"
#include <string>
#include <conio.h>

using namespace std;

int main()
{
    std::cout << "Messaging Program.\n";

    NetworkManager::GetInstance()->Init();
    NetworkManager::GetInstance()->CreateTCPSockets();

    const int SERVER = 1;
    //Adding user input for IP and Port
    string portNumber;
    string ipAddress;

    string username;

    cout << "Port number: " << endl;
    getline(cin, portNumber);

    cout << "IP Address: " << endl;
    getline(cin, ipAddress);

    u_short port = stoi(portNumber);

    //----------------------------------

    cout << "Choose a role: " << endl;
    cout << "\t1) Server" << endl;
    cout << "\t2) Client" << endl;

    int choice = 0;
    cin >> choice;

    if (choice == SERVER)
    {
        NetworkManager::GetInstance()->BindTCP(port);
        NetworkManager::GetInstance()->ListenTCP();
    }
    else
    {
        //ADDED username choice
        cout << "Choose your name: " << endl;
        cin >> username;
        username += " says: ";
        NetworkManager::GetInstance()->ConnectTCP(port, ipAddress);
    }

    while (NetworkManager::GetInstance()->GetNumConnections() <= 0)
    {
        if (choice == SERVER)
        {
            NetworkManager::GetInstance()->AcceptConnectionsTCP();
            NetworkManager::GetInstance()->SetServer();
        }
    }
    
    string myMsg;
    while (true)
    {
        if (choice == SERVER)
        {
            NetworkManager::GetInstance()->AcceptConnectionsTCP();
        }
        if (_kbhit())
        {
            getline(cin, myMsg);
            if (myMsg.length() > 0)
            {
                myMsg.insert(0, username); //Inser username to message sent
                NetworkManager::GetInstance()->SendDataTCP(myMsg.c_str());
            }
        }
        char rcvMessage[NetworkManager::MAX_MESSAGE];
        int size = NetworkManager::GetInstance()->ReceiveDataTCP(rcvMessage);
        
        if (size > 0)
        {
            cout << rcvMessage << endl << endl;
            if (choice == SERVER)//Broadcast
            {
                NetworkManager::GetInstance()->SendDataTCP(rcvMessage);
            }
        }
    }

    NetworkManager::GetInstance()->Shutdown();

    return 0;
}