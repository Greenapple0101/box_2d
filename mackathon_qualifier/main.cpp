#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <vector>

struct Platform
{
    Rectangle rect;
    float bounciness;
    float boost;
    bool wideGround;
    float flex = 0.0f;
    float flexVel = 0.0f;
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

static Rectangle HitboxCentered(float cx, float cy, float hw, float hh)
{
    return {cx - hw, cy - hh, hw * 2.0f, hh * 2.0f};
}

static void ResolveSolidX(float &cx, float &cy, float hw, float hh, float &vx, const std::vector<Platform> &plats)
{
    Rectangle pr = HitboxCentered(cx, cy, hw, hh);
    for (const Platform &p : plats)
    {
        const Rectangle &s = p.rect;
        if (!CheckCollisionRecs(pr, s))
            continue;
        if (vx > 0.0f)
            cx = s.x - hw - 0.02f;
        else if (vx < 0.0f)
            cx = s.x + s.width + hw + 0.02f;
        vx = 0.0f;
        pr = HitboxCentered(cx, cy, hw, hh);
    }
}

static void ResolveSolidY(float &cx, float &cy, float hw, float hh, float &vy, bool &grounded, std::vector<Platform> &plats,
                          float vyBeforeMove)
{
    grounded = false;
    Rectangle pr = HitboxCentered(cx, cy, hw, hh);
    for (size_t pi = 0; pi < plats.size(); ++pi)
    {
        Platform &p = plats[pi];
        const Rectangle &s = p.rect;
        if (!CheckCollisionRecs(pr, s))
            continue;
        if (vyBeforeMove > 0.0f)
        {
            cy = s.y - hh - 0.02f;
            float impact = std::max(vyBeforeMove, 0.0f);
            const float softLand = 95.0f;
            float centerX = s.x + s.width * 0.5f;
            float edge = std::max(s.width * 0.5f, 1.0f);
            float landShape = 1.0f - fabsf(cx - centerX) / edge * 0.42f;
            landShape = std::clamp(landShape, 0.5f, 1.0f);
            float wobble = (0.55f + impact * 0.032f) * landShape * (0.65f + p.bounciness * 1.1f);
            if (p.wideGround)
                wobble *= 0.5f;
            p.flexVel += wobble;
            if (p.bounciness > 0.02f && impact > softLand)
            {
                vy = -(impact * p.bounciness + p.boost);
                vy = std::max(vy, -1150.0f);
            }
            else
            {
                vy = 0.0f;
                grounded = true;
            }
        }
        else if (vyBeforeMove < 0.0f)
        {
            cy = s.y + s.height + hh + 0.02f;
            vy = 0.0f;
        }
        pr = HitboxCentered(cx, cy, hw, hh);
    }
}

static void DrawPlayerCharacter(Vector2 center, Vector2 lookDir, float baseR)
{
    float faceX = lookDir.x;
    if (fabsf(faceX) < 0.15f)
        faceX = (lookDir.x >= 0.0f) ? 1.0f : -1.0f;
    Vector2 flatLook = Normalize({faceX, lookDir.y * 0.35f});

    Color hood = {72, 158, 255, 255};
    Color hoodShadow = {52, 118, 205, 255};
    Color skin = {255, 210, 190, 255};

    DrawEllipse((int)center.x, (int)(center.y + baseR * 0.62f), baseR * 0.95f, baseR * 0.28f, Fade(BLACK, 0.3f));

    DrawCircleV({center.x, center.y + baseR * 0.1f}, baseR * 0.92f, hoodShadow);
    DrawCircleV({center.x, center.y + baseR * 0.02f}, baseR * 0.85f, hood);

    Vector2 head = {center.x + flatLook.x * baseR * 0.12f, center.y - baseR * 0.72f + flatLook.y * baseR * 0.08f};
    DrawCircleV({head.x, head.y - baseR * 0.22f}, baseR * 0.42f, hood);
    DrawCircleV(head, baseR * 0.58f, skin);

    Vector2 eyeShift = {flatLook.x * baseR * 0.1f, flatLook.y * baseR * 0.1f};
    DrawCircleV({head.x - baseR * 0.16f + eyeShift.x, head.y - baseR * 0.02f + eyeShift.y}, baseR * 0.11f, BLACK);
    DrawCircleV({head.x + baseR * 0.16f + eyeShift.x, head.y - baseR * 0.02f + eyeShift.y}, baseR * 0.11f, BLACK);
}

static void DrawBentLayerStrip(const Rectangle &r, float flexVis, float amp, float y0, float y1, Color c)
{
    if (y1 <= y0)
        return;
    const int N = 14;
    float step = r.width / (float)N;
    auto sag = [&](float u, float y) {
        float t = std::clamp((y - r.y) / std::max(r.height, 1.0f), 0.0f, 1.0f);
        float bend = 0.52f + 0.48f * t;
        return sinf(u * PI) * amp * flexVis * bend;
    };
    for (int i = 0; i < N; ++i)
    {
        float u0 = (float)i / (float)N;
        float u1 = (float)(i + 1) / (float)N;
        float x0 = r.x + (float)i * step;
        float x1 = r.x + (float)(i + 1) * step;
        float s00 = sag(u0, y0);
        float s01 = sag(u1, y0);
        float s10 = sag(u0, y1);
        float s11 = sag(u1, y1);
        Vector2 v0 = {x0, y0 + s00};
        Vector2 v1 = {x1, y0 + s01};
        Vector2 v2 = {x1, y1 + s11};
        Vector2 v3 = {x0, y1 + s10};
        DrawTriangle(v0, v1, v2, c);
        DrawTriangle(v0, v2, v3, c);
    }
}

static void DrawGrassPlatform(const Platform &p)
{
    const Rectangle &r = p.rect;
    float e = p.bounciness;
    bool tramp = e > 0.65f;

    Color dirt = {92, 62, 28, 255};
    Color grassDark = tramp ? Color{30, 118, 44, 255} : Color{24, 108, 38, 255};
    Color grassMid = tramp ? Color{48, 162, 62, 255} : Color{40, 148, 56, 255};
    Color grassTop = tramp ? Color{78, 210, 88, 255} : Color{58, 188, 72, 255};
    Color grassBright = tramp ? Color{110, 235, 102, 255} : Color{88, 218, 86, 255};

    float flexVis = p.flex * (p.wideGround ? 0.55f : 1.0f);
    float bendAmp = tramp ? 34.0f : 24.0f;

    float dirtH = p.wideGround ? 22.0f : 10.0f;
    float yDirtTop = r.y + r.height - dirtH;
    DrawBentLayerStrip(r, flexVis, bendAmp, yDirtTop, r.y + r.height, dirt);
    DrawBentLayerStrip(r, flexVis, bendAmp, r.y + 6.0f, yDirtTop, grassDark);
    DrawBentLayerStrip(r, flexVis, bendAmp, r.y + 4.0f, yDirtTop - 2.0f, grassMid);

    float stripH = std::fmax(4.0f, r.height * 0.22f);
    DrawBentLayerStrip(r, flexVis, bendAmp, r.y, r.y + stripH, grassTop);
    DrawBentLayerStrip(r, flexVis, bendAmp, r.y, r.y + 3.0f, grassBright);

    int blades = (int)(r.width / 14.0f);
    for (int i = 0; i < blades; ++i)
    {
        float t = (blades <= 1) ? 0.5f : (float)i / (float)(blades - 1);
        float u = (r.width > 1.0f) ? (6.0f + t * (r.width - 12.0f)) / r.width : 0.5f;
        int bx = (int)(r.x + 6 + t * (r.width - 12));
        float yGrass = r.y + stripH * 0.35f;
        float depth = std::clamp((yGrass - r.y) / std::max(r.height, 1.0f), 0.0f, 1.0f);
        float s = sinf(u * PI) * bendAmp * flexVis * (0.52f + 0.48f * depth);
        int by = (int)(r.y + s);
        int h = 3 + (int)(((bx * 7919 + i * 104729) % 4 + 4) % 4) + (tramp ? 2 : 0);
        Color blade = tramp ? Color{130, 245, 115, 255} : Color{98, 228, 95, 255};
        DrawLine(bx, by + 2, bx + (i % 2 == 0 ? -1 : 1), by - h, blade);
    }

    Color rim = {16, 92, 34, 255};
    DrawRectangleLinesEx(r, 2, rim);
    if (tramp)
    {
        float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 6.0f);
        unsigned char g = (unsigned char)(200 + (int)(35 * pulse));
        Color pulseCol = {60, g, 55, 255};
        DrawRectangleLinesEx(Rectangle{r.x - 2, r.y - 2, r.width + 4, r.height + 4}, 2, pulseCol);
    }
}

static void DrawCloudPuff(float cx, float cy, float scale, Color tint)
{
    float s = scale;
    DrawCircleV({cx, cy}, 28 * s, tint);
    DrawCircleV({cx + 26 * s, cy + 4 * s}, 22 * s, tint);
    DrawCircleV({cx - 24 * s, cy + 6 * s}, 20 * s, tint);
    DrawCircleV({cx + 12 * s, cy - 14 * s}, 18 * s, tint);
}

static void DrawSkyAndCloudsWorld(const Camera2D &cam, int screenW, int screenH)
{
    Vector2 a = GetScreenToWorld2D({0.0f, 0.0f}, cam);
    Vector2 b = GetScreenToWorld2D({(float)screenW, (float)screenH}, cam);
    float topY = std::min(a.y, b.y) - 400.0f;
    float botY = std::max(a.y, b.y) + 600.0f;
    int bands = 14;
    for (int i = 0; i < bands; ++i)
    {
        float t = (float)i / (float)(bands - 1);
        float y0 = topY + (botY - topY) * ((float)i / (float)bands);
        float y1 = topY + (botY - topY) * ((float)(i + 1) / (float)bands);
        Color c = Color{
            (unsigned char)(100 + (int)(40 * t)),
            (unsigned char)(170 + (int)(50 * t)),
            (unsigned char)(235 - (int)(25 * t)),
            255};
        DrawRectangle(-2000, (int)y0, screenW + 4000, (int)(y1 - y0) + 2, c);
    }

    float tsec = (float)GetTime();
    for (int i = -3; i < 28; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            float wy = cam.target.y + (float)i * 190.0f + (float)j * 37.0f;
            float wx = fmodf((float)(i * 241 + j * 173) + tsec * 18.0f + cam.target.y * 0.04f, (float)screenW + 500.0f) - 150.0f;
            wx += (float)(j - 2) * 180.0f;
            Color cloud = Fade(WHITE, 0.28f + (float)(j % 3) * 0.08f);
            DrawCloudPuff(wx, wy, 0.9f + 0.08f * (float)(j % 2), cloud);
        }
    }
}

static void AddStarterPlatforms(std::vector<Platform> &out, int screenW, float groundY)
{
    float y = groundY - 48.0f;
    float anchors[] = {screenW * 0.38f, screenW * 0.58f, screenW * 0.45f, screenW * 0.62f, screenW * 0.4f, screenW * 0.55f};
    for (float anchor : anchors)
    {
        float w = 260.0f;
        float x = anchor - w * 0.5f;
        x = std::clamp(x, 40.0f, (float)screenW - w - 40.0f);
        out.push_back({{x, y, w, 28.0f}, 0.34f, 28.0f, false});
        y -= (float)GetRandomValue(108, 138);
    }
}

static float AppendPlatformsUp(std::vector<Platform> &out, int screenW, float fromY, float toY, float &anchorX)
{
    float y = fromY;
    float minY = fromY;
    while (y > toY)
    {
        y -= (float)GetRandomValue(105, 148);
        float w = (float)GetRandomValue(220, 300);
        float step = (float)GetRandomValue(-168, 168);
        anchorX = std::clamp(anchorX + step, w * 0.5f + 44.0f, (float)screenW - w * 0.5f - 44.0f);
        float x = anchorX - w * 0.5f;
        bool trampoline = GetRandomValue(0, 100) < 22;
        float b = trampoline ? 0.82f + (float)GetRandomValue(0, 8) / 100.0f : 0.32f + (float)GetRandomValue(0, 20) / 100.0f;
        float boost = trampoline ? 200.0f + (float)GetRandomValue(0, 80) : 22.0f + (float)GetRandomValue(0, 35);
        float h = trampoline ? 30.0f : 26.0f;
        out.push_back({{x, y, w, h}, b, boost, false});
        minY = std::min(minY, y);
    }
    return minY;
}

int main()
{
    const int screenW = 1280;
    const int screenH = 720;

    const float gravity = 1980.0f;
    const float jumpSpeed = 755.0f;
    const float playerHw = 12.0f;
    const float playerHh = 20.0f;
    const float coyoteTime = 0.11f;
    const float jumpBufferTime = 0.12f;

    const float groundY = 7080.0f;
    const float groundH = 90.0f;

    InitWindow(screenW, screenH, "Bouncy Climb - Max Height");
    SetTargetFPS(60);

    std::vector<Platform> platforms;
    platforms.push_back({{0.0f, groundY, (float)screenW, groundH}, 0.08f, 0.0f, true});
    AddStarterPlatforms(platforms, screenW, groundY);
    float anchorX = platforms.back().rect.x + platforms.back().rect.width * 0.5f;
    float genTop = platforms.back().rect.y;
    genTop = AppendPlatformsUp(platforms, screenW, genTop - 16.0f, genTop - 2400.0f, anchorX);

    Vector2 player = {screenW * 0.5f, groundY - playerHh - 1.0f};
    Vector2 pVel = {0.0f, 0.0f};
    Vector2 playerLook = {1.0f, -0.35f};
    float playerRadius = 16.0f;
    float runAccel = 3400.0f;
    float maxRunSpeed = 300.0f;
    float timeSinceGrounded = 0.0f;
    float jumpBuffered = 0.0f;

    Camera2D cam = {};
    cam.offset = {(float)screenW * 0.5f, (float)screenH * 0.52f};
    cam.target = player;
    cam.zoom = 1.0f;
    cam.rotation = 0.0f;

    float bestHeightPx = 0.0f;
    bool gameOver = false;

    auto feetY = [&]() { return player.y + playerHh; };

    auto resetRun = [&]() {
        platforms.clear();
        platforms.push_back({{0.0f, groundY, (float)screenW, groundH}, 0.08f, 0.0f, true});
        AddStarterPlatforms(platforms, screenW, groundY);
        anchorX = platforms.back().rect.x + platforms.back().rect.width * 0.5f;
        genTop = platforms.back().rect.y;
        genTop = AppendPlatformsUp(platforms, screenW, genTop - 16.0f, genTop - 2400.0f, anchorX);
        player = {screenW * 0.5f, groundY - playerHh - 1.0f};
        pVel = {0.0f, 0.0f};
        playerLook = {1.0f, -0.4f};
        timeSinceGrounded = 0.0f;
        jumpBuffered = 0.0f;
        cam.target = player;
        bestHeightPx = 0.0f;
        gameOver = false;
        SetRandomSeed((unsigned int)(GetTime() * 1000.0));
    };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (!gameOver)
        {
            float inputX = 0.0f;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) inputX -= 1.0f;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) inputX += 1.0f;

            bool wantJump = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
            if (wantJump)
                jumpBuffered = jumpBufferTime;

            bool groundedPrev = timeSinceGrounded < coyoteTime;
            if (jumpBuffered > 0.0f && groundedPrev)
            {
                pVel.y = -jumpSpeed;
                jumpBuffered = 0.0f;
                timeSinceGrounded = coyoteTime + 1.0f;
            }

            float control = groundedPrev ? 1.0f : 0.58f;
            float target = inputX * maxRunSpeed;
            if (inputX > 0.0f && pVel.x < target)
                pVel.x = std::min(pVel.x + runAccel * control * dt, target);
            else if (inputX < 0.0f && pVel.x > target)
                pVel.x = std::max(pVel.x - runAccel * control * dt, target);
            else if (fabsf(inputX) < 0.01f)
            {
                float fric = groundedPrev ? 2600.0f : 520.0f;
                if (pVel.x > 0.0f)
                    pVel.x = std::max(0.0f, pVel.x - fric * dt);
                else if (pVel.x < 0.0f)
                    pVel.x = std::min(0.0f, pVel.x + fric * dt);
            }

            pVel.y += gravity * dt;
            pVel.y = std::min(pVel.y, 1050.0f);

            float vyBefore = pVel.y;
            player.x += pVel.x * dt;
            ResolveSolidX(player.x, player.y, playerHw, playerHh, pVel.x, platforms);

            player.y += pVel.y * dt;
            bool grounded = false;
            ResolveSolidY(player.x, player.y, playerHw, playerHh, pVel.y, grounded, platforms, vyBefore);

            for (Platform &pl : platforms)
            {
                const float k = 72.0f;
                const float d = 14.0f;
                pl.flexVel += (-pl.flex) * k * dt;
                pl.flexVel -= pl.flexVel * d * dt;
                pl.flex += pl.flexVel * dt;
            }

            if (grounded)
                timeSinceGrounded = 0.0f;
            else
                timeSinceGrounded += dt;

            if (jumpBuffered > 0.0f)
                jumpBuffered -= dt;

            player.x = std::clamp(player.x, playerHw + 2.0f, (float)screenW - playerHw - 2.0f);

            if (fabsf(inputX) > 0.01f)
                playerLook = {inputX > 0.0f ? 1.0f : -1.0f, -0.35f};
            else
                playerLook = {playerLook.x, -0.45f};

            float fh = feetY();
            float heightPx = groundY - fh;
            bestHeightPx = std::max(bestHeightPx, heightPx);

            while (player.y < genTop + 620.0f)
            {
                float nextTop = genTop - (float)GetRandomValue(1900, 2800);
                genTop = AppendPlatformsUp(platforms, screenW, genTop - 18.0f, nextTop, anchorX);
            }

            float camFollow = 10.0f;
            cam.target.x = screenW * 0.5f;
            cam.target.y += (player.y - cam.target.y) * std::min(1.0f, camFollow * dt);

            if (player.y > cam.target.y + (float)screenH * 0.58f)
                gameOver = true;
        }

        if (IsKeyPressed(KEY_R))
            resetRun();

        BeginDrawing();
        ClearBackground(Color{120, 185, 245, 255});

        BeginMode2D(cam);
        DrawSkyAndCloudsWorld(cam, screenW, screenH);
        for (const Platform &pl : platforms)
            DrawGrassPlatform(pl);
        DrawPlayerCharacter(player, playerLook, playerRadius);
        EndMode2D();

        DrawText(TextFormat("HEIGHT  %.0f m", bestHeightPx / 70.0f), 24, 20, 32, RAYWHITE);
        DrawText(TextFormat("NOW     %.0f m", (groundY - feetY()) / 70.0f), 24, 56, 24, LIGHTGRAY);
        DrawText("A/D move  |  Space/W/Up jump  |  R restart", 24, screenH - 36, 20, GRAY);

        if (gameOver)
        {
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.62f));
            DrawText("FELL DOWN", screenW / 2 - 130, screenH / 2 - 80, 48, ORANGE);
            DrawText(TextFormat("Best height: %.0f m", bestHeightPx / 70.0f), screenW / 2 - 150, screenH / 2 - 10, 28, RAYWHITE);
            DrawText("R to try again", screenW / 2 - 100, screenH / 2 + 40, 26, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
