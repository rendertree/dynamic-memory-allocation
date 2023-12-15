// by Wildan Wijanarko

#include <stdio.h>
#include "raylib.h"
#include <cassert>
#include <memory>
#include <string>

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

#define MAX_COLLIDERS 5

using namespace std;

Rectangle recData[MAX_COLLIDERS] = {
    { 10,  200, 100, 100 },
    { 100, 200, 100, 100 },
    { 200, 200, 100, 100 },
    { 300, 200, 100, 100 },
    { 400, 200, 100, 100 },
};

struct Object
{
    ~Object()
    {
        DestroyPhysicsBody(body);
    }

    Rectangle rec;
    PhysicsBody body = nullptr;
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "");

    SetTargetFPS(60);

    // We assume this rectangle is a camera rectangle
    Rectangle cameraRec = { 10, 10, 200, 200 };
    
    shared_ptr<Object> collidingObjects[MAX_COLLIDERS];
    int numCollidingObjects = 0; // Variable to keep track of the number of colliding objects

    for (int i = 0; i < MAX_COLLIDERS; i++)
    {
        collidingObjects[i] = nullptr;
    }

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
                auto newObject = make_shared<Object>();
                
                // Store the data
                newObject->rec = recData[i];
                newObject->body = CreatePhysicsBodyRectangle(
                    (Vector2){ recData[i].x, recData[i].y },
                    recData[i].width,
                    recData[i].height, 
                    12.0f 
                );
                collidingObjects[numCollidingObjects] = newObject;
                numCollidingObjects++;
            }
            else 
            {
                // No collision, delete the physics body and reset the pointer
                if (collidingObjects[i] != nullptr)
                {
                    DestroyPhysicsBody(collidingObjects[i]->body);
                    collidingObjects[i].reset();
                }
            }
        }

        int diffAlocHeap = numCollidingObjects - GetPhysicsBodiesCount();

        if (diffAlocHeap < 0) diffAlocHeap *= -1.0f;

        // The max of the diff is 3,
        // Because there may be some delay with the physics system, we can't make it equal (but it's fine and safe),
        // and at least it's not more than 3.
        assert(diffAlocHeap < 3);

        string strPhysicsBody = { "Physics Body: " };
        string strObjects     = { "Objects: " };

        BeginDrawing();

        DrawFPS(0, 0);
        DrawText(strPhysicsBody.append(to_string(GetPhysicsBodiesCount())).c_str(), 100, 50,  21, BLACK);
        DrawText(strObjects.append(to_string(GetPhysicsBodiesCount())).c_str(), 100, 100, 21, BLACK);

        ClearBackground(RAYWHITE);

        for (int i = 0; i < numCollidingObjects; i++)
        {
            DrawRectangleRec(
                { collidingObjects[i]->body->position.x - collidingObjects[i]->rec.width / 2,
                collidingObjects[i]->body->position.y - collidingObjects[i]->rec.height / 2,
                collidingObjects[i]->rec.width, collidingObjects[i]->rec.height }, RED);

        }

        int bodiesCount = GetPhysicsBodiesCount();
        for (int i = 0; i < bodiesCount; i++)
        {
            PhysicsBody body = GetPhysicsBody(i);

            int vertexCount = GetPhysicsShapeVerticesCount(i);
            for (int j = 0; j < vertexCount; j++)
            {
                // Get physics bodies shape vertices to draw lines
                // Note: GetPhysicsShapeVertex() already calculates rotation transformations
                Vector2 vertexA = GetPhysicsShapeVertex(body, j);

                int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
                Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                DrawLineV(vertexA, vertexB, GREEN);     // Draw a line between two vertex positions
            }
        }

        DrawRectangleLines(cameraRec.x, cameraRec.y, cameraRec.width, cameraRec.height, GREEN);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
