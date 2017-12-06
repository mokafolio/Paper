#ifndef PAPER_RENDER_RENDERINTERFACE_HPP
#define PAPER_RENDER_RENDERINTERFACE_HPP

namespace paper
{
    namespace render
    {
    	//1. Render all consecutive static paths onto one layer.
    	//2. Render animated paths onto their own layers.
    	//3. Composite all layers.
        class STICK_API RenderInterface
        {
        public:

            //these have to be implemented
            virtual stick::Error drawPath(Path _path, const Mat3f * _transform) = 0;
            virtual stick::Error beginClipping(Path _clippingPath, const Mat3f * _transform) = 0;
            virtual stick::Error endClipping(Path _clippingPath, const Mat3f * _transform) = 0;

            //these can be implemented
            virtual stick::Error prepareDrawing() { return stick::Error(); }
            virtual stick::Error finishDrawing() { return stick::Error(); }

            stick::Error drawChildren(Item _item, const Mat3f * _transform);
            stick::Error drawItem(Item _item, const Mat3f * _transform);
        };
    }
}

#endif //PAPER_RENDER_RENDERINTERFACE_HPP
