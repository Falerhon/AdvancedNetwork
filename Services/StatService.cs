using CUBEGAMEAPI.Models;
using System.Linq;

namespace CUBEGAMEAPI.Services
{
    public class StatService : IStatService
    {
        private readonly UserDb _context;
        
        public StatService(UserDb context, IAchievementService achievementService)
        {
            _context = context;
        }

        public PlayerStatsModel GetStats(int id)
        {
            return _context.Stats.FirstOrDefault(s => s.Id == id);
        }

        public void UpdateStats(PlayerStatsModel stats)
        {
            _context.Stats.Update(stats);
            _context.SaveChanges();
        }

        public void RecordWin(int id)
        {
            var stats = GetOrCreateStats(id);
            stats.GamesWon++;
            stats.CurrentWinStreak++;
            stats.GamesPlayed++;
            UpdateStats(stats);
        }

        public void RecordLoss(int id)
        {
            var stats = GetOrCreateStats(id);
            stats.CurrentWinStreak = 0;
            stats.GamesPlayed++;
            UpdateStats(stats);
        }

        public void RecordObjectDestroyed(int id, int count = 1)
        {
            var stats = GetOrCreateStats(id);
            stats.ObjectsDestroyed += count;
            UpdateStats(stats);
        }

        private PlayerStatsModel GetOrCreateStats(int id)
        {
            var stats = GetStats(id);
            if (stats == null)
            {
                stats = new PlayerStatsModel
                {
                    GamesWon = 0,
                    ObjectsDestroyed = 0,
                    CurrentWinStreak = 0
                };
                _context.Stats.Add(stats);
                _context.SaveChanges();
            }
            return stats;
        }
    }
}