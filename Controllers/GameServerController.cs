using System.Security.Claims;
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
        
        // POST: api/server/remove
        [HttpPost("remove")]
        public IActionResult Remove([FromBody] GameServer request)
        {
            _serverService.RemoveServer(request);
            return Ok();
        }
        
        // POST: api/server/free
        [HttpPost("free")]
        public IActionResult Free()
        {
            int id = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value);
            
            _serverService.MarkAsOccupied(id, false);
            
            _serverService.RemoveLobby(id);
            
            return Ok();
        }
    }
}