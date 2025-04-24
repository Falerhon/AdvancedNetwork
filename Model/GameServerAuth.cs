namespace CUBEGAMEAPI.Models
{
    public class GameServerAuth
    {
        public int Id { get; set; }
        public string ServerName { get; set; }
        public string PasswordHash { get; set; }
    }
}