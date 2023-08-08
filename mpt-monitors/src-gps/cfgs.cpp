#include "cfgs.h"

#include <iostream>

#include "cfgset.h"
#include "workbag.h"

std::ostream &operator<<(std::ostream &s, const PEStepResult r) {
    switch (r) {
        case PEStepResult::None:
            s << "None";
            break;
        case PEStepResult::Accept:
            s << "Accept";
            break;
        case PEStepResult::Reject:
            s << "Reject";
            break;
    }
    return s;
}

void Cfg_1::queueNextConfigurations(Workbag &workbag) {
    /*
    std::cout << "Queueng next configurations:\n";
    std::cout << "Positions: ";
    for (int i = 0; i < 2; ++i)
        std::cout << positions[i] << ", ";
    std::cout << "\n";
    */

    assert(mPE.accepted() && mPE.cond(trace(0), trace(1)));

    ConfigurationsSet<3> S;
    S.add(Cfg_1(traces, positions));
    S.add(Cfg_2(traces, positions));
    S.add(Cfg_3(traces, positions));
    workbag.push(std::move(S));
}
