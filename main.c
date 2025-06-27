#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define W 1000
#define H 650
#define NUM_ORBITS 8

typedef struct  {
    Vector3 color;
} Orbit;

Orbit orbits[NUM_ORBITS] = {
    { {1.0f, 0.0f, 0.0f} }, // red
    { {0.0f, 1.0f, 0.0f} }, // green
    { {0.0f, 0.0f, 1.0f} }, // blue
    { {1.0f, 1.0f, 1.0f} }, // white
    { {1.0f, 1.0f, 0.0f} }, // yellow
    { {1.0f, 0.0f, 1.0f} }, // magenta
    { {0.0f, 1.0f, 1.0f} }, // cyan
    { {1.0f, 0.4f, 0.0f} }  // orange
};

Vector3 starts[NUM_ORBITS] = {
    {6.5f, 0.0f, 0.0f}, 
    {8.3f, 1.0f, 0.0f}, 
    {8.1f, 1.0f, 0.0f}, 
    {7.9f, 0.0f, 0.0f}, 
    {7.5f, 0.0f,  0.0f}, 
    {-1.0f, -1.0f, 0.0f}, 
    {0.0f, -1.0f, 0.0f}, 
    {1.0f, -1.0f, 0.0f}
};

Vector3 axes[NUM_ORBITS] = {
    {0.7071, 0.7071, 0}, 
    {1, 0, 0}, 
    {-0.7071, 0.7071, 0}, 
    {0, 0, 1}, 
    {0, 0, 1},  
    {-0.7071, 0.7071, 0}, 
    {1, 0, 0},
    {0.7071, 0.7071, 0}, 
};

int main(void) {
    // Initialization
    InitWindow(W, H, "Raylib - Demo");
    SetTargetFPS(60);

    // Create and unload FBO
    RenderTexture2D hdr = LoadRenderTexture(W, H);
    UnloadTexture(hdr.texture);

    // Update the Texture2D metadata
    hdr.texture.id = rlLoadTexture(NULL, W, H, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
    hdr.texture.width = W;
    hdr.texture.height = H;
    hdr.texture.mipmaps = 1;
    hdr.texture.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

    // Re-attach texture as the FBO's color target
    rlFramebufferAttach(hdr.id, hdr.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    // Initialize camera
    Camera3D cam = { 0 };
    cam.position = (Vector3) { 0.0f, 2.0f, 3.0f };
    cam.target   = (Vector3) { 0.0f, 0.0f, 0.0f };
    cam.up       = (Vector3) { 0.0f, 1.0f, 0.0f };
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    
    // Load shaders
    Shader sh = LoadShader("resources/shaders/default.vs", "resources/shaders/phong.fs");
    int locLightPos = GetShaderLocation(sh, "u_lightPos");
    int locLightCol = GetShaderLocation(sh, "u_lightColor");
    int locLightInt = GetShaderLocation(sh, "u_lightIntensity");
    int locEyePos   = GetShaderLocation(sh, "u_eyePos");
    int locAmb      = GetShaderLocation(sh, "u_ambientColor");
    int locSpec     = GetShaderLocation(sh, "u_specularColor");
    int locShine    = GetShaderLocation(sh, "u_shininess");
    sh.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(sh, "diffuseMap");
    sh.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(sh, "normalMap");

    Shader shEmis  = LoadShader("resources/shaders/default.vs", "resources/shaders/emissive.fs");
    int locEmis    = GetShaderLocation(shEmis, "u_emissiveColor");
    int locEmisInt = GetShaderLocation(shEmis, "u_emissiveIntensity");
    shEmis.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shEmis, "emissionMap");

    Shader shSky = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    int locRotView    = GetShaderLocation(shSky, "rotView");
    int locProjection = GetShaderLocation(shSky, "matProjection");
    shSky.locs[SHADER_LOC_MAP_CUBEMAP] = GetShaderLocation(shSky, "cubemap");

    Shader shHDR = LoadShader(NULL, "resources/shaders/hdr.fs");
    int locHdrGamma = GetShaderLocation(shHDR, "u_gamma");
    int locHdrExposure = GetShaderLocation(shHDR, "u_exposure");
    shHDR.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(shHDR, "hdrBuffer");

    // Load cubemap images
    Image px = LoadImage("resources/textures/right.jpg");
    Image nx = LoadImage("resources/textures/left.jpg"); 
    Image py = LoadImage("resources/textures/top.jpg"); 
    Image ny = LoadImage("resources/textures/bottom.jpg");
    Image pz = LoadImage("resources/textures/front.jpg");
    Image nz = LoadImage("resources/textures/back.jpg");

    int fw = px.width;
    int fh = px.height;
    
    // Create an image atlas for the cubemap
    Image atlas = GenImageColor(fw*4, fh*3, BLANK);
    ImageDraw(&atlas, nx, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {0*fw, 1*fh, fw, fh}, WHITE); 
    ImageDraw(&atlas, px, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {2*fw, 1*fh, fw, fh}, WHITE); 
    ImageDraw(&atlas, py, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {1*fw, 0*fh, fw, fh}, WHITE); 
    ImageDraw(&atlas, ny, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {1*fw, 2*fh, fw, fh}, WHITE); 
    ImageDraw(&atlas, pz, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {1*fw, 1*fh, fw, fh}, WHITE); 
    ImageDraw(&atlas, nz, (Rectangle) {0*fw, 0*fh, fw, fh}, (Rectangle) {3*fw, 1*fh, fw, fh}, WHITE);

    // Load textures and models
    //Model moonModel = LoadModel("resources/objects/sponza.glb");
    //Texture2D moonTex = LoadTexture("resources/textures/moon.jpg");
    //Texture2D moonNormalTex = LoadTexture("resources/textures/moon_normal.jpg");

    Model sponzaModel = LoadModel("resources/objects/sponza.glb");
    for (int i = 0; i < sponzaModel.materialCount; i++) {
        sponzaModel.materials[i].shader = sh;        
    }

    //moonModel.materials[0].shader = sh;
    //moonModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = moonTex;
    //moonModel.materials[0].maps[MATERIAL_MAP_NORMAL].texture = moonNormalTex;

    Texture2D sunTex = LoadTexture("resources/textures/sun.jpg"); 

    // Load the sun image for CPU sampling
    Image sunImg = LoadImage("resources/textures/sun.jpg");
    Color c = GetImageColor(sunImg, sunImg.width / 2, sunImg.height / 2);
    float sunMask = (0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b) / 255.0f;
    sunMask = pow(sunMask * 1.5f, 0.8f);
    UnloadImage(sunImg);

    TextureCubemap skyTex = LoadTextureCubemap(atlas, CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE);

    // Create skybox model
    Model skyModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    skyModel.materials[0].shader = shSky;
    skyModel.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = skyTex;
    
    Model orbitModels[NUM_ORBITS];
    Vector3 lightColor[NUM_ORBITS];
    float lightIntensity[NUM_ORBITS];
    Vector3 emissiveColor[NUM_ORBITS];
    float emissiveIntensity[NUM_ORBITS];
    
    // Create orbit models
    for (int i = 0; i < NUM_ORBITS; i++) {
        Mesh mesh = GenMeshSphere(0.2f, 64, 64);
        orbitModels[i] = LoadModelFromMesh(mesh);

        orbitModels[i].materials[0].shader = shEmis;
        orbitModels[i].materials[0].maps[MATERIAL_MAP_EMISSION].texture = sunTex;

        lightColor[i] = emissiveColor[i] = Vector3Scale(orbits[i].color, sunMask);
        lightIntensity[i] = emissiveIntensity[i] = 2.5f;
    }

    // Set colors
    Vector3 ambientColor = { 0.05f, 0.05f, 0.05f };
    Vector3 specularColor = { 1.0f, 1.0f, 1.0f };
    float shininess = 32.0f;

    // Set HDR parameters
    float exposure = 1.0f;
    float gamma = 1.0f;

    // Set objects rotation and scale
    float moonSpinAngle = 0.0f;
    float moonSpinSpeed = 45.0;
    Vector3 moonSpinAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 moonScale = { 0.7f, 0.7f, 0.7f };

    float orbitSpinAngle = 0.0f;
    float orbitSpinSpeed = 0;
    Vector3 orbitScale = { 0.2f, 0.2f, 0.2f };
    
    while (!WindowShouldClose()) {
        UpdateCamera(&cam, CAMERA_FREE);

        // Only use rotation part of the view matrix
        Matrix view = GetCameraMatrix(cam);
        view.m12 = view.m13 = view.m14 = 0.0f;
        SetShaderValueMatrix(shSky, locRotView, view);

        // Build projection matrix
        float aspect = (float)W / (float)H;
        Matrix projection = MatrixPerspective(cam.fovy * DEG2RAD, aspect, 0.1f, 1000.0f);
        SetShaderValueMatrix(shSky, locProjection, projection);

        // Calculate rotation angles
        float dt = GetFrameTime();
        moonSpinAngle += moonSpinSpeed * dt;
        orbitSpinAngle += orbitSpinSpeed * dt;

        Vector3 positions[NUM_ORBITS];
        for (int i = 0; i < NUM_ORBITS; i++) {
            positions[i] = Vector3RotateByAxisAngle(starts[i], axes[i], orbitSpinAngle);
        }

        // Set shader values
        SetShaderValueV(sh, locLightPos, &positions[0], SHADER_UNIFORM_VEC3, NUM_ORBITS);
        SetShaderValueV(sh, locLightCol, &lightColor[0], SHADER_UNIFORM_VEC3, NUM_ORBITS);
        SetShaderValueV(sh, locLightInt, &lightIntensity[0], SHADER_UNIFORM_FLOAT, NUM_ORBITS);

        SetShaderValue(sh, locEyePos, &cam.position, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locAmb, &ambientColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locSpec, &specularColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locShine, &shininess, SHADER_UNIFORM_FLOAT);
        
        SetShaderValue(shSky, locRotView, &view, SHADER_LOC_MATRIX_VIEW);

        for (int i = 0; i < NUM_ORBITS; i++) {
            SetShaderValue(shEmis, locEmis, &emissiveColor[i], SHADER_UNIFORM_VEC3);
            SetShaderValue(shEmis, locEmisInt, &emissiveIntensity[i], SHADER_UNIFORM_FLOAT);
        }

        BeginDrawing();
        //BeginTextureMode(hdr);
            ClearBackground(BLACK);
            BeginMode3D(cam);

                BeginShaderMode(shSky);
                    rlDisableBackfaceCulling();
                    rlDisableDepthMask();

                    DrawModel(skyModel, cam.position, 1.0f, WHITE);

                    rlEnableBackfaceCulling();
                    rlEnableDepthMask();
                EndShaderMode();

                BeginShaderMode(sh);
                    //DrawModelEx(moonModel, (Vector3) {0.0f, 0.0f, 0.0f}, moonSpinAxis, moonSpinAngle, moonScale, WHITE);
                    DrawModel(sponzaModel, (Vector3) {0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                EndShaderMode();

                BeginShaderMode(shEmis);
                    for (int i = 0; i < NUM_ORBITS; i++) {
                        SetShaderValue(shEmis, locEmis, &emissiveColor[i], SHADER_UNIFORM_VEC3);
                        SetShaderValue(shEmis, locEmisInt, &emissiveIntensity[i], SHADER_UNIFORM_FLOAT);
                        DrawModelEx(orbitModels[i], positions[i], axes[i], orbitSpinAngle, orbitScale, WHITE);
                    }
                EndShaderMode();

            EndMode3D();
            DrawFPS(10, 10);
        //EndTextureMode();
        EndDrawing();

        /*
        BeginDrawing();
            ClearBackground(BLACK);

            BeginShaderMode(shHDR);
                SetShaderValue(shHDR, locHdrGamma, &gamma, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shHDR, locHdrExposure, &exposure, SHADER_UNIFORM_FLOAT);
                SetShaderValueTexture(shHDR, SHADER_LOC_MAP_DIFFUSE, hdr.texture);
                DrawTextureRec(hdr.texture, (Rectangle) {0, 0, W, -H}, (Vector2) {0, 0}, WHITE);
            EndShaderMode();

            DrawFPS(10, 10);
        EndDrawing();
        */
    }

    UnloadShader(sh);
    UnloadShader(shEmis);
    for (int i = 0; i < NUM_ORBITS; i++) UnloadModel(orbitModels[i]);
    //UnloadModel(moonModel);
    //UnloadTexture(moonTex);
    //UnloadTexture(moonNormalTex);
    UnloadTexture(sunTex);
    UnloadTexture(hdr.texture);
    UnloadImage(px);
    UnloadImage(nx);
    UnloadImage(py);
    UnloadImage(ny);
    UnloadImage(pz);
    UnloadImage(nz);
    UnloadImage(atlas);

    CloseWindow();

    return 0;
}
