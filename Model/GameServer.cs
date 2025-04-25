using System.ComponentModel.DataAnnotations;

namespace CUBEGAMEAPI.Models
{
    public class GameServer
    {
        [Key]
        public int Id { get; set; }

        public string IP { get; set; }
        public int Port { get; set; }

        public int MaxPlayers { get; set; }
        public int CurrentPlayers { get; set; }

        public bool IsOnline { get; set; }
        public bool IsOccupied { get; set; }
        public DateTime LastHeartbeat { get; set; }
    }
}