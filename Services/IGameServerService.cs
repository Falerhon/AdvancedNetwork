using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IGameServerService
    {
        void RegisterOrUpdate(GameServer server);
    }
}