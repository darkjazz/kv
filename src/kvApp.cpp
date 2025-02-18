#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "osc.h"


using namespace ci;
using namespace ci::app;
using namespace std;


class kvApp : public App {
  public:
    void prepareSettings(Settings* settings);
    void setup() override;
    void resize() override;
    void keyDown( KeyEvent event ) override;
    void update() override;
    void draw() override;
    void shutdown();
    
    OSCMessenger *oscMessenger;
    World *world;
    GraphicsRenderer *ogl;
    Rule *rule;
//    Boids *boids;

private:
    int _winSizeX, _winSizeY;
    int _frameRate, _inport, _outport, _windowMode, _fullScreen;
    std::string _remoteHost;
};

void kvApp::prepareSettings(Settings* settings) {
    auto displays = Display::getDisplays();
    
    // Check if a secondary display is available
    DisplayRef targetDisplay;
    if (displays.size() > 1) {
        targetDisplay = displays[1]; // Select the second display
    } else {
        targetDisplay = Display::getMainDisplay(); // Fallback to main display
    }
    
    auto targetSize = targetDisplay->getSize();
    
    _winSizeX = 1024; // Default width
    _winSizeY = 768;  // Default height
    _frameRate = 32;
    _remoteHost = "127.0.0.1";
    _inport = 7000;
    _outport = 57120;
    _windowMode = 0;
    _fullScreen = 0;

    // Use App::getCommandLineArgs() to retrieve command-line arguments
    std::vector<std::string> args = getCommandLineArgs();
    for (size_t i = 1; i < args.size(); i += 2) {
        if (args[i] == "-screenx") {
            _winSizeX = atoi(args[i + 1].c_str());
        } else if (args[i] == "-screeny") {
            _winSizeY = atoi(args[i + 1].c_str());
        } else if (args[i] == "-fps") {
            _frameRate = atoi(args[i + 1].c_str());
        } else if (args[i] == "-remote") {
            _remoteHost = args[i + 1];
        } else if (args[i] == "-inport") {
            _inport = atoi(args[i + 1].c_str());
        } else if (args[i] == "-outport") {
            _outport = atoi(args[i + 1].c_str());
        } else if (args[i] == "-wmode") {
            _windowMode = atoi(args[i + 1].c_str());
        } else if (args[i] == "-full") {
            _fullScreen = atoi(args[i + 1].c_str());
        }
    }

    if (_fullScreen > 0) {
        // Fullscreen: Match window size to the screen size
        settings->setFullScreen();
        _winSizeX = targetSize.x;
        _winSizeY = targetSize.y;
    } else {
        // Position window on the secondary display or fallback to main display
        if (_windowMode == 0)
            settings->setWindowPos(targetDisplay->getBounds().x1, targetDisplay->getBounds().y1);
        else
            settings->setWindowPos(targetDisplay->getBounds().x2 - _winSizeX, targetDisplay->getBounds().y2 - _winSizeY);
    }

    settings->setWindowSize(_winSizeX, _winSizeY);
    settings->setBorderless(true);
    settings->setFrameRate(_frameRate);
}

void kvApp::resize() {
    ogl->reshape();
}

void kvApp::setup()
{
    oscMessenger = new OSCMessenger(_remoteHost, _outport, _inport);
    world = new World();
    ogl = new GraphicsRenderer(world);
    oscMessenger->setOgl(ogl);
    oscMessenger->setWorld(world);
    
    ogl->setupOgl();
}

void kvApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'f' || event.getChar() == 'F' ) {
        setFullScreen( ! isFullScreen() );
        if (isFullScreen()) { hideCursor(); }
        else { showCursor(); }
    }
    if( event.getChar() == 'i' || event.getChar() == 'I' ) {
        
    }

}

void kvApp::update()
{
    ogl->update();
}

void kvApp::draw()
{
    
    ogl->startDraw();
        
    if (world->initialized()) {
        
        int x, y, z;
        world->prepareNext();
        
        for (x = 0; x < world->sizeX(); x++) {
            for (y = 0; y < world->sizeY(); y++) {
                for (z = 0; z < world->sizeZ(); z++) {
                    
                    if (world->ruleInitialized)
                        world->next(x, y, z);
                    if (world->somActivated)
                        world->nextSOM(x, y, z);

                    ogl->drawFragment(&world->cells[x][y][z]);
                }
            }
        }

        world->finalizeNext();
                
    }
            
    ogl->endDraw();
}

void kvApp::shutdown() {
    delete ogl;
    delete world;
}

CINDER_APP( kvApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
