// by Wildan Wijanarko

#include <stdio.h>
#include <cassert>
#include <memory>
#include <string>
#include <array>

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

const static inline void DrawBrickTexture(
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
        DrawBrickTexture(texture, rec, (Vector2){ body->position.x - 50.0f, body->position.y - 50.0f });
    }

    Rectangle rec;
    PhysicsBody body = nullptr;
};

template<size_t N>
struct ObjectAllocator
{
    size_t size = 0;
    int numCollidingObjects = 0; // Variable to keep track of the number of colliding objects
    array<shared_ptr<Object>, N> objectsArray = {};

    void Update(Rectangle recs[], const Rectangle& rec, int recsArrSize)
    {
        numCollidingObjects = 0;

        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionRecs(rec, recs[i]))
            {
                // Collision detected, check if the object already exists
                if (objectsArray[i] == nullptr)
                {
                    // If the object doesn't exist, create it and allocate memory on the heap
                    auto newObject = make_shared<Object>();
                    newObject->rec = recData[i];
                    newObject->body = CreatePhysicsBodyRectangle(
                        (Vector2){ recData[i].x, recData[i].y },
                        recData[i].width,
                        recData[i].height,
                        12.0f
                    );

                    // Increase the heap size
                    size += sizeof(Object);

                    // Store the object in the array
                    objectsArray[i] = newObject;
                }
                // Increment the count of colliding objects
                numCollidingObjects++;
            }
            else
            {
                // No collision, delete the physics body and reset the pointer
                if (objectsArray[i] != nullptr)
                {
                    // Decrease the heap size
                    size -= sizeof(Object);

                    // Release memory and reset the pointer
                    objectsArray[i].reset();
                }
            }
        }
    }
};

int main(void)
{
    const int screenWidth = 720;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "");

    SetTargetFPS(60);

    // We assume this rectangle is a camera rectangle
    Rectangle cameraRec = { 10, 10, 200, 200 };
    int vertexCount = 0;
    int bodiesCount = 0;

    bool enabled = true;
    bool drawGrid = true;
    bool drawTexture = true;

    float recSize = 100.0f;
    float recPosY = 350.0f;

    Texture2D texture = LoadTexture("brick.png");

    ObjectAllocator<MAX_COLLIDERS> allocator = {};

    for (int i = 0; i < MAX_COLLIDERS; i++)
    {
        allocator.objectsArray[i] = nullptr;
    }

    while (!WindowShouldClose())
    {
        if (enabled)
        {
            allocator.Update(recData, cameraRec, MAX_COLLIDERS);
        }

        int diffAllocHeap = allocator.numCollidingObjects - GetPhysicsBodiesCount();

        if (diffAllocHeap < 0) diffAllocHeap *= -1.0f;

        // The max of the diff is 2,
        // because there may be some delay with the physics system, we can't make it equal (but it's fine and safe),
        // and at least it's not more than 2.
        assert(diffAllocHeap < 2);

        string strPhysicsBody = { "Physics Body: " };
        string strObjects     = { "Objects: " };
        string strVertices    = { "Vertices: " };
        string strHeapSize    = { "Heap Size: " };
        
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (enabled)
        {
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

                // DrawRectangleRec({   
                //     allocator.objectsArray[i]->body->position.x - allocator.objectsArray[i]->rec.width / 2,
                //     allocator.objectsArray[i]->body->position.y - allocator.objectsArray[i]->rec.height / 2,
                //     allocator.objectsArray[i]->rec.width, allocator.objectsArray[i]->rec.height }, RED
                // );

                if (drawTexture)
                {
                    allocator.objectsArray[i]->Draw(texture);
                }
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
                    DrawBrickTexture(texture, rec, (Vector2){ rec.x, rec.y });
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
        DrawText(strPhysicsBody.append(to_string(GetPhysicsBodiesCount())).c_str(),     20, 40,  21, BLACK);
        DrawText(strObjects.append(to_string(allocator.numCollidingObjects)).c_str(),   20, 70,  21, BLACK);
        DrawText(strVertices.append(to_string(vertexCount * bodiesCount)).c_str(),      20, 100, 21, BLACK);
        DrawText(strHeapSize.append(to_string(allocator.size)).c_str(),                 20, 130, 21, BLACK);

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

const static inline void DrawBrickTexture(
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
