namespace CUBEGAMEAPI.Services
{
    public interface IMatchmakingService
    {
        void EnqueuePlayer(int userId, float score);
        List<int> ProcessMatchmaking();
        void RemoveFromMatchmaking(List<int> IDs);
    }
}