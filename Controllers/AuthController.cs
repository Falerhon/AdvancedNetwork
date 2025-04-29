using Microsoft.AspNetCore.Mvc;
using CUBEGAMEAPI.Models;
using CUBEGAMEAPI.Services;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class AuthController : ControllerBase
    {
        private readonly IAuthService _authService;

        public AuthController(IAuthService authService)
        {
            _authService = authService;
        }

        [HttpPost("login")]
        public IActionResult Login([FromBody] UserModel login)
        {
            (string token,int id) = _authService.Authenticate(login);
            if (token == null)
                return Unauthorized();

            return Ok(new { token, id });
        }
    }
}