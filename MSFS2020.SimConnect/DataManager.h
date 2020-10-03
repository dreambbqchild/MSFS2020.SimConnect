#pragma once
#include <windows.h>
#include <simconnect.h>
#include "Models.h"
#include "BroadcastSocket.h"

enum class RequestId : DWORD {
    Position,
    Attitude
};

enum class DataDefinitionId : DWORD {
    Position,
    Attitude
};

class DataManager
{
private:    
    HANDLE hSimConnect;
    DataDefinitionId defineId;
    const RequestId requestId;

protected:
    void Add(const char* datumName, const char* unitsName, SIMCONNECT_DATATYPE datumType = SIMCONNECT_DATATYPE_FLOAT64, float epsilon = 0, DWORD datumID = SIMCONNECT_UNUSED)
    {
        auto hr = SimConnect_AddToDataDefinition(hSimConnect, (SIMCONNECT_DATA_DEFINITION_ID)defineId, datumName, unitsName, datumType, epsilon, datumID);
    }

public:    
    DataManager(HANDLE hSimConnect, DataDefinitionId defineId, RequestId requestId) : hSimConnect(hSimConnect), requestId(requestId), defineId(defineId) { }

    void StartRequesting()
    {
        SimConnect_RequestDataOnSimObject(hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)requestId, (SIMCONNECT_DATA_DEFINITION_ID)defineId, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND, SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
    }
    
    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size) PURE;
    virtual ~DataManager() {}
};

class PositionDataManager : public DataManager
{
public:
    PositionDataManager(HANDLE hSimConnect) : DataManager(hSimConnect, DataDefinitionId::Position, RequestId::Position) 
    {
        Add("PLANE LATITUDE", "Degrees");
        Add("PLANE LONGITUDE", "Degrees");
        Add("PLANE ALTITUDE", "Meters");
        Add("GPS GROUND TRUE TRACK", "Degrees");
        Add("GPS GROUND SPEED", "Meters per second");
    }    

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size)
    {
        auto model = (const PositionModel*)ptrData;
        sprintf_s(buffer, size, "XGPSMSFS,%0.3f,%0.3f,%0.2f,%0.2f,%0.2f",
            model->Longitude,
            model->Latitude,
            model->Altitude,
            model->GroundTrack,
            model->GroundSpeed
        );

        return buffer;
    }
    virtual ~PositionDataManager() {}
};

class AttitudeDataManager : public DataManager
{
public:
    AttitudeDataManager(HANDLE hSimConnect) : DataManager(hSimConnect, DataDefinitionId::Attitude, RequestId::Attitude) 
    {
        Add("PLANE PITCH DEGREES", "Degrees");
        Add("PLANE BANK DEGREES", "Degrees");
        Add("PLANE HEADING DEGREES TRUE", "Degrees");
    }    

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size)
    {
        auto model = (const AttitudeModel*)ptrData;
        sprintf_s(buffer, size, "XATTMSFS,%0.3f,%0.3f,%0.3f",
            model->TrueHeading,
            -model->Pitch,
            -model->Bank
        );

        return buffer;
    }
    virtual ~AttitudeDataManager() {}
};