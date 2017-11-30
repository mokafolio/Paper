// This example shows the very basic steps to create a Paper Document
// and draw it.

// we use GLFW to open a simple window
#include <GLFW/glfw3.h>

//include some paper headers.
#include <Paper/Document.hpp>
#include <Paper/CurveLocation.hpp>

//we want to use the OpenGL renderer, so we include it.
#include <Paper/OpenGL/GLRenderer.hpp>

#include <Crunch/Randomizer.hpp>

#include <Stick/Thread.hpp>
#include <Stick/FileUtilities.hpp>


//we want to use the paper, brick, crunch & stick namespaces
using namespace paper; // paper namespace
using namespace brick; // brick namespace for entity / component things
using namespace crunch; // crunch namespace for math
using namespace stick; // stick namespace for core data structures/containers etc.

int main(int _argc, const char * _args[])
{
    // initialize glfw
    if (!glfwInit())
        return EXIT_FAILURE;

    // and set some hints to get the correct opengl versions/profiles
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create the window
    GLFWwindow * window = glfwCreateWindow(800, 600, "Hello Paper Example", NULL, NULL);
    if (window)
    {
        glfwMakeContextCurrent(window);

        // the brick entity hub to use by the paper document.
        Hub hub;
        // create the document.
        Document doc = createDocument(hub);
        doc.setSize(800, 600);
        // create the opengl renderer for the document
        opengl::GLRenderer renderer(doc);

        LinearGradient grad;
        grad.setOrigin(Vec2f(200, 0));
        grad.setDestination(Vec2f(100, 200));
        grad.addStop(ColorRGBA(1.0, 0.0, 0.0, 1.0), 0.25);
        grad.addStop(ColorRGBA(1.0, 1.0, 0.0, 1.0), 0.75);

        Path circle = doc.createCircle(Vec2f(100, 100), 50);
        circle.setFill(grad);
        circle.setStroke(ColorRGBA(0.0, 0.0, 1.0, 1.0));
        circle.setStrokeWidth(10.0);
        // circle.setFill("red");


        // Vec2f dir = normalize(grad.destination() - grad.origin());
        // Vec2f perp(-dir.y, dir.x);
        // Line2f line(grad.origin(), dir);
        // LineSegment2f segment(grad.origin(), grad.destination());

        // auto angle = toDegrees(std::atan2(dir.y, dir.x));
        // auto side = line.side(circle.position());
        // printf("ANGLE %f SIDE %i\n", angle, side);
        // Rect bounds = circle.bounds();

        // LineSegment2f shifted = segment;
        // if (bounds.contains(grad.origin()) || bounds.contains(grad.destination()) ||
        //         intersect(bounds.leftSegment(), segment) ||
        //         intersect(bounds.topSegment(), segment) ||
        //         intersect(bounds.rightSegment(), segment) ||
        //         intersect(bounds.bottomSegment(), segment))
        // {
        //     auto dig = bounds.diagonal();
        //     auto ddir = segment.side(circle.position()) == -1 ? perp : -perp;
        //     shifted = LineSegment2f(shifted.positionOne() + ddir * dig, shifted.positionTwo() + ddir * dig);
        // }

        // Path rct = doc.createPath();
        // Float len = length(grad.destination() - grad.origin());
        // Float dig = std::max(bounds.diagonal(), len)  * 0.5;
        // Vec2f a = bounds.center() - dir * dig + perp * dig;
        // Vec2f b = bounds.center() + dir * dig + perp * dig;
        // Vec2f c = bounds.center() + dir * dig - perp * dig;
        // Vec2f d = bounds.center() - dir * dig - perp * dig;
        // rct.setFill(ColorRGBA(1.0, 1.0, 0.5, 0.5));
        // rct.addPoint(a);
        // rct.addPoint(b);
        // rct.addPoint(c);
        // rct.addPoint(d);

        // Path origin = doc.createCircle(grad.origin(), 3);
        // origin.setFill("green");

        // Path dest = doc.createCircle(grad.destination(), 3);
        // dest.setFill("blue");

        // for (auto & stop : grad.stops())
        // {
        //     Path c = doc.createCircle(grad.origin() + dir * len * stop.offset , 3);
        //     c.setFill(stop.color);
        // }

        // Path brect = doc.createRectangle(bounds.min(), bounds.max());
        // brect.setFill(ColorRGBA(0, 0, 0, 0.1));

        // Path shiftedPath = doc.createPath();
        // shiftedPath.setStroke("black");
        // shiftedPath.addPoint(shifted.positionOne());
        // shiftedPath.addPoint(shifted.positionTwo());

        // the main loop
        while (!glfwWindowShouldClose(window))
        {
            // clear the background to black
            glClearColor(200, 2000, 2000, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // get the window size from the glfw window
            // and set it as the viewport on the renderer
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer.setViewport(width, height);
            renderer.setTransform(Mat3f::identity());

            auto err = renderer.draw();
            if (err)
            {
                printf("ERROR: %s\n", err.message().cString());
                return EXIT_FAILURE;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    else
    {
        glfwTerminate();
        printf("Could not open GLFW window :(\n");
        return EXIT_FAILURE;
    }

    // clean up glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
