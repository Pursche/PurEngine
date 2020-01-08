#pragma once

namespace Renderer
{
    namespace Backend
    {
        struct ConstantBufferBackend
        {
            virtual void Apply(u32 frameIndex, void* data, size_t size) = 0;
        };
    }

    template <typename T>
    struct ConstantBuffer
    {
        T resource;

        constexpr size_t GetSize()
        {
            return sizeof(T);
        }

        void Apply(u32 frameIndex)
        {
            backend->Apply(frameIndex, &resource, GetSize());
        }

        Backend::ConstantBufferBackend* backend;

    protected:
        ConstantBuffer() {}; // This has to be created through Renderer::CreateConstantBuffer<T>

        friend class Renderer;
    };
}