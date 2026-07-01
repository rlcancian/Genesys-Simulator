#include <chrono>
#include <iostream>
#include <locale>
#include <string>
#include <thread>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/Continuous/DiffusionField.h"

int main() {
    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();

    DiffusionField field(model, "DiffusionUnicodeDemo");
    field.setDimensions(2);
    field.setPointsPerDimension(41);
    field.setDomainLength(3.0);
    field.setDiffusionCoefficient(0.25);
    field.setBoundaryCondition("Neumann");
    field.setInitialCondition("Gaussian");
    field.setInitialParameter(0.20);
    field.setOdeSolver("DormandPrince54");

    field.setStartTime(0.0);
    field.setStopTime(1.5);
    field.setStepSize(0.05);

    const std::string shades[] = {"  ", "░░", "▒▒", "▓▓", "██"};

    std::string err;

    while (field.getCurrentTime() < field.getStopTime()) {
        if (!field.advanceOneStep(err)) {
            std::cerr << "Erro: " << err << "\n";
            return 1;
        }

        const double maxv = field.getMaxValue();

        std::cout << "\033[2J\033[H";
        std::cout << "DiffusionField \"" << "DiffusionUnicodeDemo" << "\""
                  << " | t=" << field.getCurrentTime()
                  << " | mass=" << field.getTotalMass()
                  << " | max=" << maxv
                  << "\n\n";

        const unsigned int n = field.getPointsPerDimension();

        for (unsigned int i = 0; i < n; ++i) {
            for (unsigned int j = 0; j < n; ++j) {
                double v = field.getFieldValue({i, j});
                std::size_t k = 0;

                if (maxv > 0.0) {
                    double x = v / maxv;
                    if (x < 0.0) x = 0.0;
                    if (x > 1.0) x = 1.0;
                    k = static_cast<std::size_t>(x * 4.0);
                }

                std::cout << shades[k];
            }
            std::cout << '\n';
        }

        std::cout << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(180));
    }

    return 0;
}
