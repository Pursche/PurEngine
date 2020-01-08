#include <Windows.h>
#include <Core.h>

#include <Utils/Timer.h>
#include <Window/Window.h>

// Old renderer
#include <Renderer/Old/OldRenderer.h>

// Rendergraph
#include <Renderer/Renderers/DX12/RendererDX12.h>

#include "Camera.h"

u32 MainRenderLayer = "MainLayer"_h; // _h will compiletime hash the string into a u32

INT WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    PSTR /*lpCmdLine*/, INT nCmdShow)
{
    const int width = 1280;
    const int height = 720;

    Window mainWindow(hInstance, nCmdShow, Vector2(width, height));

    /*OldRenderer oldRenderer;
    if (!oldRenderer.Init(&mainWindow, width, height))
    {
        mainWindow.SendMessageBox("Error", "Failed to initialize direct3d 12");
    }*/

    Renderer::Renderer* renderer = new Renderer::RendererDX12();
    renderer->InitWindow(&mainWindow);

    Camera camera(Vector3(0,0,-5));

    Timer timer;
    while (true)
    {
        f32 deltaTime = timer.GetDeltaTime();
        timer.Tick();
        
        if (mainWindow.WantsToExit())
        {
            //oldRenderer.Cleanup();
            mainWindow.ConfirmExit();
            break;
        }
        mainWindow.Update(deltaTime);

        camera.Update(deltaTime);

        //oldRenderer.SetViewMatrix(camera.GetViewMatrix().Inverted());
        //oldRenderer.Update(deltaTime);
        //oldRenderer.Render();

        // RenderGraph, this is just a mockup so far and needs almost all implementation
        Renderer::RenderGraphDesc renderGraphDesc;
        Renderer::RenderGraph renderGraph = renderer->CreateRenderGraph(renderGraphDesc);

        // Create Resources
        Renderer::ImageDesc mainColorDesc;
        mainColorDesc.dimensions = Vector2i(1280, 720);
        mainColorDesc.format = Renderer::IMAGE_FORMAT_R16G16B16A16_FLOAT;
        mainColorDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

        Renderer::ImageID mainColor = renderer->CreateImage(mainColorDesc);

        Renderer::DepthImageDesc mainDepthDesc;
        mainDepthDesc.dimensions = Vector2i(1280, 720);
        mainDepthDesc.format = Renderer::DEPTH_IMAGE_FORMAT_D32_FLOAT;
        mainDepthDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

        Renderer::DepthImageID mainDepth = renderer->CreateDepthImage(mainDepthDesc);

        Renderer::RenderLayer mainLayer = renderer->GetRenderLayer(MainRenderLayer);

        struct ViewConstantBuffer
        {
            Matrix view;
            Matrix proj;

            float padding[128];
        };

        Renderer::ConstantBuffer<ViewConstantBuffer> viewConstantBuffer = renderer->CreateConstantBuffer<ViewConstantBuffer>();

        // Set ProjMatrix
        {
            Matrix& projMatrix = viewConstantBuffer.resource.proj;

            const f32 fov = (68.0f) / 2.0f;
            const f32 nearClip = 0.1f;
            const f32 farClip = 10.0f;
            f32 aspectRatio = static_cast<f32>(width) / static_cast<f32>(height);

            f32 tanFov = Math::Tan(Math::DegToRad(fov));
            projMatrix.right.x = 1.0f / (tanFov * aspectRatio);
            projMatrix.up.y = 1.0f / tanFov;
            projMatrix.at.z = -(farClip + nearClip) / (nearClip - farClip);
            projMatrix.pos.z = 2 * farClip * nearClip / (nearClip - farClip);
            projMatrix.pad3 = 1.0f;
            projMatrix.pad4 = 0.0f;
            projMatrix.Transpose();
            
            viewConstantBuffer.Apply(0);
        }
        
        Renderer::ModelDesc modelDesc;
        modelDesc.path = "models/cube.model";

        Renderer::ModelID cubeModel = renderer->LoadModel(modelDesc);
        Renderer::InstanceData cubeInstance;
        cubeInstance.modelMatrix = Matrix(); // Move, rotate etc the model here

        mainLayer.RegisterModel(cubeModel, cubeInstance); // This registers a cube model to be drawn in this layer with cubeInstance's model constantbuffer
        
        // Depth Prepass
        {
            struct DepthPrepassData
            {
                Renderer::RenderPassMutableResource depth;
            };

            renderGraph.AddPass<DepthPrepassData>("Depth Prepass",
            [mainDepth](DepthPrepassData& data, Renderer::RenderPassBuilder& builder) // Setup
            { 
                data.depth = builder.Write(mainDepth, Renderer::RenderPassBuilder::WriteMode::WRITE_MODE_RENDERTARGET, Renderer::RenderPassBuilder::LoadMode::LOAD_MODE_CLEAR);

                return true; // Return true from setup to enable this pass, return false to disable it
            },
            [&renderer](DepthPrepassData& data, Renderer::CommandList& commandList) // Execute
            {
                Renderer::GraphicsPipelineDesc pipelineDesc;

                // Shaders
                Renderer::VertexShaderDesc vertexShaderDesc;
                vertexShaderDesc.path = "test1.vs.hlsl";
                pipelineDesc.vertexShader = renderer->LoadShader(vertexShaderDesc); // This will load shader or use cached loaded shader

                Renderer::PixelShaderDesc pixelShaderDesc;
                pixelShaderDesc.path = "test1.ps.hlsl";
                pipelineDesc.pixelShader = renderer->LoadShader(pixelShaderDesc); // This will load shader or use cached loaded shader

                // Depth state
                pipelineDesc.depthStencilState.depthWriteEnable = true;

                // Render targets
                pipelineDesc.depthStencil = data.depth;

                // Set pipeline
                Renderer::GraphicsPipelineID pipeline = renderer->CreatePipeline(pipelineDesc); // This will compile the pipeline and return the ID, or return ID of cached pipeline
                commandList.SetPipeline(pipeline);

                // Render main layer
                Renderer::RenderLayer mainLayer = renderer->GetRenderLayer(MainRenderLayer);

                for (auto const& model : mainLayer.GetModels())
                {
                    auto const& id = Renderer::ModelID(model.first);
                    auto const& instances = model.second;

                    for (auto const& instance : instances)
                    {
                        commandList.Draw(id, instance);
                    }
                }
            });
        }

        // Main Pass
        {
            /*Renderer::VertexShaderDesc vertexShaderDesc;
            vertexShaderDesc.path = "shaders/main.vs.hlsl";

            Renderer::PixelShaderDesc pixelShaderDesc;
            pixelShaderDesc.path = "shaders/main.ps.hlsl";

            Renderer::GraphicsPipelineDesc pipelineDesc;
            // pipelineDesc.rasterizerState and pipelineDesc.blendState is available as well
            pipelineDesc.depthStencilState.depthWriteEnable = false;
            pipelineDesc.depthStencilState.depthFunc = Renderer::COMPARISON_FUNC_EQUAL; // Only pass depth test if depth is equal
            pipelineDesc.vertexShader = renderer.LoadShader(vertexShaderDesc);
            pipelineDesc.pixelShader = renderer.LoadShader(pixelShaderDesc);

            pipelineDesc.depthStencil = mainDepth;
            pipelineDesc.renderTargets[0].image = mainColor;
            // pipelineDesc.renderTargets[0].blendState is available as well

            Renderer::GraphicsPipelineID pipeline = renderer.CreatePipeline(pipelineDesc);*/

            //struct MainPassData
            //{
            //    Renderer::RenderPassResource depth;
            //    Renderer::RenderPassMutableResource color;
            //};

            //renderGraph.AddPass<MainPassData>("Main Pass",
            //[&](MainPassData& data, Renderer::RenderPassBuilder& builder) // OnSetup
            //{ 
            //    data.depth = builder.Read(mainDepth, Renderer::RenderPassBuilder::ShaderStage::SHADER_STAGE_PIXEL);
            //    data.color = builder.Write(mainColor, Renderer::RenderPassBuilder::WriteMode::WRITE_MODE_RENDERTARGET, Renderer::RenderPassBuilder::LoadMode::LOAD_MODE_CLEAR);
            //},
            //[=](MainPassData& /*data*/, Renderer::CommandList& commandList) // OnExecute
            //{ 
            //    commandList.AddDrawCommand(cubeModel);
            //});
        }

        renderGraph.Setup();
        renderGraph.Execute();
    }

    return 0;
}