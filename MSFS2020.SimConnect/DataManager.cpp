#include "DataManager.h"
#include <thread>
#include <stdio.h>

PositionDataManager::PositionDataManager(HANDLE hSimConnect) : DataManager(hSimConnect, DataDefinitionId::Position, RequestId::Position)
{
    Add("PLANE LATITUDE", "Degrees");
    Add("PLANE LONGITUDE", "Degrees");
    Add("PLANE ALTITUDE", "Meters");
    Add("GPS GROUND TRUE TRACK", "Degrees");
    Add("GPS GROUND SPEED", "Meters per second");
}

const char* PositionDataManager::Convert(const DWORD* ptrData, char* buffer, size_t size)
{
    auto model = (const PositionModel*)ptrData;
    sprintf_s(buffer, size, "XGPSMSFS,%lf,%lf,%lf,%lf,%lf",
        model->Longitude,
        model->Latitude,
        model->Altitude,
        model->GroundTrack,
        model->GroundSpeed
    );

    return buffer;
}

AttitudeDataManager::AttitudeDataManager(HANDLE hSimConnect) : DataManager(hSimConnect, DataDefinitionId::Attitude, RequestId::Attitude)
    {
        Add("PLANE PITCH DEGREES", "Degrees");
        Add("PLANE BANK DEGREES", "Degrees");
        Add("PLANE HEADING DEGREES TRUE", "Degrees");
    }

const char* AttitudeDataManager::Convert(const DWORD* ptrData, char* buffer, size_t size)
{
    auto model = (const AttitudeModel*)ptrData;
    sprintf_s(buffer, size, "XATTMSFS,%lf,%lf,%lf",
        model->TrueHeading,
        -model->Pitch,
        -model->Bank
    );

    return buffer;
}    

TrafficDataManager::TrafficDataManager(HANDLE hSimConnect) : runPoll(true), pollPtr(nullptr), DataManager(hSimConnect, DataDefinitionId::Traffic)
{
    Add("PLANE LATITUDE", "Degrees");
    Add("PLANE LONGITUDE", "Degrees");
    Add("PLANE ALTITUDE", "Feet");
    Add("VELOCITY WORLD Y", "Feet per minute");
    Add("PLANE HEADING DEGREES TRUE", "Degrees");
    Add("GROUND VELOCITY", "Knots");
    Add("SIM ON GROUND", "Bool", SIMCONNECT_DATATYPE_INT32);
    Add("ATC ID", nullptr, SIMCONNECT_DATATYPE_STRING64);
}

 const char* TrafficDataManager::Convert(const DWORD* ptrData, char* buffer, size_t size)
{
    auto model = (const TrafficModel*)ptrData;
    sprintf_s(buffer, size, "XTRAFFICMSFS,%d,%0.3lf,%0.3lf,%0.1lf,%0.1lf,%d,%0.3lf,%0.1lf,%s",
        model->ObjectId,
        model->Latitude,
        model->Longitude,
        model->Altitude,
        model->VerticalSpeed,
        model->OnGround ? 0 : 1,
        model->TrueHeading,
        model->GroundVelocity,        
        model->TailNumber
    );

    return buffer;
}

void TrafficDataManager::StartRequesting()
{
    pollPtr = (intptr_t*)new std::thread([=]()
    {
        while (runPoll) 
        {
            RequestTrafficData(RequestId::Aircraft, SIMCONNECT_SIMOBJECT_TYPE_AIRCRAFT);
            RequestTrafficData(RequestId::Helicopter, SIMCONNECT_SIMOBJECT_TYPE_HELICOPTER);
            Sleep(1000 / 10);
        }
    });   
}

void TrafficDataManager::StopRequesting()
{
    if (!pollPtr)
        return;

    runPoll = false;
    ((std::thread*)pollPtr)->join();
    pollPtr = nullptr;
}

TrafficDataManager::~TrafficDataManager() 
{
    StopRequesting();
}