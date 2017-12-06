#ifndef PAPER_RENDER_RENDERER_HPP
#define PAPER_RENDER_RENDERER_HPP

namespace paper
{
	namespace render
	{
		class RenderInterface;

		class STICK_API Renderer
		{
		public:

			Renderer();

			Renderer(Document _document, RenderInterface * _renderEngine);

			stick::Error draw();
		};
	}
}

#endif //PAPER_RENDER_RENDERER_HPP
