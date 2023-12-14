// by Wildan Wijanarko

#include <stdio.h>
#include "raylib.h"
#include <stdlib.h>

#define MAX_COLLIDERS 5

Rectangle recData[MAX_COLLIDERS] = {
    { 10,  200, 100, 100 },
    { 100, 200, 100, 100 },
    { 200, 200, 100, 100 },
    { 300, 200, 100, 100 },
    { 400, 200, 100, 100 },
};

typedef struct Object
{
    Rectangle rec;
} Object;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "");

    SetTargetFPS(60);

    Rectangle cameraRec = { 10, 10, 200, 200 };

    Object* collidingObjects[MAX_COLLIDERS];
    int numCollidingObjects = 0; // Variable to keep track of the number of colliding objects

    while (!WindowShouldClose())
    {
        // Reset the counter for each frame
        numCollidingObjects = 0;

        cameraRec.x = GetMouseX() - 100.0f;
        cameraRec.y = GetMouseY() - 100.0f;

        for (int i = 0; i < MAX_COLLIDERS; i++)
        {
            if (CheckCollisionRecs(cameraRec, recData[i]))
            {
                // Collision detected, store the colliding object on the heap
                Object* newObject = (Object *)malloc(sizeof(Object));
                newObject->rec = recData[i]; // Store any relevant information here
                collidingObjects[numCollidingObjects] = newObject;
                numCollidingObjects++;
            }
            else
            {
                // No collision, set the pointer to NULL and free memory if it was allocated before
                if (collidingObjects[i] != NULL)
                {
                    free(collidingObjects[i]);
                }
                collidingObjects[i] = NULL;
            }
        }

        printf("%d \n", numCollidingObjects);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        for (int i = 0; i < numCollidingObjects; i++)
        {
            DrawRectangleRec(collidingObjects[i]->rec, RED);
        }

        DrawRectangleLines(cameraRec.x, cameraRec.y, cameraRec.width, cameraRec.height, GREEN);

        EndDrawing();
    }

    // Free allocated memory before closing the window
    for (int i = 0; i < numCollidingObjects; i++)
    {
        free(collidingObjects[i]);
    }

    CloseWindow();

    return 0;
}