#pragma once
#include "includes.h"
#include "Models.h"
#include "BroadcastSocket.h"

const int About42Miles = 67593;

enum class RequestId : DWORD {
    None,
    Position,
    Attitude,
    Aircraft,
    Helicopter
};

enum class DataDefinitionId : DWORD {
    Position,
    Attitude,
    Traffic
};

class DataManager
{
private:
    const RequestId requestId;

protected:
    const HANDLE hSimConnect;
    const DataDefinitionId defineId;    

    inline void Add(const char* datumName, const char* unitsName, SIMCONNECT_DATATYPE datumType = SIMCONNECT_DATATYPE_FLOAT64, float epsilon = 0, DWORD datumID = SIMCONNECT_UNUSED)
    {
        SimConnect_AddToDataDefinition(hSimConnect, (SIMCONNECT_DATA_DEFINITION_ID)defineId, datumName, unitsName, datumType, epsilon, datumID);
    }

public:    
    DataManager(HANDLE hSimConnect, DataDefinitionId defineId, RequestId requestId = RequestId::None) : hSimConnect(hSimConnect), requestId(requestId), defineId(defineId) { }

    inline void StartRequesting()
    {
        SimConnect_RequestDataOnSimObject(hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)requestId, (SIMCONNECT_DATA_DEFINITION_ID)defineId, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME, SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
    }

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size) PURE;
    virtual ~DataManager() {}
};

class PositionDataManager : public DataManager
{
public:
    PositionDataManager(HANDLE hSimConnect);

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size);
    virtual ~PositionDataManager() {}
};

class AttitudeDataManager : public DataManager
{
public:
    AttitudeDataManager(HANDLE hSimConnect);

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size);
    virtual ~AttitudeDataManager() {}
};

class TrafficDataManager : public DataManager
{
private:
    bool runPoll;
    intptr_t* pollPtr;

    inline void RequestTrafficData(RequestId requestId, SIMCONNECT_SIMOBJECT_TYPE type)
    {
        SimConnect_RequestDataOnSimObjectType(hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)requestId, (SIMCONNECT_DATA_DEFINITION_ID)defineId, About42Miles, type);
    }

public:
    TrafficDataManager(HANDLE hSimConnect);

    virtual const char* Convert(const DWORD* ptrData, char* buffer, size_t size);
    virtual void StartRequesting();
    void StopRequesting();

    virtual ~TrafficDataManager();
};