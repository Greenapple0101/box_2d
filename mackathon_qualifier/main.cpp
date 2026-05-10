#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct Enemy
{
    Vector2 pos;
    float radius;
    float speed;
    float hp;
    bool boss;
};

struct Bullet
{
    Vector2 pos;
    Vector2 vel;
    float life;
    float damage;
};

enum UpgradeType
{
    UPGRADE_DAMAGE = 0,
    UPGRADE_FIRE_RATE = 1,
    UPGRADE_MOVE_SPEED = 2
};

static float Length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 Normalize(Vector2 v)
{
    float len = Length(v);
    if (len <= 0.0001f) return {0.0f, 0.0f};
    return {v.x / len, v.y / len};
}

int main()
{
    const int screenW = 1280;
    const int screenH = 720;
    const float gameDuration = 180.0f;
    InitWindow(screenW, screenH, "Mackathon Qualifier - Maple Tactics Prototype");
    SetTargetFPS(60);

    Vector2 player = {(float)screenW * 0.5f, (float)screenH * 0.5f};
    float playerRadius = 16.0f;
    float playerSpeed = 220.0f;
    float playerHp = 100.0f;

    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;

    float shotInterval = 0.35f;
    float shotTimer = 0.0f;
    float bulletSpeed = 520.0f;
    float bulletDamage = 18.0f;

    int wave = 1;
    int score = 0;
    float gameTimer = 0.0f;
    bool gameOver = false;
    bool gameClear = false;
    bool bossSpawned = false;

    bool selectingUpgrade = false;
    UpgradeType options[3] = {UPGRADE_DAMAGE, UPGRADE_FIRE_RATE, UPGRADE_MOVE_SPEED};

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (!gameOver && !gameClear)
        {
            if (!selectingUpgrade)
            {
                gameTimer += dt;
            }

            if (gameTimer >= gameDuration)
            {
                gameClear = true;
            }

            if (!selectingUpgrade)
            {
                Vector2 move = {0.0f, 0.0f};
                if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1.0f;
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1.0f;
                if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
                if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;
                move = Normalize(move);
                player.x += move.x * playerSpeed * dt;
                player.y += move.y * playerSpeed * dt;
                player.x = std::clamp(player.x, playerRadius, (float)screenW - playerRadius);
                player.y = std::clamp(player.y, playerRadius, (float)screenH - playerRadius);
            }

            if (!gameClear)
            {
                int targetWave = 1 + (int)(gameTimer / 25.0f);
                if (targetWave > wave)
                {
                    wave = targetWave;
                    selectingUpgrade = true;
                    options[0] = (UpgradeType)GetRandomValue(0, 2);
                    options[1] = (UpgradeType)GetRandomValue(0, 2);
                    options[2] = (UpgradeType)GetRandomValue(0, 2);
                }

                if (selectingUpgrade)
                {
                    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1))
                    {
                        if (options[0] == UPGRADE_DAMAGE) bulletDamage += 5.0f;
                        if (options[0] == UPGRADE_FIRE_RATE) shotInterval = std::max(0.12f, shotInterval - 0.05f);
                        if (options[0] == UPGRADE_MOVE_SPEED) playerSpeed += 22.0f;
                        selectingUpgrade = false;
                    }
                    else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
                    {
                        if (options[1] == UPGRADE_DAMAGE) bulletDamage += 5.0f;
                        if (options[1] == UPGRADE_FIRE_RATE) shotInterval = std::max(0.12f, shotInterval - 0.05f);
                        if (options[1] == UPGRADE_MOVE_SPEED) playerSpeed += 22.0f;
                        selectingUpgrade = false;
                    }
                    else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3))
                    {
                        if (options[2] == UPGRADE_DAMAGE) bulletDamage += 5.0f;
                        if (options[2] == UPGRADE_FIRE_RATE) shotInterval = std::max(0.12f, shotInterval - 0.05f);
                        if (options[2] == UPGRADE_MOVE_SPEED) playerSpeed += 22.0f;
                        selectingUpgrade = false;
                    }
                }
                else
                {
                    shotTimer += dt;
                    if (shotTimer >= shotInterval && !enemies.empty())
                    {
                        shotTimer = 0.0f;
                        int closest = 0;
                        float closestDist = 999999.0f;
                        for (int i = 0; i < (int)enemies.size(); ++i)
                        {
                            Vector2 toEnemy = {enemies[i].pos.x - player.x, enemies[i].pos.y - player.y};
                            float d = Length(toEnemy);
                            if (d < closestDist)
                            {
                                closestDist = d;
                                closest = i;
                            }
                        }
                        Vector2 dir = Normalize({enemies[closest].pos.x - player.x, enemies[closest].pos.y - player.y});
                        bullets.push_back({player, {dir.x * bulletSpeed, dir.y * bulletSpeed}, 1.8f, bulletDamage});
                    }

                    float spawnChance = dt * (0.95f + wave * 0.15f);
                    if (GetRandomValue(0, 10000) < (int)(spawnChance * 10000.0f))
                    {
                        int edge = GetRandomValue(0, 3);
                        Vector2 p = {0.0f, 0.0f};
                        if (edge == 0) p = {(float)GetRandomValue(0, screenW), -20.0f};
                        if (edge == 1) p = {(float)screenW + 20.0f, (float)GetRandomValue(0, screenH)};
                        if (edge == 2) p = {(float)GetRandomValue(0, screenW), (float)screenH + 20.0f};
                        if (edge == 3) p = {-20.0f, (float)GetRandomValue(0, screenH)};
                        float hp = 20.0f + wave * 4.0f;
                        float speed = 45.0f + wave * 4.5f;
                        enemies.push_back({p, 12.0f, speed, hp, false});
                    }

                    if (!bossSpawned && gameTimer > 150.0f)
                    {
                        bossSpawned = true;
                        enemies.push_back({{(float)screenW * 0.5f, -80.0f}, 34.0f, 70.0f, 600.0f, true});
                    }
                }
            }

            for (Bullet &b : bullets)
            {
                b.pos.x += b.vel.x * dt;
                b.pos.y += b.vel.y * dt;
                b.life -= dt;
            }
            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet &b)
                               { return b.life <= 0.0f || b.pos.x < -40 || b.pos.x > screenW + 40 || b.pos.y < -40 || b.pos.y > screenH + 40; }),
                bullets.end());

            for (Enemy &e : enemies)
            {
                Vector2 toPlayer = {player.x - e.pos.x, player.y - e.pos.y};
                Vector2 dir = Normalize(toPlayer);
                e.pos.x += dir.x * e.speed * dt;
                e.pos.y += dir.y * e.speed * dt;

                float d = Length({player.x - e.pos.x, player.y - e.pos.y});
                if (d < playerRadius + e.radius)
                {
                    playerHp -= e.boss ? 20.0f * dt : 11.0f * dt;
                }
            }

            for (Enemy &e : enemies)
            {
                for (Bullet &b : bullets)
                {
                    float d = Length({e.pos.x - b.pos.x, e.pos.y - b.pos.y});
                    if (d < e.radius + 4.0f && b.life > 0.0f)
                    {
                        e.hp -= b.damage;
                        b.life = -1.0f;
                    }
                }
            }

            int before = (int)enemies.size();
            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(), [](const Enemy &e)
                               { return e.hp <= 0.0f; }),
                enemies.end());
            int killed = before - (int)enemies.size();
            score += killed * 10;

            if (playerHp <= 0.0f)
            {
                gameOver = true;
            }
        }

        BeginDrawing();
        ClearBackground(Color{21, 25, 41, 255});

        for (const Bullet &b : bullets) DrawCircleV(b.pos, 4.0f, YELLOW);
        for (const Enemy &e : enemies)
        {
            Color c = e.boss ? Color{189, 63, 77, 255} : Color{120, 155, 255, 255};
            DrawCircleV(e.pos, e.radius, c);
            if (e.boss)
            {
                DrawCircleLines((int)e.pos.x, (int)e.pos.y, e.radius + 4.0f, ORANGE);
            }
        }
        DrawCircleV(player, playerRadius, Color{100, 255, 180, 255});

        DrawText(TextFormat("WAVE %d", wave), 24, 20, 28, RAYWHITE);
        DrawText(TextFormat("SCORE %d", score), 24, 52, 24, LIGHTGRAY);
        DrawText(TextFormat("HP %.0f", playerHp), 24, 82, 24, Color{255, 120, 120, 255});
        DrawText(TextFormat("TIME %.0f / 180", gameTimer), 24, 112, 22, GOLD);
        DrawText("Move: WASD / Arrow | Auto Attack", 24, screenH - 34, 20, GRAY);

        if (selectingUpgrade)
        {
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.7f));
            DrawText("WAVE UP! PICK 1 UPGRADE", screenW / 2 - 220, 160, 34, RAYWHITE);
            auto toName = [](UpgradeType t) -> const char * {
                if (t == UPGRADE_DAMAGE) return "Damage +5";
                if (t == UPGRADE_FIRE_RATE) return "Attack Speed +";
                return "Move Speed +";
            };
            DrawText(TextFormat("1) %s", toName(options[0])), screenW / 2 - 160, 260, 30, GREEN);
            DrawText(TextFormat("2) %s", toName(options[1])), screenW / 2 - 160, 310, 30, SKYBLUE);
            DrawText(TextFormat("3) %s", toName(options[2])), screenW / 2 - 160, 360, 30, ORANGE);
        }

        if (gameOver)
        {
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.65f));
            DrawText("DEFEAT", screenW / 2 - 110, screenH / 2 - 70, 70, RED);
            DrawText("Press R to restart", screenW / 2 - 140, screenH / 2 + 20, 30, RAYWHITE);
            if (IsKeyPressed(KEY_R))
            {
                player = {(float)screenW * 0.5f, (float)screenH * 0.5f};
                playerHp = 100.0f;
                playerSpeed = 220.0f;
                bulletDamage = 18.0f;
                shotInterval = 0.35f;
                shotTimer = 0.0f;
                gameTimer = 0.0f;
                wave = 1;
                score = 0;
                gameOver = false;
                gameClear = false;
                bossSpawned = false;
                selectingUpgrade = false;
                enemies.clear();
                bullets.clear();
            }
        }

        if (gameClear)
        {
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.65f));
            DrawText("MISSION CLEAR", screenW / 2 - 220, screenH / 2 - 70, 64, GREEN);
            DrawText(TextFormat("Final Score: %d", score), screenW / 2 - 130, screenH / 2 + 10, 34, RAYWHITE);
            DrawText("Press R to play again", screenW / 2 - 160, screenH / 2 + 60, 28, LIGHTGRAY);
            if (IsKeyPressed(KEY_R))
            {
                player = {(float)screenW * 0.5f, (float)screenH * 0.5f};
                playerHp = 100.0f;
                playerSpeed = 220.0f;
                bulletDamage = 18.0f;
                shotInterval = 0.35f;
                shotTimer = 0.0f;
                gameTimer = 0.0f;
                wave = 1;
                score = 0;
                gameOver = false;
                gameClear = false;
                bossSpawned = false;
                selectingUpgrade = false;
                enemies.clear();
                bullets.clear();
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
