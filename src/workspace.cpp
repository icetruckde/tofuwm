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
    for(WmWindow* w : floatingWindows)
        w->hide();
    for(WmWindow* w : windows)
        w->hide();
}

void Workspace::show() {
    for(WmWindow* w : floatingWindows)
        w->show();
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
}

std::shared_ptr<WmContainer> Workspace::createNewContainer() {
    std::shared_ptr<WmContainer> container;
    switch (workspaceTilingMode_) {
        case WorkspaceTilingMode::Horizontal:
            container = make_shared<WmSplitter>(WmSplitterType::Horizontal);
            break;
        case WorkspaceTilingMode::Vertical:
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
    WmContainer *container = 0;

    if (lastActiveTiledWindow) {
        container = lastActiveTiledWindow->parent();
    } else if (windows.size() > 0) {
        container = windows.back()->parent();
    }

    if (!container) {
        if (!rootContainer)
            rootContainer = createNewContainer();
        container = rootContainer.get();
    }

    container->add(w->shared());
    container->realign();
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
        if (lastActiveTiledWindow == w)
            lastActiveTiledWindow = 0;
    }
}

WmContainer* Workspace::checkCleanContainer(WmContainer *container) {
    if (container->size() > 0)
        return container;

    WmContainer* parent = container->parent();
    if (parent != 0) {
        parent ->remove(container);
        return checkCleanContainer(parent);
    }

    // no parent -> we are root container
    rootContainer.reset();
    return 0; // no parent container
}

void Workspace::removeWindow(WmWindow* w) {
    switch (w->windowMode) {
        case WindowMode::Floating:
            floatingWindows.remove(w);
            break;
        case WindowMode::Tiled:
            windows.remove(w);
            if (lastActiveTiledWindow == w)
                lastActiveTiledWindow = 0;
            WmContainer* container = w->parent();
            if (container)
                container->remove(w);
            WmContainer* toAlign = checkCleanContainer(container);
            if (toAlign)
                toAlign->realign();
            break;
    }
    if (w == lastActiveTiledWindow)
        lastActiveTiledWindow = 0;
}
