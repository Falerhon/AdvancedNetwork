using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IGameServerService
    {
        void RegisterOrUpdate(GameServer server);
        void RemoveServer(GameServer server);
        void MarkAsOccupied(GameServer server, bool occupied);
        void RemoveLobby(int serverId);
    }
}