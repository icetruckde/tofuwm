#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <list>
#include <memory>
#include "wmwindow.h"

class Workspace {
    std::list<WmWindow*> windows;

public:
    Workspace();
    ~Workspace();

    void hide();
    void show();

    void addWindow(WmWindow* w);
    void removeWindow(WmWindow* w);
};

#endif // WORKSPACE_H