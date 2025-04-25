using Microsoft.AspNetCore.Mvc;
using CUBEGAMEAPI.Models;
using CUBEGAMEAPI.Services;
using Microsoft.AspNetCore.Authorization;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    [Authorize(Roles = "Server")]
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
        
        // POST: api/server/free
        [HttpPost("free")]
        public IActionResult Free([FromBody] GameServer request)
        {
            _serverService.MarkAsOccupied(request, false);
            
            _serverService.RemoveLobby(request.Id);
            
            return Ok();
        }
        
    }
}