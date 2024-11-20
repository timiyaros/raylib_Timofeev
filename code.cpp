#include "raylib.h"
#include <math.h>

// Глобальные константы
#define PLAYER_SPEED 10
#define BULLET_SPEED 15
#define TARGET_SPEED 5

// Глобальные переменные
Vector2 playerPos = { 910.0f, 510.0f };  // Позиция игрока
Vector2 bulletPos = { 0.0f, 0.0f };     // Позиция пули
Vector2 bulletVelocity = { 0.0f, 0.0f }; // Направление пули
Vector2 targetPos = { 0.0f, 0.0f };     // Позиция цели
Vector2 targetVelocity = { 0.0f, 0.0f }; // Направление цели

bool bulletActive = false; // Флаг для пули
int health = 3; //хп
int shirHealth = 150; // ширина хп
int random = 0;
int count = 0;
bool game = true; //игра идёт
Texture2D player = LoadTexture("C:/Users/Kids01/Downloads/texture.png");

// Функция для нормализации вектора
Vector2 NormalizeVector(Vector2 vec) {
    float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
    if (length != 0) {
        vec.x /= length;
        vec.y /= length;
    }
    return vec;
}

int main(void)
{
    InitWindow(1920, 1080, "game");
    SetTargetFPS(60);

    // Инициализация начальных позиций
    random = GetRandomValue(1, 4);
    if (random == 1) {
        targetPos.x = GetRandomValue(-10, 0);
        targetPos.y = GetRandomValue(0, 1030);
    }
    if (random == 2) {
        targetPos.x = GetRandomValue(0, 1870);
        targetPos.y = GetRandomValue(-10, 0);
    }
    if (random == 3) {
        targetPos.x = GetRandomValue(1920, 1970);
        targetPos.y = GetRandomValue(0, 1080);
    }
    if (random == 4) {
        targetPos.x = GetRandomValue(0, 1870);
        targetPos.y = GetRandomValue(1080, 1100);
    }


    while (!WindowShouldClose())
    {
        // Движение игрока
        if (IsKeyDown(KEY_A) && playerPos.x > 0) { playerPos.x -= PLAYER_SPEED; }
        if (IsKeyDown(KEY_D) && playerPos.x < 1870) { playerPos.x += PLAYER_SPEED; }
        if (IsKeyDown(KEY_S) && playerPos.y < 1030) { playerPos.y += PLAYER_SPEED; }
        if (IsKeyDown(KEY_W) && playerPos.y > 5) { playerPos.y -= PLAYER_SPEED; }
        if (IsKeyPressed(KEY_F11)) { ToggleFullscreen(); }

        // Стрельба
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !bulletActive) {
            bulletPos = playerPos;  // Пуля появляется в центре игрока
            bulletActive = true;

            // Вычисляем вектор от пули до цели
            Vector2 directionToTarget = { targetPos.x - bulletPos.x, targetPos.y - bulletPos.y };
            bulletVelocity = NormalizeVector(directionToTarget);  // Нормализуем вектор для движения пули
        }

        // Движение цели (зеленого квадрата) к игроку
        Vector2 directionToPlayer = { playerPos.x - targetPos.x, playerPos.y - targetPos.y };
        targetVelocity = NormalizeVector(directionToPlayer);  // Нормализуем вектор для движения цели

        targetPos.x += targetVelocity.x * TARGET_SPEED;
        targetPos.y += targetVelocity.y * TARGET_SPEED;

        // Двигаем пулю, если она была выстрелена
        if (bulletActive) {
            bulletPos.x += bulletVelocity.x * BULLET_SPEED;
            bulletPos.y += bulletVelocity.y * BULLET_SPEED;

            // Если пуля выходит за пределы экрана, сбрасываем её
            if (bulletPos.x < 0 || bulletPos.x > 1920 || bulletPos.y < 0 || bulletPos.y > 1080) {
                bulletActive = false;
            }

            // Проверка столкновения пули с целью
            Rectangle bulletRect = { bulletPos.x, bulletPos.y, 15, 15 };
            Rectangle targetRect = { targetPos.x, targetPos.y, 30, 30 };
            if (CheckCollisionRecs(bulletRect, targetRect)) {
                count = count + 1;
                bulletActive = false;  // Пуля уничтожает цель

                // Перемещаем цель на новое место
                random = GetRandomValue(1, 4);
                if (random == 1) {
                    targetPos.x = GetRandomValue(-10, 0);
                    targetPos.y = GetRandomValue(0, 1030);
                }
                if (random == 2) {
                    targetPos.x = GetRandomValue(0, 1870);
                    targetPos.y = GetRandomValue(-10, 0);
                }
                if (random == 3) {
                    targetPos.x = GetRandomValue(1920, 1970);
                    targetPos.y = GetRandomValue(0, 1080);
                }
                if (random == 4) {
                    targetPos.x = GetRandomValue(0, 1870);
                    targetPos.y = GetRandomValue(1080, 1100);
                }
            }
            if (count >= 5) {
                DrawRectangle(targetPos.x, targetPos.y, 30, 30, YELLOW);
            }
        }

        // Проверка столкновения игрока с целью
        Rectangle playerRect = { playerPos.x, playerPos.y, 50, 50 };
        Rectangle targetRect = { targetPos.x, targetPos.y, 30, 30 };
        if (CheckCollisionRecs(playerRect, targetRect)) {
            // Если игрок столкнулся с целью, цель исчезает
            targetPos.x = GetRandomValue(0, 1870);
            targetPos.y = GetRandomValue(0, 1030);
            health = health - 1;
            shirHealth = shirHealth - 50;
        }
        if (health <= 0) {
                
                game = false;
            }

                // Отображение
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawRectangle(1700, 100, shirHealth, 50, GREEN);

            // Отрисовка игрока
            DrawRectangle(playerPos.x, playerPos.y, 50, 50, BLUE);
            DrawTexture(player, playerPos.x, playerPos.y, WHITE);
            // Отрисовка цели
            DrawRectangle(targetPos.x, targetPos.y, 30, 30, GREEN);
            // Отрисовка пули
            if (bulletActive) {
                DrawRectangle(bulletPos.x, bulletPos.y, 15, 15, RED);
            }
            if (!game) {
                DrawRectangle(0, 0, 1000000, 1000000, BLACK);
                DrawText("game over", 800, 550, 40, RED);
            }

            EndDrawing();
        
    }

    CloseWindow();  // Закрываем окно
    return 0;
}
