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
        public IActionResult RecordWin() =>
            RecordAndRespond(() => _statsService.RecordWin(int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value)));
        
        [HttpPost("loss")]
        public IActionResult RecordLoss() =>
            RecordAndRespond(() => _statsService.RecordLoss(int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value)));

        [HttpPost("destroy")]
        public IActionResult RecordDestroy([FromQuery] int count = 1) =>
            RecordAndRespond(() => _statsService.RecordObjectDestroyed(int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value), count));

        private IActionResult RecordAndRespond(Action updateAction)
        {
            updateAction();
            var stats = _statsService.GetStats(int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value));
            return Ok(stats);
        }
    }
}