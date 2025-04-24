namespace CUBEGAMEAPI.Services
{
    public interface IGameServerAuthService
    {
        string Authenticate(string serverName, string password);
    }
}