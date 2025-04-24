using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public class MatchmakingEntry
    {
        public int UserId { get; set; }
        public float MatchmakingScore { get; set; }
    }
    
    public class MatchmakingService : IMatchmakingService
    {
        private readonly UserDb _context;
        private readonly List<MatchmakingEntry> _queue = new();
    
        public MatchmakingService(UserDb context)
        {
            _context = context;
        }
    
        public void EnqueuePlayer(int userId)
        {
            var stats = _context.Stats.FirstOrDefault(p => p.Id == userId);
            if (stats == null) return;
    
            var score = CalculateScore(stats);
    
            lock (_queue)
            {
                _queue.Add(new MatchmakingEntry
                {
                    UserId = userId,
                    MatchmakingScore = score
                });
            }
        }
    
        public void ProcessMatchmaking()
        {
            lock (_queue)
            {
                var sorted = _queue.OrderBy(q => q.MatchmakingScore).ToList();
    
                //Do we have at least 4 players
                while (sorted.Count >= 4)
                {
                    var group = sorted.Take(4).ToList();
                    //Get the list of ID's for the queued players
                    AssignPlayersToGameServer(group.Select(p => p.UserId).ToList());
                    //Remove the 4 players we just sent in a server
                    sorted = sorted.Skip(4).ToList();
                    _queue.RemoveAll(p => group.Contains(p));
                }
            }
        }
    
        private void AssignPlayersToGameServer(List<int> userIds)
        {
            var Server = _context.GameServers
                .FirstOrDefault(s => s.CurrentPlayers < s.MaxPlayers);
            
            if(Server == null)
                return;
            
            LobbyModel lobby = new LobbyModel();
            lobby.userIds = userIds;
            lobby.ServerId = Server.Id;
            
            _context.Lobby.Add(lobby);
        }
    
        private float CalculateScore(PlayerStatsModel stats)
        {
            float score = 0;
            if (stats.GamesPlayed <= 0)
            {
                score = 1;
            }
            else
            {
                score = ((stats.GamesWon * 10) / stats.GamesPlayed) + stats.CurrentWinStreak * 5;
            }
            return score;
        }
    }
}