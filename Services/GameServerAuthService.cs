using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;
using CUBEGAMEAPI.Models;
using Microsoft.AspNetCore.Identity;
using Microsoft.IdentityModel.Tokens;

namespace CUBEGAMEAPI.Services
{
    public class GameServerAuthService : IGameServerAuthService
    {
        private readonly UserDb _context;
        private readonly IConfiguration _config;
        private readonly PasswordHasher<GameServerAuth> _hasher;

        public GameServerAuthService(UserDb context, IConfiguration config)
        {
            _context = context;
            _config = config;
            _hasher = new PasswordHasher<GameServerAuth>();
        }

        public string Authenticate(string serverName, string password)
        {
            var server = _context.GameServersAuth.FirstOrDefault(s => s.ServerName == serverName);
            if (server == null) return null;

            var result = _hasher.VerifyHashedPassword(server, server.PasswordHash, password);
            if (result == PasswordVerificationResult.Failed)
                return null;

            var claims = new[]
            {
                new Claim(ClaimTypes.NameIdentifier, server.Id.ToString()),
                new Claim(ClaimTypes.Name, server.ServerName),
                new Claim(ClaimTypes.Role, "Server")
            };

            var key = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_config["Jwt:Key"]));
            var creds = new SigningCredentials(key, SecurityAlgorithms.HmacSha256);

            var token = new JwtSecurityToken(
                issuer: _config["Jwt:Issuer"],
                audience: _config["Jwt:Audience"],
                claims: claims,
                expires: DateTime.UtcNow.AddHours(12),
                signingCredentials: creds
            );

            return new JwtSecurityTokenHandler().WriteToken(token);
        }
    }
}

