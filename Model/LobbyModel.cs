using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace CUBEGAMEAPI.Models
{
    public class LobbyModel
    {
        [Key]
        public int Id { get; set; }

        [ForeignKey("Player")] 
        public List<int> userIds { get; set; }

        [ForeignKey("GameServer")] 
        public int ServerId { get; set; }
    }
}

