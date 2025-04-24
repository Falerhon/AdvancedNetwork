using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace CUBEGAMEAPI.Models
{

//The actual achievement
    public class Achievement
    {
        [Key]
        public int Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string StatToTrack { get; set; } // e.g., "ObjectsDestroyed", "GamesWon"
        public int Threshold { get; set; } // e.g., 50 objects
    }

//Link between users and their owned achivements
    public class UserAchievement
    {
        [Key]
        public int Id { get; set; }
        public int UserId { get; set; }
        [ForeignKey("Achivement")]
        public int AchievementId { get; set; }
        public bool IsUnlocked { get; set; }
        
        public Achievement Achievement { get; set; }
    }

}