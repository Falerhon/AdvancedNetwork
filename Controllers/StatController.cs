using System.Security.Claims;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using CUBEGAMEAPI.Services;

namespace CUBEGAMEAPI.Controllers
{
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
        
        [HttpPost("win")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordWin([FromQuery] int id) =>
            RecordAndRespond(id, () => _statsService.RecordWin(id));
        
        [HttpPost("loss")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordLoss([FromQuery] int id) =>
            RecordAndRespond(id, () => _statsService.RecordLoss(id));

        [HttpPost("destroy")]
        [Authorize(Roles = "Server")]
        public IActionResult RecordDestroy([FromQuery] int id,[FromQuery] int count = 1) =>
            RecordAndRespond(id, () => _statsService.RecordObjectDestroyed(id, count));

        private IActionResult RecordAndRespond(int id, Action updateAction)
        {
            updateAction();
            var stats = _statsService.GetStats(id);
            return Ok(stats);
        }
    }
}