#include "Visualisation/Visualisation.h"
#include "TMgenerator/TMGenerator.h"

int main() {
    map<string, Color> colorMap{{"A", Color(1.0f, 0.0f, 0.0f, .5f)},
                                {"W", Color(0.259f, 0.373f, 0.82f, .5f)},
                                {"GW", Color(0.259f, 0.373f, 0.82f, .5f)},
                                {"black", Color(0.0f, 0.0f, 0.0f, 1.0f)},
                                {"white", Color(1.0f, 1.0f, 1.0f, 1.0f)},
                                {"default", Color(1.0f, 1.0f, 1.0f, 1.0f)}};
    Visualisation v(45.0f, 0.1f, 100.0f, colorMap);
    v.run();
    return 0;
}