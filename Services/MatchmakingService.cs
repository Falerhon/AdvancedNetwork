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
        private readonly List<MatchmakingEntry> _queue = new();
    
        public void EnqueuePlayer(int userId, float score)
        {
            lock (_queue)
            {
                _queue.Add(new MatchmakingEntry
                {
                    UserId = userId,
                    MatchmakingScore = score
                });
            }
        }
    
        public List<int> ProcessMatchmaking()
        {
            var matchedUsers = new List<int>();
            lock (_queue)
            {
                //Sort the queue so the players have a similar score
                var sorted = _queue.OrderBy(q => q.MatchmakingScore).ToList();
    
                //Do we have at least 4 players
                if (sorted.Count >= 4)
                {
                    var group = sorted.Take(4).ToList();
                    //Get the list of ID's for the queued players
                    matchedUsers = group.Select(p => p.UserId).ToList();
                }
            }

            return matchedUsers;
        }

        public void RemoveFromMatchmaking(List<int> IDs)
        {
            lock (_queue)
            {
                //Remove the players we just sent in a server
                _queue.RemoveAll(p => IDs.Contains(p.UserId));
            }
        }
    }
}