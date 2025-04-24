using System.Security.Claims;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using CUBEGAMEAPI.Models;
using CUBEGAMEAPI.Services;

namespace CUBEGAMEAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class AchievementsController : ControllerBase
    {
    
        private readonly UserDb _context;
        private readonly IAchievementService _achievementService;

        public AchievementsController(UserDb context, IAchievementService achievementService)
        {
            _context = context;
            _achievementService = achievementService;
        }

        // GET: api/achievements/me
        [Authorize]
        [HttpGet("MyAchivements")]
        public IActionResult GetMyAchievements()
        {
            int id = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)?.Value);
            var stats = _context.Stats.FirstOrDefault(s => s.Id == id);
            
            //Updating the achivements
            _achievementService.CheckForNewAchievements(id, stats);

            var unlocked = _context.UserAchievements
                .Where(ua => ua.UserId == id && ua.IsUnlocked)
                .Include(ua => ua.AchievementId)
                .Select(ua => new
                {
                    ua.Achievement.Name,
                    ua.Achievement.Description,
                })
                .ToList();

            return Ok(unlocked);
        }

    }
}
