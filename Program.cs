using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.IdentityModel.Tokens;
using System.Text;
using CUBEGAMEAPI.Services;
using Microsoft.EntityFrameworkCore;
using CUBEGAMEAPI.Models;
using Microsoft.AspNetCore.Identity;
using Microsoft.OpenApi.Models;

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddDbContext<UserDb>(opt => opt.UseInMemoryDatabase("UserList"));
builder.Services.AddDatabaseDeveloperPageExceptionFilter();

builder.Services.AddEndpointsApiExplorer();
builder.Services.AddOpenApiDocument(config =>
{
    config.DocumentName = "UserAPI";
    config.Title = "UserAPI v1";
    config.Version = "v1";
});

builder.Services.AddControllers();
builder.Services.AddScoped<IAuthService, AuthService>();

//JWT config
builder.Services.AddAuthentication(options =>
{
    options.DefaultAuthenticateScheme = JwtBearerDefaults.AuthenticationScheme;
    options.DefaultChallengeScheme = JwtBearerDefaults.AuthenticationScheme;
})
.AddJwtBearer(options =>
{
    var key = Encoding.UTF8.GetBytes(builder.Configuration["Jwt:Key"]);
    options.TokenValidationParameters = new TokenValidationParameters
    {
        ValidateIssuer = true,
        ValidateAudience = true,
        ValidateLifetime = true,
        ValidateIssuerSigningKey = true,
        ValidIssuer = builder.Configuration["Jwt:Issuer"],
        ValidAudience = builder.Configuration["Jwt:Audience"],
        IssuerSigningKey = new SymmetricSecurityKey(key)
    };
});

//Adding services
builder.Services.AddScoped<IStatService, StatService>();
builder.Services.AddScoped<IAchievementService, AchievementService>();
builder.Services.AddScoped<IGameServerService, GameServerService>();
builder.Services.AddScoped<IGameServerAuthService, GameServerAuthService>();
builder.Services.AddScoped<IMatchmakingService, MatchmakingService>();

builder.Services.AddAuthorization();

builder.Services.AddSwaggerGen(c =>
{
    c.SwaggerDoc("v1", new OpenApiInfo { Title = "", Version = "v1" });

    // Add Bearer token security definition
    c.AddSecurityDefinition("Bearer", new OpenApiSecurityScheme
    {
        Name = "Authorization",
        Type = SecuritySchemeType.ApiKey,
        Scheme = "Bearer",
        BearerFormat = "JWT",
        In = ParameterLocation.Header,
        Description = "Enter 'Bearer' followed by your token."
    });

    c.AddSecurityRequirement(new OpenApiSecurityRequirement
    {
        {
            new OpenApiSecurityScheme
            {
                Reference = new OpenApiReference
                {
                    Type = ReferenceType.SecurityScheme,
                    Id = "Bearer"
                }
            },
            new string[] {}
        }
    });
});

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI(c =>
    {
        c.SwaggerEndpoint("/swagger/v1/swagger.json", "My API V1");
    });
}

app.UseHttpsRedirection();
app.UseAuthentication(); // Ensure authentication is enabled
app.UseAuthorization();

app.MapGet("/usersItems", async (UserDb db) =>
    await db.Users.ToListAsync());

app.MapGet("/useritems/{name}", async (string name, UserDb db) =>
    await db.Users.FindAsync(name)
        is UserModel user
            ? Results.Ok(user)
            : Results.NotFound());

app.MapPost("/useritems", async (UserModel user, UserDb db) =>
{
    PasswordHasher<UserModel> _hasher = new PasswordHasher<UserModel>();

    user.PasswordHash = _hasher.HashPassword(user, user.PasswordHash);
    
    db.Users.Add(user);
    PlayerStatsModel statsModel = new PlayerStatsModel();
    db.Stats.Add(statsModel);
    await db.SaveChangesAsync();

    return Results.Created($"/useritems/{user.Username}", user);
});

app.MapPut("/useritems/{id}", async (int id, UserModel inputUser, UserDb db) =>
{
    var user = await db.Users.FindAsync(id);

    if (user is null) return Results.NotFound();

    user.Username = inputUser.Username;
    user.PasswordHash = inputUser.PasswordHash;

    await db.SaveChangesAsync();

    return Results.NoContent();
});

app.MapDelete("/useritems/{id}", async (int id, UserDb db) =>
{
    if (await db.Users.FindAsync(id) is UserModel user)
    {
        db.Users.Remove(user);
        if (await db.Stats.FindAsync(id) is PlayerStatsModel statsModel)
        {
            db.Stats.Remove(statsModel);
        }
        
        await db.SaveChangesAsync();
        return Results.NoContent();
    }

    return Results.NotFound();
});


app.MapControllers();

app.Run();