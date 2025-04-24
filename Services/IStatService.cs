using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IStatService
    {
        PlayerStatsModel GetStats(int id);
        void UpdateStats(PlayerStatsModel stats);
        void RecordWin(int id);
        void RecordLoss(int id);
        void RecordObjectDestroyed(int id, int count = 1);
    }
}