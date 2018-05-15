// This example shows the very basic steps to create a Paper Document
// and draw it.

// we use GLFW to open a simple window
#include <GLFW/glfw3.h>

//include some paper headers.
#include <Paper/Document.hpp>
#include <Paper/CurveLocation.hpp>

#include <Paper/Tarp/TarpRenderer.hpp>

#include <Crunch/Randomizer.hpp>
#include <Crunch/MatrixFunc.hpp>

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
        tarp::TarpRenderer renderer;
        auto err = renderer.init(doc);
        if(err)
        {
            printf("Error %s\n", err.message().cString());
            return EXIT_FAILURE;
        }


        // Randomizer r;

        // LinearGradient grad = doc.createLinearGradient(Vec2f(r.randomf(100, 700), r.randomf(100, 500)),
        //                       Vec2f(r.randomf(100, 700), r.randomf(100, 500)));

        // for (int i = 0; i < r.randomi(2, 10); ++i)
        // {
        //     grad.addStop(ColorRGBA(r.randomf(0, 1), r.randomf(0, 1), r.randomf(0, 1), 1.0), r.randomf(0, 1));
        // }

        Path circle = doc.createCircle(Vec2f(400, 300), 50);
        circle.setFill(ColorRGBA(1, 0, 0, 1));
        circle.setStroke(ColorRGBA(0.0, 1.0, 1.0, 1.0));
        circle.setStrokeWidth(10.0);
        circle.setFill("red");


        // the main loop
        while (!glfwWindowShouldClose(window))
        {
            // clear the background to black
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // get the window size from the glfw window
            // and set it as the viewport on the renderer
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer.setViewport(0, 0, width, height);

            glfwGetWindowSize(window, &width, &height);
            renderer.setProjection(Mat4f::ortho(0, width, height, 0, -1, 1));
            // renderer.setTransform(Mat3f::identity());

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
