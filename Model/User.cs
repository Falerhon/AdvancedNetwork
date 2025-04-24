using System.ComponentModel.DataAnnotations;
namespace CUBEGAMEAPI.Models
{
    public class UserModel
{
    [Key]
    public int Id { get; set; }
    public string Username { get; set; }
    public string PasswordHash { get; set; }
}
}
