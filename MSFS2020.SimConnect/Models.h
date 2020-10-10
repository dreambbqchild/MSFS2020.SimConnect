#pragma once
#pragma pack(push, 1)
struct PositionModel
{
    double Latitude, Longitude, Altitude, GroundTrack, GroundSpeed;
};

struct AttitudeModel
{
    double Pitch, Bank, TrueHeading;
};

struct TrafficModelRaw
{
    double Latitude, Longitude, Altitude, VerticalSpeed, TrueHeading, GroundVelocity;
    uint32_t OnGround;
    char TailNumber[64];
};

struct TrafficModel : public TrafficModelRaw {
    int ObjectId;
};

#pragma pack(pop)