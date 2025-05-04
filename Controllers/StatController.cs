using System.Security.Claims;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using CUBEGAMEAPI.Services;

namespace CUBEGAMEAPI.Controllers
{
    
    public class PlayerCubeDestroyed
    {
        public int PlayerId { get; set; }
        public int Cubes { get; set; }
    }
    
    [ApiController]
    [Route("api/[controller]")]
    [Authorize]
    public class StatsController : ControllerBase
    {
        private readonly IStatService _statsService;

        public StatsController(IStatService statService)
        {
            _statsService = statService;
        }

        [HttpGet("Info")]
        public IActionResult GetStats()
        {
            int id = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value);
            var stats = _statsService.GetStats(id);

            if (stats == null)
                return NotFound();

            return Ok(stats);
        }
        
        [HttpPost("InfoSpecific")]
        public IActionResult GetStatsSpecifix([FromBody] int playerid)
        {
            var stats = _statsService.GetStats(playerid);

            if (stats == null)
                return NotFound();

            return Ok(stats);
        }
        
        [HttpPost("win")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordWin([FromBody] int id) =>
            RecordAndRespond(id, () => _statsService.RecordWin(id));
        
        [HttpPost("loss")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordLoss([FromBody] int id) =>
            RecordAndRespond(id, () => _statsService.RecordLoss(id));

        [HttpPost("destroy")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordDestroy([FromBody] PlayerCubeDestroyed pcd) =>
            RecordAndRespond(pcd.PlayerId, () => _statsService.RecordObjectDestroyed(pcd.PlayerId, pcd.Cubes));

        private IActionResult RecordAndRespond(int id, Action updateAction)
        {
            updateAction();
            var stats = _statsService.GetStats(id);
            return Ok(stats);
        }
    }
}