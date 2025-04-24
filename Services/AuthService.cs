using System;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;
using Microsoft.IdentityModel.Tokens;
using CUBEGAMEAPI.Models;
using Microsoft.AspNetCore.Identity;

namespace CUBEGAMEAPI.Services
{
    public class AuthService : IAuthService
    {
        private readonly UserDb _context;
        private readonly IConfiguration _configuration;
        private readonly PasswordHasher<UserModel> _hasher;

        public AuthService(UserDb context, IConfiguration configuration)
        {
            _context = context;
            _configuration = configuration;
            _hasher = new PasswordHasher<UserModel>();
        }

        public string Authenticate(UserModel login)
        {
            var user = _context.Users.FirstOrDefault(s => s.Username == login.Username);
            if (user == null) return null;

            var result = _hasher.VerifyHashedPassword(user, user.PasswordHash, login.PasswordHash);
            if (result == PasswordVerificationResult.Failed)
                return null;

            var claims = new[]
            {
                new Claim(ClaimTypes.NameIdentifier, user.Id.ToString())
            };

            var key = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(_configuration["Jwt:Key"]));
            var creds = new SigningCredentials(key, SecurityAlgorithms.HmacSha256);

            var token = new JwtSecurityToken(
                issuer: _configuration["Jwt:Issuer"],
                audience: _configuration["Jwt:Audience"],
                claims: claims,
                expires: DateTime.UtcNow.AddHours(1),
                signingCredentials: creds);

            return new JwtSecurityTokenHandler().WriteToken(token);
        }
    }
}