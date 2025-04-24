namespace CUBEGAMEAPI.Services
{
    public interface IMatchmakingService
    {
        void EnqueuePlayer(int userId);
        void ProcessMatchmaking();
    }
}