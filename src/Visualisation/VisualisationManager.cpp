#include "VisualisationManager.h"
#include "Visualisation.h"

VisualisationManager::VisualisationManager() {
    glfwWorker = make_unique<thread>([this]{
        map<string, Color> colorMap{{"A", Color(1.0f, 0.0f, 0.0f, .5f)},
                                    {"W", Color(0.259f, 0.373f, 0.82f, .5f)},
                                    {"GW", Color(0.259f, 0.373f, 0.82f, .5f)},
                                    {"default", Color(1.0f, 1.0f, 1.0f, 1.0f)}};
        Visualisation v(45.0f, 0.1f, 100.0f, colorMap);
        bool go = true;
        while (go){
            VisualisationCommand command;
            if(updates.dequeue(command)){
                switch (command) {
                    case VisualisationCommand::Rebuild:
                        v.rebuild(tape);
                        break;
                    case VisualisationCommand::ExportMesh:
                        v.exportMesh("mesh.obj");
                        break;
                }
            }
            go = v.update();
        }
    });
}

VisualisationManager::~VisualisationManager() {

}

VisualisationManager *VisualisationManager::getInstance() {

    if(!_instanceFlag){
        _instance = unique_ptr<VisualisationManager>(new VisualisationManager());
        _instanceFlag = true;
        return _instance.get();
    }else{
        return _instance.get();
    }
}

void VisualisationManager::setTape(TMTape3D * tape) {
    VisualisationManager::tape = tape;
    updates.enqueue(VisualisationCommand::Rebuild);

}

void VisualisationManager::waitForExit() {
    glfwWorker->join();
}

TMTape3D *VisualisationManager::getTape() const {
    return tape;
}
