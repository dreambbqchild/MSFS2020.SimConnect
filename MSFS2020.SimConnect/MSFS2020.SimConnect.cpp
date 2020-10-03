#include <iostream>
#include "BroadcastSocket.h"
#include "DataManager.h"

bool hasQuit = false;

PositionDataManager* position;
AttitudeDataManager* attitude;
BroadcastSocket broadcastSocket;

void CALLBACK MessageProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext)
{
    switch (pData->dwID)
    {
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
    {
        char buffer[128] = {0};
        auto pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
        DataManager* manager = nullptr;

        switch ((RequestId)pObjData->dwRequestID)
        {
        case RequestId::Position:
        {
            manager = position;
            break;
        }
        break;
        case RequestId::Attitude:
        {
            manager = attitude;
            break;
        }
        default:
            return;
        }

        broadcastSocket.Send(manager->Convert(&pObjData->dwData, buffer, sizeof(buffer)));
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

int main()
{       
    HANDLE hSimConnect = nullptr;
    auto hEventHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hEventHandle) {
        std::cout << "Unable to create event :(" << std::endl;
        return 0;
    }

    BroadcastSocket::Starting();

    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "MSFS2020.SimConnect", nullptr, 0, hEventHandle, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL)))
    {        
        std::cout << "Connected!" <<  std::endl;        
        
        broadcastSocket.Open();
        position = new PositionDataManager(hSimConnect);
        attitude = new AttitudeDataManager(hSimConnect);

        position->StartRequesting();
        attitude->StartRequesting();

        while (!hasQuit && ::WaitForSingleObject(hEventHandle, INFINITE) == WAIT_OBJECT_0)
            SimConnect_CallDispatch(hSimConnect, MessageProc, NULL);
                  
        delete position;
        delete attitude;
        broadcastSocket.Close();        
        SimConnect_Close(hSimConnect);
        CloseHandle(hEventHandle);
    }    
    else
        std::cout << "Failed to Connect :(" << std::endl;

    BroadcastSocket::Quitting();
}
