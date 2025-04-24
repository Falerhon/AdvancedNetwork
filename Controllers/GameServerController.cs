using Microsoft.AspNetCore.Mvc;
using CUBEGAMEAPI.Models;
using CUBEGAMEAPI.Services;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class GameServerController : ControllerBase
    {
        private readonly IGameServerService _serverService;

        public GameServerController(IGameServerService serverService)
        {
            _serverService = serverService;
        }

        // POST: api/server/register
        [HttpPost("register")]
        public IActionResult Register([FromBody] GameServer request)
        {
            _serverService.RegisterOrUpdate(request);
            return Ok();
        }
        
    }
}