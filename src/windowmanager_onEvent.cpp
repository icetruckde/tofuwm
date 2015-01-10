#include <iostream>
#include <sstream>
#include <cmath>
#include <X11/Xlib.h>
#include "windowmanager.h"


using namespace std;


void WindowManager::onMapRequest() {
    shared_ptr <WmWindow> w(make_shared<WmWindow>(displayPtr.get(), root, event.xmaprequest.window));
    windows.insert(make_pair(w->window, w));
    windows.insert(make_pair(w->frame, w));

    w->setWorkspace(currentWorkspace);
    w->setDefaultEventMask();

    stringstream ss;
    ss << desktop.x << ":" << desktop.w << ":" << desktop.y << ":" << desktop.h;
    addDebugText(ss.str());

    XWindowAttributes attributes;
    XGetWindowAttributes(displayPtr.get(), w->frame, &attributes);
    w->relocate(desktop.x + (desktop.w - attributes.width)/2,
                desktop.y + (desktop.h - attributes.height)/2,
                attributes.width, attributes.height);

    XMapWindow(displayPtr.get(), w->window);
    w->show();

    if (currentWindow == 0)
        currentWindow = w.get();
}

void WindowManager::onKeyPress() {
    if (event.xkey.state & keyGrabber.defaultModifier()) {
        if (event.xkey.state & ShiftMask) {
            if (event.xkey.keycode == keyGrabber.keyClose()) {
                addDebugText("WINDOW CLOSE");
                if (currentWindow)
                    currentWindow->close();
                currentWindow = 0; // @TODO: Select next window by mouse position
            }
        } else {
            if (event.xkey.keycode == keyGrabber.keyWorkspace1()) {
                addDebugText("WORKSPACE 1");
                changeWorkspace(0);
            } else if (event.xkey.keycode == keyGrabber.keyWorkspace2()) {
                addDebugText("WORKSPACE 2");
                changeWorkspace(1);
            } else if (event.xkey.keycode == keyGrabber.keyDMenu()) {
                addDebugText("DMENU SPAWN");
                char *const parmList[] = {(char *) "dmenu_run", 0};
                spawn("/usr/bin/dmenu_run", parmList);
            }
        }
    }
}

void WindowManager::onKeyRelease() {
}

void WindowManager::onButtonPress() {
    if (event.xbutton.state & keyGrabber.defaultModifier()) {
        if (event.xbutton.subwindow) {
            moveWindow = findWindow(event.xbutton.subwindow);
            if (!moveWindow)
                return;
            currentWindow = moveWindow;
            currentWindow->setActive(true);
            XGetWindowAttributes(displayPtr.get(), moveWindow->frame, &moveWindowAttributes);
            moveWindowStart = event.xbutton;
            XRaiseWindow(displayPtr.get(), moveWindow->frame);
            if (moveWindowStart.button == 3) { // Right mouse
                moveWindowExpandXPositive = moveWindowStart.x_root >= moveWindowAttributes.x + moveWindowAttributes.width / 2;
                moveWindowExpandYPositive = moveWindowStart.y_root >= moveWindowAttributes.y + moveWindowAttributes.height / 2;
            }
        }
    }
}

void WindowManager::onButtonRelease() {
    moveWindowStart.subwindow = None;
    moveWindow = 0;
}

void WindowManager::onEnter() {
    setCurrentWindow(event.xcrossing.window);
    currentWindow->setActive(true);
}

void WindowManager::onLeave() {
    if (!currentWindow)
        return;

    currentWindow->setActive(false);
    setCurrentWindow(0);
}

void WindowManager::onMotion() {
    if (event.xbutton.state & keyGrabber.defaultModifier()) {
        if (moveWindow) {
            int xdiff = event.xbutton.x_root - moveWindowStart.x_root;
            int ydiff = event.xbutton.y_root - moveWindowStart.y_root;
            if (moveWindowStart.button == 1) { // Left mouse
                moveWindow->relocate(moveWindowAttributes.x + xdiff, moveWindowAttributes.y + ydiff, moveWindowAttributes.width, moveWindowAttributes.height);
            } else if (moveWindowStart.button == 3) { // Right mouse
                moveWindow->relocate(moveWindowAttributes.x + (moveWindowExpandXPositive ? 0 : xdiff), moveWindowAttributes.y + (moveWindowExpandYPositive ? 0 : ydiff),
                    moveWindowAttributes.width + (moveWindowExpandXPositive ? xdiff : -xdiff), moveWindowAttributes.height + (moveWindowExpandYPositive ? ydiff : -ydiff));
            }
        }
    }
}

void WindowManager::onConfigureRequest() {
}

void WindowManager::onCirculateRequest() {
}