using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IAuthService
    {
        string Authenticate(UserModel login);
    }
}