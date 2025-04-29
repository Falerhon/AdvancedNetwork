using CUBEGAMEAPI.Models;

namespace CUBEGAMEAPI.Services
{
    public interface IAuthService
    {
        (string, int) Authenticate(UserModel login);
    }
}