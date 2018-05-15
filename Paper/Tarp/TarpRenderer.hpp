#ifndef PAPER_TARP_TARPRENDERER_HPP
#define PAPER_TARP_TARPRENDERER_HPP

#include <Paper/RenderInterface.hpp>

namespace paper
{
    namespace tarp
    {
        namespace detail
        {
            struct TarpStuff;
        };

        class STICK_API TarpRenderer : public RenderInterface
        {
        public:

            TarpRenderer();

            ~TarpRenderer();


            stick::Error init(Document _doc) final;

            void setViewport(Float _x, Float _y, Float _widthInPixels, Float _heightInPixels) final;

            void setProjection(const Mat4f & _projection) final;

            void reserveItems(stick::Size _count) final;

            stick::Error drawPath(Path _path, const Mat3f & _transform) final;

            stick::Error beginClipping(Path _clippingPath, const Mat3f & _transform) final;

            stick::Error endClipping() final;

            stick::Error prepareDrawing() final;

            stick::Error finishDrawing() final;


        private:

            stick::UniquePtr<detail::TarpStuff> m_tarp;
            Rect m_viewport;
        };
    }
}

#endif //PAPER_TARP_TARPRENDERER_HPP

//@TODO: Ideas for future rendering improvements:
//1. Render all consecutive static paths onto one layer.
//2. Render animated paths onto their own layers.
//3. Composite all layers.
//=======================================================
