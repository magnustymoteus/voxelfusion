#include "VisualisationManager.h"
#include "Visualisation.h"

VisualisationManager::VisualisationManager() {
    glfwWorker = make_unique<thread>([this]{
        Visualisation v;
        bool go = true;
        while (go){
            bool rebuild = false;
            updates.dequeue(rebuild);
            if(rebuild){
                v.rebuild(tape);
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
    updates.enqueue(true);

}

void VisualisationManager::waitForExit() {
    glfwWorker->join();
}
