#pragma once
#include <thread>
#include <memory>
#include "MTMDTuringMachine/TMTape.h"
#include "AtomicQueue.h"

using namespace std;

enum class VisualisationCommand {
    Rebuild,
    ExportMesh
};

class VisualisationManager {
private:
    unique_ptr<thread> glfwWorker;
    TMTape3D* tape;

    VisualisationManager();
    inline static unique_ptr<VisualisationManager> _instance;
    inline static bool _instanceFlag{false};

public:
    AtomicQueue<VisualisationCommand> updates;
    ~VisualisationManager();
    static VisualisationManager* getInstance();
    void setTape(TMTape3D * tape);

    TMTape3D *getTape() const;

    void waitForExit();
};
