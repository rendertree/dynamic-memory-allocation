// by Wildan Wijanarko

#include <stdio.h>
#include <cassert>
#include <memory>
#include <string>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#define MAX_COLLIDERS 4

using namespace std;

Rectangle recData[MAX_COLLIDERS] = {
    { 3.0f + 98.0f * 1.0f + 52.0f * 0.0f,  350, 100, 100 },
    { 3.0f + 98.0f * 2.0f + 52.0f * 1.0f,  350, 100, 100 },
    { 3.0f + 98.0f * 3.0f + 52.0f * 2.0f,  350, 100, 100 },
    { 3.0f + 98.0f * 4.0f + 52.0f * 3.0f,  350, 100, 100 },
};

const static inline void DrawBrick(
    const Texture2D& texture, 
    const Rectangle& rec,
    const Vector2& pos
);

struct Object
{
    ~Object()
    {
        DestroyPhysicsBody(body);
    }

    inline void Draw(const Texture2D& texture) const 
    {
        DrawBrick(texture, rec, (Vector2){ body->position.x - 50.0f, body->position.y - 50.0f });
    }

    Rectangle rec;
    PhysicsBody body = nullptr;
};

int main(void)
{
    const int screenWidth = 720;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "");

    SetTargetFPS(60);

    // We assume this rectangle is a camera rectangle
    Rectangle cameraRec = { 10, 10, 200, 200 };
    
    shared_ptr<Object> collidingObjects[MAX_COLLIDERS];
    int numCollidingObjects = 0; // Variable to keep track of the number of colliding objects
    int vertexCount = 0;
    int bodiesCount = 0;

    bool enabled = true;
    bool drawGrid = true;
    bool drawTexture = true;

    float recSize = 100.0f;
    float recPosY = 350.0f;

    for (int i = 0; i < MAX_COLLIDERS; i++)
    {
        collidingObjects[i] = nullptr;
    }

    Texture2D texture = LoadTexture("brick.png");

    while (!WindowShouldClose())
    {
        // Reset the counter for each frame
        numCollidingObjects = 0;

        if (enabled)
        {
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
                        collidingObjects[i].reset();
                    }
                }
            }
        }

        int diffAllocHeap = numCollidingObjects - GetPhysicsBodiesCount();

        if (diffAllocHeap < 0) diffAllocHeap *= -1.0f;

        // The max of the diff is 2,
        // because there may be some delay with the physics system, we can't make it equal (but it's fine and safe),
        // and at least it's not more than 2.
        assert(diffAllocHeap < 2);

        string strPhysicsBody = { "Physics Body: " };
        string strObjects     = { "Objects: " };
        string strVertices    = { "Vertices: " };
        
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (enabled)
        {
            for (int i = 0; i < numCollidingObjects; i++)
            {
                DrawRectangleRec({   
                    collidingObjects[i]->body->position.x - collidingObjects[i]->rec.width / 2,
                    collidingObjects[i]->body->position.y - collidingObjects[i]->rec.height / 2,
                    collidingObjects[i]->rec.width, collidingObjects[i]->rec.height }, RED
                );

                if (drawTexture)
                {
                    collidingObjects[i]->Draw(texture);
                }
            }

            bodiesCount = GetPhysicsBodiesCount();
            for (int i = 0; i < bodiesCount; i++)
            {
                PhysicsBody body  = GetPhysicsBody(i);
                vertexCount       = GetPhysicsShapeVerticesCount(i);
                Vector2* vertices = new Vector2[vertexCount];

                for (int j = 0; j < vertexCount; j++)
                {
                    vertices[j] = GetPhysicsShapeVertex(body, j);

                    int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); // Get next vertex or first to close the shape
                    Vector2 nextVertex = GetPhysicsShapeVertex(body, jj);

                    DrawLineV(vertices[j], nextVertex, GREEN);
                    DrawCircleV(vertices[j], 3, YELLOW);
                }
                
                delete[] vertices;
            }
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {   
                const Rectangle rec = { recData[i].x - 50, recData[i].y - 50, recData[i].width, recData[i].height };
                DrawRectangleRec(rec, RED);
                DrawRectangleLinesEx(rec, 1.2f, GREEN);
                if (drawTexture)
                {
                    DrawBrick(texture, rec, (Vector2){ rec.x, rec.y });
                }
            }
        }

        if (drawGrid)
        {
            rlPushMatrix();
            rlTranslatef(0, 25 * 50, 0);
            rlRotatef(90, 1, 0, 0);
            DrawGrid(100, 50);
            rlPopMatrix();
        }

        DrawRectangleLines(cameraRec.x, cameraRec.y, cameraRec.width, cameraRec.height, GREEN);

        float uiSettingsLeft = screenWidth - 150;

        DrawFPS(0, 0);
        DrawText(strPhysicsBody.append(to_string(GetPhysicsBodiesCount())).c_str(), 20, 40,  21, BLACK);
        DrawText(strObjects.append(to_string(numCollidingObjects)).c_str(),         20, 70,  21, BLACK);
        DrawText(strVertices.append(to_string(vertexCount * bodiesCount)).c_str(),  20, 100, 21, BLACK);

        GuiGroupBox((Rectangle){ uiSettingsLeft - 10, 20, 150, 260 }, "Settings");
        enabled     = GuiCheckBox((Rectangle){ uiSettingsLeft, 40 + 30 * 0, 20, 20 }, "Enabled", enabled);
        drawGrid    = GuiCheckBox((Rectangle){ uiSettingsLeft, 40 + 30 * 1, 20, 20 }, "Draw Grid", drawGrid);
        drawTexture = GuiCheckBox((Rectangle){ uiSettingsLeft, 40 + 30 * 2, 20, 20 }, "Draw Texture", drawTexture);

        cameraRec.width = GuiSliderBar(
            (Rectangle){ uiSettingsLeft + 40, 130 + 20 * 0, 50, 15 }, 
            "CamRec", 
            TextFormat("%3.2f", cameraRec.width),
            cameraRec.width,
            10.0f, 450.0f);

        recSize = GuiSliderBar(
            (Rectangle){ uiSettingsLeft + 40, 130 + 20 * 1, 50, 15 }, 
            "Rec", 
            TextFormat("%3.2f", recSize),
            recSize,
            50.0f, 100.0f);
        
        recPosY = GuiSliderBar(
            (Rectangle){ uiSettingsLeft + 40, 130 + 20 * 2, 50, 15 }, 
            "RecY", 
            TextFormat("%3.2f", recPosY),
            recPosY,
            180.0f, 354.0f);
        
        for (int i = 0; i < 4; i++)
        {
            recData[i].y      = recPosY;
            recData[i].height = recSize;
            recData[i].width  = recSize;
        }

        if (GuiButton((Rectangle){ uiSettingsLeft, 250, 50, 20}, "RESET"))
        {
            recSize = 100.0f;
            recPosY = 350.0f;
            cameraRec.width = 200.0f;
        }

        cameraRec.height = cameraRec.width;
        cameraRec.x = GetMouseX() - cameraRec.width / 2;
        cameraRec.y = GetMouseY() - cameraRec.height / 2; 

        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    return 0;
}

const static inline void DrawBrick(
    const Texture2D& texture, 
    const Rectangle& rec,
    const Vector2& pos)
{
    DrawTextureRec(
        texture,
        rec,
        pos,
        WHITE
    );
}