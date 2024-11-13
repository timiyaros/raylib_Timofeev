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
int health = 3;
int shirHealth = 150;
bool game = true;
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
    targetPos.x = GetRandomValue(0, 1870);
    targetPos.y = GetRandomValue(0, 1030);
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
                bulletActive = false;  // Пуля уничтожает цель
                // Перемещаем цель на новое место
                targetPos.x = GetRandomValue(0, 1870);
                targetPos.y = GetRandomValue(0, 1030);
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
        if (health == 0) {
            DrawText("game over", 800, 550, 40, RED);
            game = false;
        }




        while (game) {
            // Отображение
            BeginDrawing();
            ClearBackground(RAYWHITE);
            // отрисовка хп
            DrawRectangle(1700, 100, shirHealth, 50, GREEN);
            // Отрисовка игрока
            DrawRectangle(playerPos.x, playerPos.y, 50, 50, BLUE);
            // Отрисовка цели
            DrawRectangle(targetPos.x, targetPos.y, 30, 30, GREEN);
            // Отрисовка пули
            if (bulletActive) {
                DrawRectangle(bulletPos.x, bulletPos.y, 15, 15, RED);
            }
        }
        EndDrawing();
    }
    CloseWindow();  // Закрываем окно
    return 0;
}
