using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public class GameServerService : IGameServerService
    {
        private readonly UserDb _context;

        public GameServerService(UserDb context)
        {
            _context = context;
        }

        public void RegisterOrUpdate(GameServer server)
        {
            var existing = _context.GameServers
                .FirstOrDefault(s => s.IP == server.IP && s.Port == server.Port);

            if (existing != null)
            {
                existing.MaxPlayers = server.MaxPlayers;
                existing.CurrentPlayers = server.CurrentPlayers;
                existing.IsOnline = true;
                existing.IsOccupied = false;
                existing.LastHeartbeat = DateTime.UtcNow;
            }
            else
            {
                server.IsOnline = true;
                server.LastHeartbeat = DateTime.UtcNow;
                _context.GameServers.Add(server);
            }

            _context.SaveChanges();
        }

        public void MarkOfflineStaleServers(TimeSpan timeout)
        {
            var threshold = DateTime.UtcNow.Subtract(timeout);
            var staleServers = _context.GameServers
                .Where(s => s.IsOnline && s.LastHeartbeat < threshold)
                .ToList();

            foreach (var server in staleServers)
            {
                server.IsOnline = false;
            }

            _context.SaveChanges();
        }

        //Update weather or not the server is currently hosting a game
        public void MarkAsOccupied(GameServer server, bool occupied)
        {
            var serv = _context.GameServers
                .FirstOrDefault(s => s.IP == server.IP && s.Port == server.Port);
            
            if(serv == null)
                return;

            serv.IsOccupied = occupied;
            _context.SaveChanges();
        }

        //Delete the lobby linked to the server id
        public void RemoveLobby(int serverId)
        {
            var lobby = _context.Lobby.FirstOrDefault(l => l.ServerId == serverId);
            
            if(lobby == null)
                return;

            _context.Remove(lobby);

            _context.SaveChanges();
        }
    }
}

