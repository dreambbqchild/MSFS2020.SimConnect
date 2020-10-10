#include <iostream>
#include <thread>
#include "BroadcastSocket.h"
#include "DataManager.h"

#define ESC "\x1B"
#define ClearLine ESC "[2K"
#define MoveToTemplate ESC "[%d;%dH"
#define BUFFER_SIZE 256
bool hasQuit = false;

PositionDataManager* position;
AttitudeDataManager* attitude;
TrafficDataManager* traffic;
BroadcastSocket broadcastSocket;

struct ConsoleData {
    char Position[BUFFER_SIZE];
    char Attitude[BUFFER_SIZE];    
};
ConsoleData liveData = { 0 };

void PrintState() 
{
    while (!hasQuit) 
    {
        ConsoleData copiedData = liveData;
        printf(MoveToTemplate ClearLine "%s\r\n" ClearLine "%s", 2, 1, copiedData.Position, copiedData.Attitude);
        Sleep(1000);
    }
}

void CALLBACK MessageProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        char buffer[BUFFER_SIZE] = {0};
        TrafficModel model = { };
        auto objData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;
        if (objData->dwObjectID == SIMCONNECT_OBJECT_ID_USER || objData->dwDefineID != (DWORD)DataDefinitionId::Traffic)
            return;      

        memcpy(&model, &objData->dwData, sizeof(TrafficModelRaw));
        model.ObjectId = objData->dwObjectID;
        broadcastSocket.Send(traffic->Convert((DWORD*)&model, buffer, BUFFER_SIZE));
    }
    break;
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
    {
        auto pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
        char* buffer = nullptr;
        ConsoleData workingData = liveData;
        DataManager* manager = nullptr;

        switch ((RequestId)pObjData->dwRequestID)
        {
        case RequestId::Position:
            manager = position;
            buffer = workingData.Position;
            break;
        case RequestId::Attitude:
            manager = attitude;
            buffer = workingData.Attitude;
            break;
        default:
            return;
        }

        broadcastSocket.Send(manager->Convert(&pObjData->dwData, buffer, BUFFER_SIZE));
        liveData = workingData;
        break;
    }

    case SIMCONNECT_RECV_ID_QUIT:
    {
        hasQuit = true;
        break;
    }

    default:
        break;
    }
}

HANDLE InitConsole() 
{
    auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return nullptr;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
        return nullptr;

    return hOut;
}

int main()
{       
    InitConsole();
    HANDLE hSimConnect = nullptr;
    auto hEventHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEventHandle) {
        std::cout << "Unable to create event :(" << std::endl;
        return 0;
    }

    BroadcastSocket::Starting();

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "MSFS2020.SimConnect", nullptr, 0, hEventHandle, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL)))
    {   
        std::thread printState(PrintState);
        std::cout << "Connected!" <<  std::endl;        
        
        broadcastSocket.Open();
        position = new PositionDataManager(hSimConnect);
        attitude = new AttitudeDataManager(hSimConnect);
        traffic = new TrafficDataManager(hSimConnect);

        position->StartRequesting();
        attitude->StartRequesting();
        traffic->StartRequesting();

        while (!hasQuit && ::WaitForSingleObject(hEventHandle, INFINITE) == WAIT_OBJECT_0)
            SimConnect_CallDispatch(hSimConnect, MessageProc, NULL);
                  
        printState.join();
        delete position;
        delete attitude;
        delete traffic;
        broadcastSocket.Close();        
        SimConnect_Close(hSimConnect);
        CloseHandle(hEventHandle);
    }    
    else
        std::cout << "Failed to Connect :(" << std::endl;

    BroadcastSocket::Quitting();
}
