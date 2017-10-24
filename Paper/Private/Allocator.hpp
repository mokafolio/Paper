#ifndef PAPER_PRIVATE_ALLOCATOR_HPP
#define PAPER_PRIVATE_ALLOCATOR_HPP

#include <Stick/Allocators/LinearAllocator.hpp>
#include <Stick/Allocators/Mallocator.hpp>
#include <Stick/Allocators/PoolAllocator.hpp>
#include <Stick/Allocators/FallbackAllocator.hpp>
#include <Stick/Allocators/FreeListAllocator.hpp>
#include <Stick/Allocators/GlobalAllocator.hpp>
#include <Stick/Allocators/Bucketizer.hpp>
#include <Stick/Allocators/Segregator.hpp>

namespace paper
{
    namespace detail
    {
        // using MainAllocator = stick::mem::GlobalAllocator <
        //                       stick::mem::FallbackAllocator<stick::mem::FreeListAllocator<stick::mem::Mallocator, 12582912>,
        //                       stick::mem::Mallocator>
        //                       >;

        using MainAllocator = stick::mem::GlobalAllocator <
                              stick::mem::Mallocator
                              >;

        template<stick::Size BucketCount>
        using PoolAllocator = stick::mem::PoolAllocator<MainAllocator,  stick::mem::DynamicSizeFlag,  stick::mem::DynamicSizeFlag, BucketCount>;
        using SmallAllocator = stick::mem::PoolAllocator<MainAllocator, 0, 8, 1024>;

        using PaperAllocator = stick::mem::Segregator <
                               stick::mem::T<8>, SmallAllocator,
                               stick::mem::T<128>, stick::mem::Bucketizer<PoolAllocator<1024>, 1, 128, 16>,
                               stick::mem::T<256>, stick::mem::Bucketizer<PoolAllocator<1024>, 129, 256, 32>,
                               stick::mem::T<512>, stick::mem::Bucketizer<PoolAllocator<1024>, 257, 512, 64>,
                               stick::mem::T<1024>, stick::mem::Bucketizer<PoolAllocator<1024>, 513, 1024, 128>,
                               stick::mem::T<2048>, stick::mem::Bucketizer<PoolAllocator<1024>, 1025, 2048, 256>,
                               stick::mem::T<4096>, stick::mem::Bucketizer<PoolAllocator<1024>, 2049, 4096, 512>,
                               MainAllocator>;

        class STICK_API DefaultPaperAllocator : public stick::Allocator
        {
        public:

            inline stick::mem::Block allocate(stick::Size _byteCount, stick::Size _alignment) override
            {
                return m_alloc.allocate(_byteCount, _alignment);
            }

            inline void deallocate(const stick::mem::Block & _block) override
            {
                m_alloc.deallocate(_block);
            }

        private:

            PaperAllocator m_alloc;
        };
    }
}

#endif //PAPER_PRIVATE_ALLOCATOR_HPP
