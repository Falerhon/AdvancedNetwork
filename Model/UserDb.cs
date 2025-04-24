using Microsoft.EntityFrameworkCore;

namespace CUBEGAMEAPI.Models
{
    public class UserDb : DbContext
    {
        public UserDb(DbContextOptions<UserDb> options)
            : base(options)
        {
        }

        public DbSet<UserModel> Users => Set<UserModel>();
        public DbSet<PlayerStatsModel> Stats => Set<PlayerStatsModel>();
        public DbSet<Achievement> Achievements => Set<Achievement>();
        public DbSet<UserAchievement> UserAchievements => Set<UserAchievement>();
        public DbSet<GameServer> GameServers => Set<GameServer>();
        public DbSet<GameServerAuth> GameServersAuth => Set<GameServerAuth>();
        public DbSet<LobbyModel> Lobby => Set<LobbyModel>();
    }
}