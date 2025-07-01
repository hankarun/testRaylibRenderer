#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "imgui.h"
#include "rlImGui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define W 1000
#define H 650
#define NUM_LIGHTS 8

typedef struct  {
    Vector3 color;
} Orbit;

typedef struct {
    Vector3 position;
    Vector3 color;
    float intensity;
    float range;
} Light;

int main(void) {
    // Initialization
    InitWindow(W, H, "Raylib - Demo");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    rlImGuiSetup(true);

    // Function to create render textures
    auto createRenderTextures = [](int width, int height, RenderTexture2D& hdr, RenderTexture2D& bright, RenderTexture2D pingpong[2], RenderTexture2D& fxaaBuffer) {
        // Clean up existing textures if they exist
        if (hdr.id != 0) {
            UnloadTexture(hdr.texture);
            UnloadRenderTexture(hdr);
        }
        if (bright.id != 0) {
            UnloadTexture(bright.texture);
            UnloadRenderTexture(bright);
        }
        if (fxaaBuffer.id != 0) {
            UnloadTexture(fxaaBuffer.texture);
            UnloadRenderTexture(fxaaBuffer);
        }
        for (int i = 0; i < 2; i++) {
            if (pingpong[i].id != 0) {
                UnloadTexture(pingpong[i].texture);
                UnloadRenderTexture(pingpong[i]);
            }
        }

        // Create HDR render texture
        hdr = LoadRenderTexture(width, height);
        UnloadTexture(hdr.texture);
        hdr.texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
        hdr.texture.width = width;
        hdr.texture.height = height;
        hdr.texture.mipmaps = 1;
        hdr.texture.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

        // Create bright render texture
        bright = LoadRenderTexture(width, height);
        UnloadTexture(bright.texture);
        bright.texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
        bright.texture.width = width;
        bright.texture.height = height;
        bright.texture.mipmaps = 1;
        bright.texture.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

        // Bind the same FBO and attach both to it
        rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, hdr.id);
        rlBindFramebuffer(RL_READ_FRAMEBUFFER, hdr.id);
            rlFramebufferAttach(hdr.id, hdr.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
            rlFramebufferAttach(hdr.id, bright.texture.id, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
        
        // Activate two draw color buffers
        rlActiveDrawBuffers(2);

        // Unbind buffers
        rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
        rlBindFramebuffer(RL_READ_FRAMEBUFFER, 0);

        // Ping-pong FBOs for Gaussian blur
        for (int i = 0; i < 2; i++) {
            pingpong[i] = LoadRenderTexture(width, height);
            UnloadTexture(pingpong[i].texture);

            pingpong[i].texture.id = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
            pingpong[i].texture.width = width;
            pingpong[i].texture.height = height;
            pingpong[i].texture.mipmaps = 1;
            pingpong[i].texture.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

            rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, pingpong[i].id);
            rlBindFramebuffer(RL_READ_FRAMEBUFFER, pingpong[i].id);
                rlFramebufferAttach(pingpong[i].id, pingpong[i].texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
                rlActiveDrawBuffers(1);

            rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
            rlBindFramebuffer(RL_READ_FRAMEBUFFER, 0);
        }

        // Create FXAA buffer
        fxaaBuffer = LoadRenderTexture(width, height);
    };

    // Get current window dimensions
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();

    // Create render textures
    RenderTexture2D hdr, bright, fxaaBuffer;
    RenderTexture2D pingpong[2];
    createRenderTextures(currentWidth, currentHeight, hdr, bright, pingpong, fxaaBuffer);

    // Initialize camera
    Camera3D cam = { 0 };
    cam.position = Vector3{ -1.0f, 2.0f, -0.5f };
    cam.target   = Vector3{ 0.0f, 0.0f, 0.0f };
    cam.up       = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    
    // Load shaders
    Shader sh = LoadShader("resources/shaders/default.vs", "resources/shaders/phong.fs");
    int locEyePos    = GetShaderLocation(sh, "u_eyePos");
    int locAmb       = GetShaderLocation(sh, "u_ambientColor");
    int locSpec      = GetShaderLocation(sh, "u_specularColor");
    int locShine     = GetShaderLocation(sh, "u_shininess");
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
    shHDR.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shHDR, "bloomBlur");

    Shader shBlur = LoadShader(NULL, "resources/shaders/blur.fs");
    int locBlurHorizontal = GetShaderLocation(shBlur, "u_horizontal");
    shBlur.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(shBlur, "image");

    Shader shFXAA = LoadShader(NULL, "resources/shaders/fxaa.fs");
    int locFXAATexelStep = GetShaderLocation(shFXAA, "u_texelStep");
    shFXAA.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(shFXAA, "texture0");

    // Load cubemap imagesf
    Image px = LoadImage("resources/textures/right.jpg");
    Image nx = LoadImage("resources/textures/left.jpg"); 
    Image py = LoadImage("resources/textures/top.jpg"); 
    Image ny = LoadImage("resources/textures/bottom.jpg");
    Image pz = LoadImage("resources/textures/front.jpg");
    Image nz = LoadImage("resources/textures/back.jpg");

    int fw = px.width;
    int fh = px.height;

    float fwF = static_cast<float>(fw);
    float fhF = static_cast<float>(fh);
    
    // Create an image atlas for the cubemap
    Image atlas = GenImageColor(fw*4, fh*3, BLANK);

    // Common source‐rect
    Rectangle srcRec{ 0.0f, 0.0f, fwF, fhF };

    // Build all the destination rects with floats
    Rectangle dstRecs[6] = {
        { 0*fwF, 1*fhF, fwF, fhF },  
        { 2*fwF, 1*fhF, fwF, fhF },
        { 1*fwF, 0*fhF, fwF, fhF },
        { 1*fwF, 2*fhF, fwF, fhF },
        { 1*fwF, 1*fhF, fwF, fhF },
        { 3*fwF, 1*fhF, fwF, fhF }
    };

    Image faces[6] = { nx, px, py, ny, pz, nz };

    for (int i = 0; i < 6; i++) {
        ImageDraw(&atlas, faces[i], srcRec, dstRecs[i], WHITE);
    }

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
    
    Model orbitModels[NUM_LIGHTS];
    Light lights[NUM_LIGHTS];
    
    Vector3 starts[NUM_LIGHTS] = {
    {8.3f, 1.7f, 0.0f},
    {4.3f, 1.7f, 0.0f},
    {0.3f, 1.7f, 0.0f},
    {-4.3f, 1.7f, 0.0f}
    };
    // Create orbit models
    for (int i = 0; i < NUM_LIGHTS; i++) {
        Mesh mesh = GenMeshSphere(0.2f, 64, 64);
        orbitModels[i] = LoadModelFromMesh(mesh);

        orbitModels[i].materials[0].shader = shEmis;
        orbitModels[i].materials[0].maps[MATERIAL_MAP_EMISSION].texture = sunTex;

        lights[i].color = { 1.0f, 1.0f, 1.0f }; // Default color
        lights[i].position = starts[i];
        lights[i].intensity = 1.0f; // Default intensity
        lights[i].range = 4.0f; // Default range value
    }

    // Set colors
    Vector3 ambientColor = { 0.001f, 0.001f, 0.001f };
    Vector3 specularColor = { 1.0f, 1.0f, 1.0f };
    float shininess = 32.0f;

    // Set HDR/bloom parameters
    float exposure = 1.0f;
    float gamma = 2.2f;
    float bloomThreshold = 1.0f;
    bool enableFXAA = true;

    // Light management UI
    bool showLightDetails = false;
    int selectedLight = 0;

    // Set objects rotation and scale
    float moonSpinAngle = 0.0f;
    float moonSpinSpeed = 45.0;
    Vector3 moonSpinAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 moonScale = { 0.7f, 0.7f, 0.7f };

    float orbitSpinAngle = 0.0f;
    float orbitSpinSpeed = 0;
    Vector3 orbitScale = { 0.2f, 0.2f, 0.2f };

    //bool updateCamera = false;
    
    while (!WindowShouldClose()) {
        // Check for window resize
        int newWidth = GetScreenWidth();
        int newHeight = GetScreenHeight();
        if (newWidth != currentWidth || newHeight != currentHeight) {
            currentWidth = newWidth;
            currentHeight = newHeight;
            createRenderTextures(currentWidth, currentHeight, hdr, bright, pingpong, fxaaBuffer);
        }
        /*
        if (updateCamera)
            UpdateCamera(&cam, CAMERA_FREE);
        */
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            UpdateCamera(&cam, CAMERA_FREE);

        // Only use rotation part of the view matrix
        Matrix view = GetCameraMatrix(cam);
        view.m12 = view.m13 = view.m14 = 0.0f;
        SetShaderValueMatrix(shSky, locRotView, view);

        // Build projection matrix
        float aspect = (float)currentWidth / (float)currentHeight;
        Matrix projection = MatrixPerspective(cam.fovy * DEG2RAD, aspect, 0.1f, 1000.0f);
        SetShaderValueMatrix(shSky, locProjection, projection);

        // Calculate rotation angles
        float dt = GetFrameTime();
        moonSpinAngle += moonSpinSpeed * dt;
        orbitSpinAngle += orbitSpinSpeed * dt;

        // Set shader values - we need to set each struct member individually
        for (int i = 0; i < NUM_LIGHTS; i++) {
            char uniformName[64];
            
            // Set position
            sprintf(uniformName, "u_lights[%d].position", i);
            int locPos = GetShaderLocation(sh, uniformName);
            if (locPos != -1) SetShaderValue(sh, locPos, &lights[i].position, SHADER_UNIFORM_VEC3);
            
            // Set color
            sprintf(uniformName, "u_lights[%d].color", i);
            int locCol = GetShaderLocation(sh, uniformName);
            if (locCol != -1) SetShaderValue(sh, locCol, &lights[i].color, SHADER_UNIFORM_VEC3);
            
            // Set intensity
            sprintf(uniformName, "u_lights[%d].intensity", i);
            int locInt = GetShaderLocation(sh, uniformName);
            if (locInt != -1) SetShaderValue(sh, locInt, &lights[i].intensity, SHADER_UNIFORM_FLOAT);
            
            // Set range
            sprintf(uniformName, "u_lights[%d].range", i);
            int locRange = GetShaderLocation(sh, uniformName);
            if (locRange != -1) SetShaderValue(sh, locRange, &lights[i].range, SHADER_UNIFORM_FLOAT);
        }

        SetShaderValue(sh, locEyePos, &cam.position, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locAmb, &ambientColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locSpec, &specularColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(sh, locShine, &shininess, SHADER_UNIFORM_FLOAT);

        SetShaderValue(shHDR, locHdrGamma, &gamma, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shHDR, locHdrExposure, &exposure, SHADER_UNIFORM_FLOAT);

        SetShaderValue(shSky, locRotView, &view, SHADER_LOC_MATRIX_VIEW);


        //BeginDrawing();
        BeginTextureMode(hdr);
            rlActiveDrawBuffers(2);
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
                    DrawModel(sponzaModel, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
                EndShaderMode();

                BeginShaderMode(shEmis);
                    for (int i = 0; i < NUM_LIGHTS; i++) {
                        SetShaderValue(shEmis, locEmis, &lights[i].color, SHADER_UNIFORM_VEC3);
                        SetShaderValue(shEmis, locEmisInt, &lights[i].intensity, SHADER_UNIFORM_FLOAT);
                        DrawModelEx(orbitModels[i], lights[i].position, {0,0,0}, 0, Vector3{0.2f, 0.2f, 0.2f}, WHITE);
                    }
                EndShaderMode();

            EndMode3D();
                              

            //DrawFPS(10, 10);
        EndTextureMode();
        //EndDrawing();

        bool firstIteration = true;
        int horizontal = 0;
        int amount = 10;
        
        for (int i = 0; i < amount; i++) {
            BeginTextureMode(pingpong[horizontal]);
                rlActiveDrawBuffers(1);

                BeginShaderMode(shBlur);
                    Texture2D src = firstIteration ? bright.texture : pingpong[(horizontal + 1) % 2].texture;

                    SetShaderValue(shBlur, locBlurHorizontal, &horizontal, SHADER_UNIFORM_INT);
                    SetShaderValueTexture(shBlur, shBlur.locs[SHADER_LOC_MAP_DIFFUSE], src);

                    Rectangle srcRect = { 0, 0, (float)currentWidth, -(float)currentHeight };
                    Vector2 position = { 0, 0 };
                    DrawTextureRec(src, srcRect, position, WHITE);  
                EndShaderMode();
                
            EndTextureMode();

            horizontal = (i + 1) % 2;
            firstIteration = false;
        }

        BeginTextureMode(fxaaBuffer);
            BeginShaderMode(shHDR);
                SetShaderValueTexture(shHDR, shHDR.locs[SHADER_LOC_MAP_DIFFUSE], hdr.texture);
                SetShaderValueTexture(shHDR, shHDR.locs[SHADER_LOC_MAP_EMISSION], pingpong[(horizontal + 1) % 2].texture);
                Rectangle hdrRect = { 0, 0, (float)currentWidth, -(float)currentHeight };
                Vector2 hdrPosition = { 0, 0 };
                DrawTextureRec(hdr.texture, hdrRect, hdrPosition, WHITE);
            EndShaderMode();
        EndTextureMode();

        BeginDrawing();
            if (enableFXAA) {
                Vector2 texelStep = { 1.0f / (float)currentWidth, 1.0f / (float)currentHeight };
                SetShaderValue(shFXAA, locFXAATexelStep, &texelStep, SHADER_UNIFORM_VEC2);
                
                BeginShaderMode(shFXAA);
                    SetShaderValueTexture(shFXAA, shFXAA.locs[SHADER_LOC_MAP_DIFFUSE], fxaaBuffer.texture);
                    Rectangle fxaaRect = { 0, 0, (float)currentWidth, -(float)currentHeight };
                    Vector2 fxaaPosition = { 0, 0 };
                    DrawTextureRec(fxaaBuffer.texture, fxaaRect, fxaaPosition, WHITE);
                EndShaderMode();
            } else {
                // Render without FXAA - directly draw the HDR buffer
                Rectangle directRect = { 0, 0, (float)currentWidth, -(float)currentHeight };
                Vector2 directPosition = { 0, 0 };
                DrawTextureRec(fxaaBuffer.texture, directRect, directPosition, WHITE);
            }

            rlImGuiBegin();

            
            if (ImGui::Begin("Controls")) {
                ImGui::DragFloat3("Camera Pos", (float*)&cam.position, 0.01f, -10.0f, 10.0f);
                ImGui::DragFloat3("Ambient Color", (float*)&ambientColor, 0.01f, 0.0f, 2.0f);
                ImGui::DragFloat3("Specular Color", (float*)&specularColor, 0.01f, 0.0f, 2.0f);
                ImGui::DragFloat("Gamma", &gamma, 0.01f, 1.0f, 3.0f);
                ImGui::Checkbox("Enable FXAA", &enableFXAA);
                
                ImGui::Separator();
                ImGui::Checkbox("Show Light Details", &showLightDetails);
            }
            ImGui::End();

            // Light Details Window
            if (showLightDetails) {
                if (ImGui::Begin("Light Details", &showLightDetails)) {
                    // Light selection list
                    ImGui::Text("Lights:");
                    if (ImGui::BeginListBox("##LightList", ImVec2(-1, 100))) {
                        for (int i = 0; i < NUM_LIGHTS; i++) {
                            char lightName[32];
                            sprintf(lightName, "Light %d", i);
                            
                            bool isSelected = (selectedLight == i);
                            if (ImGui::Selectable(lightName, isSelected)) {
                                selectedLight = i;
                            }
                        }
                        ImGui::EndListBox();
                    }
                    
                    ImGui::Separator();
                    
                    // Selected light properties
                    if (selectedLight >= 0 && selectedLight < NUM_LIGHTS) {
                        ImGui::Text("Light %d Properties:", selectedLight);
                        
                        // Color
                        if (ImGui::ColorEdit3("Color", (float*)&lights[selectedLight].color)) {
                        }
                        
                        // Intensity
                        if (ImGui::DragFloat("Intensity", &lights[selectedLight].intensity, 0.1f, 0.0f, 10.0f)) {
                        }
                        
                        // Range
                        ImGui::DragFloat("Range", &lights[selectedLight].range, 0.5f, 0.1f, 50.0f, "%.1f");

                        if (ImGui::DragFloat3("Orbit Start", (float*)&lights[selectedLight].position.x, 0.1f, -20.0f, 20.0f)) {
                            // Position will be recalculated in the main loop
                        }
                    }
                }
                ImGui::End();
            }
            rlImGuiEnd();

            DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadShader(sh);
    UnloadShader(shEmis);
    UnloadShader(shSky);
    UnloadShader(shHDR);
    UnloadShader(shBlur);
    UnloadShader(shFXAA);
    for (int i = 0; i < NUM_LIGHTS; i++) UnloadModel(orbitModels[i]);
    //UnloadModel(moonModel);
    //UnloadTexture(moonTex);
    //UnloadTexture( moonNormalTex);
    UnloadTexture(sunTex);
    UnloadTexture(hdr.texture);
    UnloadTexture(bright.texture);
    UnloadTexture(fxaaBuffer.texture);
    UnloadTexture(pingpong[0].texture);
    UnloadTexture(pingpong[1].texture);
    UnloadImage(px);
    UnloadImage(nx);
    UnloadImage(py);
    UnloadImage(ny);
    UnloadImage(pz);
    UnloadImage(nz);
    UnloadImage(atlas);
    UnloadRenderTexture(hdr);

    CloseWindow();

    return 0;
}
