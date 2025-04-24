using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IAchievementService
    {
        void CheckForNewAchievements(int userId, PlayerStatsModel stats);

        int GetStatValue(PlayerStatsModel stats, string statName);
    }
}