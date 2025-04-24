using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace CUBEGAMEAPI.Models
{
    public class PlayerStatsModel
    {
        [Key, ForeignKey("Player")]
        public int Id { get; set; }

        public int GamesWon { get; set; }
        
        public int GamesPlayed { get; set; }

        public int ObjectsDestroyed { get; set; }

        public int CurrentWinStreak { get; set; }
    }
}