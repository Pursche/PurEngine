#include <Windows.h>
#include <Core.h>

#include <Utils/Timer.h>
#include <Window/Window.h>

// Old renderer
#include <Renderer/Old/OldRenderer.h>

// Rendergraph
#include <Renderer/Renderer.h>

#include "Camera.h"

INT WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    PSTR /*lpCmdLine*/, INT nCmdShow)
{
    const int width = 1280;
    const int height = 720;

    Window mainWindow(hInstance, nCmdShow, Vector2(width, height));

    OldRenderer oldRenderer;
    if (!oldRenderer.Init(&mainWindow, width, height))
    {
        mainWindow.SendMessageBox("Error", "Failed to initialize direct3d 12");
    }

    Renderer::Renderer renderer;

    Camera camera(Vector3(0,0,-5));

    Timer timer;
    while (true)
    {
        f32 deltaTime = timer.GetDeltaTime();
        timer.Tick();
        
        if (mainWindow.WantsToExit())
        {
            oldRenderer.Cleanup();
            mainWindow.ConfirmExit();
            break;
        }
        mainWindow.Update(deltaTime);

        camera.Update(deltaTime);

        oldRenderer.SetViewMatrix(camera.GetViewMatrix().Inverted());
        oldRenderer.Update(deltaTime);
        oldRenderer.Render();

        // RenderGraph, this is just a mockup so far and needs almost all implementation
        Renderer::RenderGraphDesc renderGraphDesc;
        Renderer::RenderGraph renderGraph = renderer.CreateRenderGraph(renderGraphDesc);

        // Create Resources
        Renderer::ImageDesc mainColorDesc;
        mainColorDesc.dimensions = Vector2i(1280, 720);
        mainColorDesc.format = Renderer::IMAGE_FORMAT_R16G16B16A16_FLOAT;
        mainColorDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

        Renderer::ImageID mainColor = renderer.CreateImage(mainColorDesc);

        Renderer::DepthImageDesc mainDepthDesc;
        mainDepthDesc.dimensions = Vector2i(1280, 720);
        mainDepthDesc.format = Renderer::DEPTH_IMAGE_FORMAT_R32_FLOAT;
        mainDepthDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

        Renderer::DepthImageID mainDepth = renderer.CreateDepthImage(mainDepthDesc);

        Renderer::RenderLayer mainLayer = renderer.GetRenderLayer("MainLayer"_h); // _h will compiletime hash the string

        Renderer::ModelDesc modelDesc;
        modelDesc.path = "models/cube.model";

        Renderer::ModelID cubeModel = renderer.LoadModel(modelDesc);
        Renderer::InstanceData cubeInstance;
        cubeInstance.modelMatrix = Matrix(); // Move, rotate etc the model here

        mainLayer.RegisterModel(cubeModel, cubeInstance); // This registers a cube model to be drawn in this layer with cubeInstance's model constantbuffer
        
        // Depth Prepass
        {
            Renderer::VertexShaderDesc vertexShaderDesc;
            vertexShaderDesc.path = "shaders/prepass.vs.hlsl";

            Renderer::PixelShaderDesc pixelShaderDesc;
            pixelShaderDesc.path = "shaders/prepass.ps.hlsl";

            Renderer::GraphicsPipelineDesc pipelineDesc;
            pipelineDesc.cullState = Renderer::CULL_STATE_FRONT;
            pipelineDesc.frontFaceState = Renderer::FRONT_STATE_CLOCKWISE;
            pipelineDesc.sampleCount = Renderer::SAMPLE_COUNT_1;
            pipelineDesc.vertexShader = renderer.LoadShader(vertexShaderDesc);
            pipelineDesc.pixelShader = renderer.LoadShader(pixelShaderDesc);

            Renderer::GraphicsPipelineID pipeline = renderer.CreatePipeline(pipelineDesc);

            Renderer::RenderPass pass;
            pass.SetPipeline(pipeline);
            pass.AddRenderLayer(&mainLayer);
            
            renderGraph.AddPass(pass);
        }

        // Main Pass
        {
            Renderer::VertexShaderDesc vertexShaderDesc;
            vertexShaderDesc.path = "shaders/main.vs.hlsl";

            Renderer::PixelShaderDesc pixelShaderDesc;
            pixelShaderDesc.path = "shaders/main.ps.hlsl";

            Renderer::GraphicsPipelineDesc pipelineDesc;
            pipelineDesc.cullState = Renderer::CULL_STATE_FRONT;
            pipelineDesc.frontFaceState = Renderer::FRONT_STATE_CLOCKWISE;
            pipelineDesc.sampleCount = Renderer::SAMPLE_COUNT_1;
            pipelineDesc.vertexShader = renderer.LoadShader(vertexShaderDesc);
            pipelineDesc.pixelShader = renderer.LoadShader(pixelShaderDesc);

            Renderer::GraphicsPipelineID pipeline = renderer.CreatePipeline(pipelineDesc);

            Renderer::RenderPass pass;
            pass.SetPipeline(pipeline);
            pass.AddRenderLayer(&mainLayer);

            renderGraph.AddPass(pass);
        }

        renderGraph.Compile();
        renderGraph.Draw();
    }

    return 0;
}