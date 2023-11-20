#pragma once
#include <thread>
#include <memory>
#include "MTMDTuringMachine/TMTape.h"
#include "AtomicQueue.h"

using namespace std;

class VisualisationManager {
private:
    unique_ptr<thread> glfwWorker;
    AtomicQueue<bool> updates;
    TMTape3D* tape;

    VisualisationManager();
    inline static unique_ptr<VisualisationManager> _instance;
    inline static bool _instanceFlag{false};

public:
    ~VisualisationManager();
    static VisualisationManager* getInstance();
    void setTape(TMTape3D * tape);
    void waitForExit();
};
