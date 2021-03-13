#pragma once
template <class T>
class NetworkData
{
private:
    static T GetData(DWORD(*fn)(T, PULONG, BOOL), BOOL ordered)
    {
        ULONG bufSize = 0;
        T temp = nullptr;
        fn(temp, &bufSize, ordered);
        temp = (T)calloc(1, bufSize);
        if (!temp)
            exit(1);

        fn(temp, &bufSize, ordered);
        return temp;
    }

public:
    const T data;

    NetworkData(DWORD(*fn)(T, PULONG, BOOL), BOOL ordered) 
        : data(GetData(fn, ordered)) {}

    virtual ~NetworkData() 
    {
        free(data);
    }
};