using System.Security.Claims;
using CUBEGAMEAPI.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/matchmaking")]
    public class MatchmakingController : ControllerBase
    {
        private readonly IMatchmakingService _matchmakingService;

        public MatchmakingController(IMatchmakingService matchmakingService)
        {
            _matchmakingService = matchmakingService;
        }

        [HttpPost("enqueue")]
        [Authorize]
        public IActionResult Enqueue()
        {
            var userId = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)!.Value);
            _matchmakingService.EnqueuePlayer(userId);
            return Ok();
        }
    }
}

