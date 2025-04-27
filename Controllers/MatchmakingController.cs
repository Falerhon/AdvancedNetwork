using System.Security.Claims;
using CUBEGAMEAPI.Models;
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
        private readonly IGameServerService _gameServerService;
        private readonly UserDb _context;

        public MatchmakingController(IMatchmakingService matchmakingService,IGameServerService gameServerService, UserDb context)
        {
            _matchmakingService = matchmakingService;
            _gameServerService = gameServerService;
            _context = context;
        }

        [HttpPost("enqueue")]
        [Authorize]
        public IActionResult Enqueue()
        {
            var userId = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)!.Value);
            
            var stats = _context.Stats.FirstOrDefault(p => p.Id == userId);

            if (stats == null)
                return NotFound("Player had no stats");
            
            _matchmakingService.EnqueuePlayer(userId, CalculateScore(stats));

            //TODO : Check if we move this elsewhere
            var match = _matchmakingService.ProcessMatchmaking();
            
            if (match.Count > 0)
                AssignPlayersToGameServer(match);
            
            return Ok();
        }
        
        [HttpPost("Status")]
        [Authorize]
        public IActionResult CheckStatus()
        {
            var userId = int.Parse(User.FindFirst(ClaimTypes.NameIdentifier)!.Value);

            var lobby = _context.Lobby.FirstOrDefault(l => l.userIds.Contains(userId));

            if (lobby != null)
            {
                var server = _context.GameServers.FirstOrDefault(s => s.Id == lobby.ServerId);

                if (server != null)
                {
                    return Ok(server);
                }
            }
            
            return Ok("Looking for lobby");
        }
        
        private float CalculateScore(PlayerStatsModel stats)
        {
            float score = 0;
            if (stats.GamesPlayed <= 0)
            {
                score = 1;
            }
            else
            {
                score = ((stats.GamesWon * 10) / stats.GamesPlayed) + stats.CurrentWinStreak * 5;
            }
            return score;
        }
        
        //Create a new lobby and tells the matchmaking service to remove the players from the queue
        private void AssignPlayersToGameServer(List<int> userIds)
        {
            var Server = _context.GameServers
                .FirstOrDefault(s => s.CurrentPlayers < s.MaxPlayers && !s.IsOccupied);

            var s = _context.GameServers.Count();

            if (s <= 0)
            {
                return;
            }
            
            if(Server == null)
                return;
            
            _gameServerService.MarkAsOccupied(Server, true);
            
            LobbyModel lobby = new LobbyModel();
            lobby.userIds = userIds;
            lobby.ServerId = Server.Id;
            
            _context.Lobby.Add(lobby);

            _context.SaveChanges();
            
            _matchmakingService.RemoveFromMatchmaking(userIds);
        }
    }
}

