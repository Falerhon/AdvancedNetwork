

#include "LinkingContext.h"

LinkingContext::LinkingContext() : _NextId(1) {
}


NetworkId LinkingContext::Register(MBObject *object) {
    NetworkId id = _NextId++;
    _IdToObjectMap[id] = object;
    _ObjectToIdMap[object] = id;
    return id;
}

void LinkingContext::Register(NetworkId id, MBObject* object) {
    _IdToObjectMap[id] = object;
    _ObjectToIdMap[object] = id;
}

void LinkingContext::Unregister(MBObject* object) {
    auto it = _ObjectToIdMap.find(object);
    if (it != _ObjectToIdMap.end())
    {
        NetworkId id = it->second;
        _IdToObjectMap.erase(id);
        _ObjectToIdMap.erase(it);
    }
}

MBObject * LinkingContext::GetObjectByNetwordId(NetworkId id) const {
    auto it = _IdToObjectMap.find(id);
    if (it != _IdToObjectMap.end())
        return it->second;
    return nullptr;
}

NetworkId LinkingContext::GetNetworkId(MBObject* object) const {
    auto it = _ObjectToIdMap.find(object);
    if (it != _ObjectToIdMap.end())
        return it->second;

    return 0; //Invalid
}
