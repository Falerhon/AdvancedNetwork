using CUBEGAMEAPI.Services;
using CUBEGAMEAPI.DTOs;
using Microsoft.AspNetCore.Mvc;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/server/auth")]
    public class GameServerAuthController : ControllerBase
    {
        private readonly IGameServerAuthService _authService;

        public GameServerAuthController(IGameServerAuthService authService)
        {
            _authService = authService;
        }

        [HttpPost("login")]
        public IActionResult Login([FromBody] GameServerLoginRequest request)
        {
            var token = _authService.Authenticate(request.ServerName, request.Password);
            if (token == null)
                return Unauthorized();

            return Ok(new { token });
        }
    }
}

