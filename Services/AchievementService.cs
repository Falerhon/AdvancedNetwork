using CUBEGAMEAPI.Models;


namespace CUBEGAMEAPI.Services
{
    public class AchievementService : IAchievementService
    {
        private readonly UserDb _context;

        public AchievementService(UserDb context)
        {
            _context = context;
        }
    
        public void CheckForNewAchievements(int userId, PlayerStatsModel stats)
        {
            
            var allAchievements = _context.Achievements.ToList();
            var unlockedIds = _context.UserAchievements
                .Where(ua => ua.UserId == userId && ua.IsUnlocked)
                .Select(ua => ua.AchievementId)
                .ToHashSet();

            foreach (var ach in allAchievements)
            {
                if (unlockedIds.Contains(ach.Id)) continue;

                var statValue = GetStatValue(stats, ach.StatToTrack);
                if (statValue >= ach.Threshold)
                {
                    _context.UserAchievements.Add(new UserAchievement
                    {
                        UserId = userId,
                        AchievementId = ach.Id,
                        IsUnlocked = true
                    });
                }
            }

            _context.SaveChanges();
        }

        public int GetStatValue(PlayerStatsModel stats, string statName)
        {
            return statName switch
            {
                "ObjectsDestroyed" => stats.ObjectsDestroyed,
                "GamesWon" => stats.GamesWon,
                "WinStreak" => stats.CurrentWinStreak,
                "GamesPlayed" => stats.GamesPlayed,
                _ => 0
            };
        }
    }
}


