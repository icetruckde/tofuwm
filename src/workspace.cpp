#include <containers/wmsplitter.h>
#include "workspace.h"
#include "windowmanager/windowmanager.h"


using namespace std;


Workspace::Workspace(WindowManager* wm) :
wm(wm)
{
}

Workspace::~Workspace() {
}

WorkspaceMode Workspace::workspaceMode() {
    return workspaceMode_;
}

void Workspace::workspaceMode(WorkspaceMode workspaceMode) {
    workspaceMode_ = workspaceMode;
}

void Workspace::hide() {
    for(WmWindow* w : windows)
        w->hide();
}

void Workspace::show() {
    for(WmWindow* w : windows)
        w->show();
}

void Workspace::addWindow(WmWindow* w) {
    if (workspaceMode_ == WorkspaceMode::Floating || w->staysFloating()) {
        w->windowMode = WindowMode::Floating;
        floatingWindows.push_back(w);
    } else {
        w->windowMode = WindowMode::Tiled;
        addWindowToTiling(w);
        windows.push_back(w);
        lastActiveTiledWindow = w;
    }
    w->show();
}

std::shared_ptr<WmContainer> Workspace::createNewContainer() {
    std::shared_ptr<WmContainer> container;
    switch (workspaceMode_) {
        case WorkspaceMode::HorizontalTiling:
            container = make_shared<WmSplitter>(WmSplitterType::Horizontal);
            break;
        case WorkspaceMode::VerticalTiling:
            container = make_shared<WmSplitter>(WmSplitterType::Vertical);
            break;
    }
    Geometry& g = container->geometry();
    g.x = wm->desktop.x;
    g.y = wm->desktop.y;
    g.w = wm->desktop.w;
    g.h = wm->desktop.h;
    return container;
}

void Workspace::addWindowToTiling(WmWindow* w) {
    WmContainer* container = 0;
    if (!lastActiveTiledWindow && windows.size() > 0)
        lastActiveTiledWindow = windows.back();
    if (lastActiveTiledWindow) {
        container = lastActiveTiledWindow->container;
    } else {
        rootContainer = createNewContainer();
        container = rootContainer.get();
    }
    container->add(w->shared());
    container->realign();
    // @TODO: recalculate tiles and reposition windows
}

void Workspace::toggleWindowMode(WmWindow* w) {
    if (w->windowMode == WindowMode::Floating) {
        w->windowMode = WindowMode::Tiled;
        floatingWindows.remove(w);
        addWindowToTiling(w);
        windows.push_back(w);
        lastActiveTiledWindow = w;
    } else {
        w->windowMode = WindowMode::Floating;
        windows.remove(w);
        floatingWindows.push_back(w);
    }
    w->show();
}

void Workspace::removeWindow(WmWindow* w) {
    switch (w->windowMode) {
        case WindowMode::Floating:
            floatingWindows.remove(w); break;
        case WindowMode::Tiled:
            windows.remove(w);
            // @TODO: remove from container // w->container->remove();
            break;
    }
    if (w == lastActiveTiledWindow)
        lastActiveTiledWindow = 0;
}
