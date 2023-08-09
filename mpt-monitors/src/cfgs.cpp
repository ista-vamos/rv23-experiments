#include "cfgs.h"

#include <iostream>

#include "cfgset.h"
#include "workbag.h"

std::ostream &operator<<(std::ostream &s, const StepResult r) {
    switch (r) {
        case StepResult::None:
            s << "None";
            break;
        case StepResult::Accept:
            s << "Accept";
            break;
        case StepResult::Reject:
            s << "Reject";
            break;
    }
    return s;
}

template <>
Cfg_1 &AnyCfg::get<Cfg_1>() {
    return cfg.cfg1;
}

template <>
Cfg_2 &AnyCfg::get<Cfg_2>() {
    return cfg.cfg2;
}

template <>
Cfg_3 &AnyCfg::get<Cfg_3>() {
    return cfg.cfg3;
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

#ifdef DEBUG
#ifdef DEBUG_CFGS
size_t Cfg_1::__id = 0;
size_t Cfg_2::__id = 0;
size_t Cfg_3::__id = 0;
#endif
#endif
