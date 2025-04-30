#ifndef LINKINGCONTEXT_H
#define LINKINGCONTEXT_H

#include <unordered_map>
#include <cstdint>

class MBObject;

using NetworkId = uint32_t;

class LinkingContext {

    public:
    LinkingContext();
    ~LinkingContext() = default;
    LinkingContext(const LinkingContext&) = default;
    LinkingContext& operator=(const LinkingContext&) = default;
    LinkingContext(LinkingContext&&) = default;
    LinkingContext& operator=(LinkingContext&&) = default;

    NetworkId Register(MBObject* object);

    void Register(NetworkId id, MBObject* object);
    void Unregister(MBObject* object);

    MBObject* GetObjectByNetwordId(NetworkId id) const;
    NetworkId GetNetworkId(MBObject* object) const;

private:
    NetworkId _NextId;
    std::unordered_map<NetworkId, MBObject*> _IdToObjectMap;
    std::unordered_map<MBObject*, NetworkId> _ObjectToIdMap;
};



#endif //LINKINGCONTEXT_H
