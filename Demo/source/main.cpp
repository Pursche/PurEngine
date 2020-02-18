#include <Windows.h>
#include <Core.h>
#include <thread>

#include <Memory/StackAllocator.h>
#include <Utils/Timer.h>
#include <Utils/Defer.h>
#include <Window/Window.h>

// Rendergraph
#include <Renderer/Renderers/DX12/RendererDX12.h>

#include "Camera.h"

const u32 MAIN_RENDER_LAYER = "MainLayer"_h; // _h will compiletime hash the string into a u32
const size_t FRAME_ALLOCATOR_SIZE = 8 * 1024 * 1024; // 8 MB
const u8 TARGET_UPDATE_RATE = 60;

INT WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
    PSTR /*lpCmdLine*/, INT nCmdShow)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const int width = 1280;
    const int height = 720;

    Window mainWindow(hInstance, nCmdShow, Vector2(width, height));

    Renderer::Renderer* renderer = new Renderer::RendererDX12();
    renderer->InitWindow(&mainWindow);

    Camera camera(Vector3(0,0,10));

    // Create permanent Resources
    Renderer::ImageDesc mainColorDesc;
    mainColorDesc.debugName = "MainColor";
    mainColorDesc.dimensions = Vector2i(1280, 720);
    mainColorDesc.format = Renderer::IMAGE_FORMAT_R16G16B16A16_FLOAT;
    mainColorDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

    Renderer::ImageID mainColor = renderer->CreateImage(mainColorDesc);

    Renderer::DepthImageDesc mainDepthDesc;
    mainDepthDesc.debugName = "MainDepth";
    mainDepthDesc.dimensions = Vector2i(1280, 720);
    mainDepthDesc.format = Renderer::DEPTH_IMAGE_FORMAT_D32_FLOAT;
    mainDepthDesc.sampleCount = Renderer::SAMPLE_COUNT_1;

    Renderer::DepthImageID mainDepth = renderer->CreateDepthImage(mainDepthDesc);

    Renderer::RenderLayer& mainLayer = renderer->GetRenderLayer(MAIN_RENDER_LAYER);

    Renderer::MaterialDesc materialDesc;
    materialDesc.path = "Data/materials/DebugBasicPBS.material";
    Renderer::MaterialID material = renderer->LoadMaterial(materialDesc);
    
    Renderer::ModelDesc modelDesc;
    modelDesc.path = "Data/models/cube.model";

    Renderer::ModelID cubeModel = renderer->LoadModel(modelDesc);
    Renderer::InstanceData cubeInstance;
    cubeInstance.modelMatrix = Matrix(); // Move, rotate etc the model here

    mainLayer.RegisterModel(material, cubeModel, &cubeInstance); // This registers a cube model to be drawn in this layer with cubeInstance's model constantbuffer

    // ViewConstantBuffer will be a constant buffer which holds information about our Camera, like our View and Projection matrices
    struct ViewConstantBuffer
    {
        Matrix viewMatrix; // 64 bytes
        Matrix projMatrix; // 64 bytes

        float padding[128] = {};
    };

    Renderer::ConstantBuffer<ViewConstantBuffer> viewConstantBuffer = renderer->CreateConstantBuffer<ViewConstantBuffer>();

    // Set viewConstantBuffer
    {
        Matrix& projMatrix = viewConstantBuffer.resource.projMatrix;

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
        viewConstantBuffer.Apply(1);
    }

    struct ModelConstantBuffer
    {
        Vector4 colorMultiplier; // 16 bytes
        Matrix modelMatrix; // 64 bytes

        float padding[128] = {};
    };

    Renderer::ConstantBuffer<ModelConstantBuffer> modelConstantBuffer = renderer->CreateConstantBuffer<ModelConstantBuffer>();
    Memory::StackAllocator frameAllocator(FRAME_ALLOCATOR_SIZE);
    frameAllocator.Init();

    Timer timer;
    f32 targetDelta = 1.0f / TARGET_UPDATE_RATE;
    u32 frameIndex = 0;
    while (true)
    {
        f32 deltaTime = timer.GetDeltaTime();
        timer.Tick();

        frameAllocator.Reset(); // Reset the frame allocator at the start of every frame

        if (mainWindow.WantsToExit())
        {
            mainWindow.ConfirmExit();
            break;
        }
        mainWindow.Update(deltaTime);

        // Lets update our camera and the view constantbuffer
        camera.Update(deltaTime);
        viewConstantBuffer.resource.viewMatrix = camera.GetViewMatrix().Transposed();
        viewConstantBuffer.Apply(frameIndex);

        // Register models to be rendered
        mainLayer.RegisterModel(material, cubeModel, &cubeInstance);

        // Create a framegraph
        Renderer::RenderGraphDesc renderGraphDesc;
        renderGraphDesc.allocator = &frameAllocator;
        Renderer::RenderGraph renderGraph = renderer->CreateRenderGraph(renderGraphDesc);

        // Depth Prepass
        {
            struct DepthPrepassData
            {
                Renderer::RenderPassMutableResource depth;
            };

            renderGraph.AddPass<DepthPrepassData>("Depth Prepass",
            [&](DepthPrepassData& data, Renderer::RenderGraphBuilder& builder) // Setup runs singlethreaded first, here we register what resources we're going to use
            { 
                data.depth = builder.Write(mainDepth, Renderer::RenderGraphBuilder::WriteMode::WRITE_MODE_RENDERTARGET, Renderer::RenderGraphBuilder::LoadMode::LOAD_MODE_CLEAR);

                return true; // Return true from setup to enable this pass, return false to disable it
            },
            [&](DepthPrepassData& data, Renderer::CommandList& commandList) // Execute will run parallel, here we build an API-agnostic commandlist (which will be merged and executed afterwards)
            {
                Renderer::GraphicsPipelineDesc pipelineDesc;
                renderGraph.InitializePipelineDesc(pipelineDesc);

                // Shaders
                Renderer::VertexShaderDesc vertexShaderDesc;
                vertexShaderDesc.path = "Data/shaders/depthPrepass.vs.hlsl.cso";
                pipelineDesc.states.vertexShader = renderer->LoadShader(vertexShaderDesc); // This will load shader or use cached loaded shader

                // Constant buffers  TODO: Improve on this, if I set state 0 and 3 it won't work etc...
                pipelineDesc.states.constantBufferStates[0].enabled = true; // ViewCB
                pipelineDesc.states.constantBufferStates[0].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_VERTEX;
                pipelineDesc.states.constantBufferStates[1].enabled = true; // ModelCB
                pipelineDesc.states.constantBufferStates[1].shaderVisibility = Renderer::ShaderVisibility::SHADER_VISIBILITY_VERTEX;

                // Input layouts TODO: Improve on this, if I set state 0 and 3 it won't work etc... Maybe responsibility for this should be moved to ModelHandler and the cooker?
                pipelineDesc.states.inputLayouts[0].enabled = true;
                pipelineDesc.states.inputLayouts[0].SetName("POSITION");
                pipelineDesc.states.inputLayouts[0].format = Renderer::InputFormat::INPUT_FORMAT_R32G32B32_FLOAT;
                pipelineDesc.states.inputLayouts[0].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;
                pipelineDesc.states.inputLayouts[1].enabled = true;
                pipelineDesc.states.inputLayouts[1].SetName("NORMAL");
                pipelineDesc.states.inputLayouts[1].format = Renderer::InputFormat::INPUT_FORMAT_R32G32B32_FLOAT;
                pipelineDesc.states.inputLayouts[1].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;
                pipelineDesc.states.inputLayouts[2].enabled = true;
                pipelineDesc.states.inputLayouts[2].SetName("TEXCOORD");
                pipelineDesc.states.inputLayouts[2].format = Renderer::InputFormat::INPUT_FORMAT_R32G32_FLOAT;
                pipelineDesc.states.inputLayouts[2].inputClassification = Renderer::InputClassification::INPUT_CLASSIFICATION_PER_VERTEX;

                // Depth state
                pipelineDesc.states.depthStencilState.depthEnable = true;
                pipelineDesc.states.depthStencilState.depthWriteEnable = true;
                pipelineDesc.states.depthStencilState.depthFunc = Renderer::ComparisonFunc::COMPARISON_FUNC_LESS;

                // Rasterizer state
                pipelineDesc.states.rasterizerState.cullMode = Renderer::CullMode::CULL_MODE_BACK;

                // Render targets
                pipelineDesc.depthStencil = data.depth;

                // Set pipeline
                Renderer::GraphicsPipelineID pipeline = renderer->CreatePipeline(pipelineDesc); // This will compile the pipeline and return the ID, or just return ID of cached pipeline
                commandList.SetPipeline(pipeline);

                // Set viewport and scissor rect
                commandList.SetScissorRect(0, width, 0, height);
                commandList.SetViewport(0, 0, static_cast<f32>(width), static_cast<f32>(height), 0.0f, 1.0f);

                // Set view constant buffer
                commandList.SetConstantBuffer(0, viewConstantBuffer.GetGPUResource(frameIndex));

                // Clear depth to 1.0f (furthest from camera)
                commandList.Clear(mainDepth, 1.0f);

                // Render main layer
                Renderer::RenderLayer& mainLayer = renderer->GetRenderLayer(MAIN_RENDER_LAYER);

                for (auto const& material : mainLayer.GetMaterials())
                {
                    auto const& models = material.second;

                    for (auto const& model : models)
                    {
                        auto const& modelID = Renderer::ModelID(model.first);
                        auto const& instances = model.second;

                        for (auto const& instance : instances)
                        {
                            // Update model constant buffer
                            modelConstantBuffer.resource.modelMatrix = instance->modelMatrix;
                            modelConstantBuffer.resource.colorMultiplier = instance->colorMultiplier;
                            modelConstantBuffer.Apply(frameIndex);

                            // Set model constant buffer
                            commandList.SetConstantBuffer(1, modelConstantBuffer.GetGPUResource(frameIndex));

                            // Draw
                            commandList.Draw(modelID);
                        }
                    }
                }
            });
        }

        // Main Pass
        {
            struct MainPassData
            {
                Renderer::RenderPassMutableResource mainColor;
                Renderer::RenderPassMutableResource depth;
            };

            renderGraph.AddPass<MainPassData>("Main Pass",
                [&](MainPassData& data, Renderer::RenderGraphBuilder& builder) // Setup
                {
                    data.mainColor = builder.Write(mainColor, Renderer::RenderGraphBuilder::WriteMode::WRITE_MODE_RENDERTARGET, Renderer::RenderGraphBuilder::LoadMode::LOAD_MODE_CLEAR);
                    data.depth = builder.Write(mainDepth, Renderer::RenderGraphBuilder::WriteMode::WRITE_MODE_RENDERTARGET, Renderer::RenderGraphBuilder::LoadMode::LOAD_MODE_LOAD); // TODO: Should this one be Read? Maybe?

                    return true; // Return true from setup to enable this pass, return false to disable it
                },
                [&](MainPassData& data, Renderer::CommandList& commandList) // Execute
                {
                    // Clear mainColor TODO: This should be handled by the parameter in Setup, and it should definitely not act on ImageID and DepthImageID
                    commandList.Clear(mainColor, Vector4(0, 0, 0, 1));

                    // Render main layer
                    Renderer::RenderLayer& mainLayer = renderer->GetRenderLayer(MAIN_RENDER_LAYER);

                    for (auto const& material : mainLayer.GetMaterials())
                    {
                        // Set Material pipeline
                        auto const& materialID = Renderer::MaterialID(material.first);

                        Renderer::MaterialPipelineDesc pipelineDesc;
                        renderGraph.InitializePipelineDesc(pipelineDesc);

                        pipelineDesc.material = materialID;

                        // Depth state
                        pipelineDesc.states.depthStencilState.depthEnable = true;
                        pipelineDesc.states.depthStencilState.depthFunc = Renderer::ComparisonFunc::COMPARISON_FUNC_EQUAL;

                        // Rasterizer state
                        pipelineDesc.states.rasterizerState.cullMode = Renderer::CullMode::CULL_MODE_BACK;

                        // Render targets
                        pipelineDesc.renderTargets[0] = data.mainColor;
                        pipelineDesc.depthStencil = data.depth;

                        // Set pipeline
                        Renderer::MaterialPipelineID pipeline = renderer->CreatePipeline(pipelineDesc); // This will compile the pipeline and return the ID, or just return ID of cached pipeline
                        commandList.SetPipeline(pipeline);

                        // Set viewport and scissor rect
                        commandList.SetScissorRect(0, width, 0, height);
                        commandList.SetViewport(0, 0, static_cast<f32>(width), static_cast<f32>(height), 0.0f, 1.0f);

                        // Set view constant buffer
                        commandList.SetConstantBuffer(0, viewConstantBuffer.GetGPUResource(frameIndex));

                        auto const& models = material.second;

                        for (auto const& model : models)
                        {
                            auto const& modelID = Renderer::ModelID(model.first);
                            auto const& instances = model.second;

                            for (auto const& instance : instances)
                            {
                                // Update model constant buffer
                                modelConstantBuffer.resource.modelMatrix = instance->modelMatrix;
                                modelConstantBuffer.resource.colorMultiplier = instance->colorMultiplier;
                                modelConstantBuffer.Apply(frameIndex);

                                // Set model constant buffer
                                commandList.SetConstantBuffer(1, modelConstantBuffer.GetGPUResource(frameIndex));

                                // Draw
                                commandList.Draw(modelID);
                            }
                        }
                    }
            });
        }

        // Setup and Execute RenderGraph
        renderGraph.Setup();
        renderGraph.Execute();

        // Present to Window
        renderer->Present(&mainWindow, mainColor);
        //renderer->Present(&mainWindow, mainDepth);

        // Reset layers
        mainLayer.Reset();

        // Wait for update rate, this might be an overkill implementation but it has the most even update rate I've seen - MPursche
        for (deltaTime = timer.GetDeltaTime(); deltaTime < targetDelta - 0.0025f; deltaTime = timer.GetDeltaTime())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (deltaTime = timer.GetDeltaTime(); deltaTime < targetDelta; deltaTime = timer.GetDeltaTime())
        {
            std::this_thread::yield();
        }

        frameIndex = !frameIndex; // Flip between 0 and 1
    }

    renderer->Deinit();
    delete renderer;
    return 0;
}