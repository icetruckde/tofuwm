#include <sstream>
#include "keygrabber.h"
#include "windowmanager/windowmanager.h"


using namespace std;


KeyGrabber::KeyGrabber(WindowManager* wm, int workspaceCount) :
wm(wm),
workspaceCount(workspaceCount)
{
    const unsigned int defaultModifier = Mod4Mask;

    // Move active window
    hotbuttons.emplace_back(wm->display, wm->root,
            1, defaultModifier, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            [=] { wm->onMousePress(); },
            [=] { wm->onMouseRelease(); },
            [=] { wm->onMouseMotion(); });
    // Resize active window
    hotbuttons.emplace_back(wm->display, wm->root,
            3, defaultModifier, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            [=] { wm->onMousePress(); },
            [=] { wm->onMouseRelease(); },
            [=] { wm->onMouseMotion(); });

    // Spawn dmenu
    hotkeys.emplace_back(wm->display, wm->root,
            key("d"), defaultModifier,
            [=] {
                wm->addDebugText("DMENU SPAWN");
                char *const parmList[] = {(char *) "dmenu_run", 0};
                wm->spawn("/usr/bin/dmenu_run", parmList);
            }, nullptr);
    // Close active window
    hotkeys.emplace_back(wm->display, wm->root,
            key("q"), defaultModifier | ShiftMask,
            [=] {
                wm->addDebugText("WINDOW CLOSE");
                if (wm->currentWindow)
                    wm->currentWindow->close();
                wm->selectNewCurrentWindow();
            }, nullptr);

    const char* const workspaceKeys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
    for (int i = 0; i < workspaceCount; ++i) {
        hotkeys.emplace_back(wm->display, wm->root,
                key(workspaceKeys[i]), defaultModifier,
                [=] {
                    stringstream ss;
                    ss << "WORKSPACE " << (i+1);
                    wm->addDebugText(ss.str());
                    wm->changeWorkspace(i);
                }, nullptr);
    }
}

KeyGrabber::~KeyGrabber() {
}

unsigned int KeyGrabber::key(const std::string& character) {
    return XKeysymToKeycode(wm->display, XStringToKeysym(character.c_str()));
}
