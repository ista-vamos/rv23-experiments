#include "monitor.h"

#include <cassert>
#include <iostream>
#include <variant>

#include "cfgs.h"
#include "cfgset.h"
#include "events.h"
#include "workbag.h"

// #define OUTPUT

// struct Stats {
//   size_t max_wbg_size{0};
//   size_t cfgs_num{0};
// };

template <typename TracesT>
static void add_new_cfgs(Workbag &workbag, const TracesT &traces,
                         Trace<TraceEvent> *trace) {
    // set initially all elements to 'trace'
    ConfigurationsSet<3> S;

    for (auto &t : traces) {
        /* reflexivity reduction */
#ifdef REDUCT_REFLEXIVITY
        if (trace == t.get())
            continue;
#endif

        S.clear();
        S.add(Cfg_1(t.get(), trace));
        S.add(Cfg_2(t.get(), trace));
        S.add(Cfg_3(t.get(), trace));
        workbag.push(std::move(S));

#ifndef REDUCT_SYMMETRY
        S.clear();
        S.add(Cfg_1(trace, t.get()));
        S.add(Cfg_2(trace, t.get()));
        S.add(Cfg_3(trace, t.get()));
        workbag.push(std::move(S));
#endif
    }
}

// returns true to continue with next CFG
template <typename CfgTy>
StepResult move_cfg(CfgTy &cfg) {
#ifdef DEBUG
#ifdef DEBUG_CFGS
    std::cout << "MOVE " << cfg.name() << ":\n";
#endif
#endif

#ifdef MULTIPLE_MOVES
#error "Not supported yet"
    auto res = cfg.stepN();
    if (res == StepResult::Accept) {
        // std::cout << "CFG " << &c << " from " << &C << " ACCEPTED\n";
        return StepResult::Accept;
    }
    if (res == StepResult::Reject) {
        // std::cout << "CFG " << &c << " from " << &C << " REJECTED\n";
        return StepResult::Reject;
    }

    if (cfg.canProceedN<0>() == 0 && cfg.canProceedN<1>() == 0) {
        // check if the traces are done
        for (size_t idx = 0; idx < 2; ++idx) {
            if (!cfg.trace(idx)->done())
                return StepResult::None;
        }
        // std::cout << "CFG discarded becase it has read traces entirely\n";
        return StepResult::Done;
    }

#else  // !MULTIPLE_MOVES

    bool no_progress = true;

    // do step on trace 1 (if possible)
    if (cfg.template canProceed<0>()) {
        no_progress = false;

        auto res = cfg.template step<0>();
#ifdef DEBUG
#ifdef DEBUG_CFGS
        if (res == StepResult::Accept) {
            std::cout << "\033[1;35m" << cfg.name() << "** accepted **\033[0m\n";
        }
        if (res == StepResult::Reject) {
            std::cout << "\033[1;35m" << cfg.name() << "** rejected **\033[0m\n";
        }
#endif
#endif
        if (res != StepResult::None)
            return res;
    }

    // do step on trace 2 (if possible)
    if (cfg.template canProceed<1>()) {
        no_progress = false;

        auto res = cfg.template step<1>();
#ifdef DEBUG
#ifdef DEBUG_CFGS
        if (res == StepResult::Accept) {
            std::cout << "\033[1;35m" << cfg.name() << "** accepted **\033[0m\n";
        }
        if (res == StepResult::Reject) {
            std::cout << "\033[1;35m" << cfg.name() << "** rejected **\033[0m\n";
        }
#endif
#endif
        if (res != StepResult::None)
            return res;
    }

    if (no_progress) {
        // check if the traces are done
        for (size_t idx = 0; idx < 2; ++idx) {
            if (!cfg.trace(idx)->done())
                return StepResult::None;
        }
        // std::cout << "CFG discarded becase it has read traces entirely\n";
        return StepResult::Done;
    }
#endif  // MULTIPLE_MOVES

    return StepResult::None;
}

template <typename WorkbagT, typename TracesT, typename StreamsT>
void update_traces(Inputs &inputs, WorkbagT &workbag, TracesT &traces,
                   StreamsT &online_traces) {
    // get new input streams
    if (auto *stream = inputs.getNewInputStream()) {
        // std::cout << "NEW stream " << stream->id() << "\n";
        auto *trace = new Trace<TraceEvent>(stream->id());
        traces.emplace_back(trace);
        stream->setTrace(trace);
        online_traces.push_back(stream);

        add_new_cfgs(workbag, traces, trace);
    }

    // copy events from input streams to traces
    std::set<InputStream *> remove_online_traces;
    for (auto *stream : online_traces) {
        size_t n = 0;
        while (stream->hasEvent() && (n++ < ONETIME_READ_EVENTS_LIMIT)) {
            auto *event = static_cast<TraceEvent *>(stream->getEvent());
            auto *trace = static_cast<Trace<TraceEvent> *>(stream->trace());
            trace->append(event);

#ifdef DEBUG
#ifdef DEBUG_EVENTS
            std::cout << "[Stream " << stream->id() << "] event: " << *event
                      << "\n";
#endif
#endif

            if (stream->isDone()) {
                // std::cout << "Stream " << stream->id() << " DONE\n";
                remove_online_traces.insert(stream);
                trace->append(TraceEvent(Event::doneKind(), trace->size()));
                trace->setDone();
                break;
            }
        }
    }
    // remove finished traces
    if (remove_online_traces.size() > 0) {
        std::vector<InputStream *> tmp;
        tmp.reserve(online_traces.size() - remove_online_traces.size());
        for (auto *stream : online_traces) {
            if (remove_online_traces.count(stream) == 0)
                tmp.push_back(stream);
        }
        online_traces.swap(tmp);
    }
}

template <typename CfgTy, typename CfgSetTy>
static bool check_shortest(const CfgTy &c, CfgSetTy &C) {
    assert(c.matched());

    size_t sz = C.size();
    for (unsigned i = 0; i < sz; ++i) {
        auto &c2 = C.get(i);
        // take every other cfg from this set that has not failed yet
        if (!c2.failed() && (&c != &c2)) {
            if (c2.ge(c)) {
                assert(!c2.matched() || !c.ge(c2) &&
                                            "Two matches with same positions, "
                                            "MPT is not deterministic");
                // this one cannot be the shortest match as the current
                // one is a match and is shorter
                c2.set_failed();
            } else {
                return false;
            }
        }
    }

    // this is the shortest-match
#ifdef DEBUG
#ifdef DEBUG_CFGS
    std::cout << c.name() << "** is the shortest match **\n";
#endif
#endif
    return true;
}

int monitor(Inputs &inputs) {
    std::vector<std::unique_ptr<Trace<TraceEvent>>> traces;
    std::vector<InputStream *> online_traces;

    Workbag workbag;
    Workbag new_workbag;

    workbag.reserve(512);
    new_workbag.reserve(512);

#define STATS
#ifdef STATS
    // Stats stats;
    size_t max_wbg_size = 0;
    // size_t tuples_num = 0;
#endif
    size_t violations = 0;

    while (true) {
#ifdef DEBUG
        std::cout << "\033[1;34m--- Iteration start ---\033[0m\n";
#endif
        /////////////////////////////////
        /// UPDATE TRACES (NEW TRACES AND NEW EVENTS)
        /////////////////////////////////

        update_traces(inputs, workbag, traces, online_traces);

        /////////////////////////////////
        /// MOVE TRANSDUCERS
        /////////////////////////////////

        size_t wbg_size = workbag.size();
        size_t wbg_invalid = 0;
#ifdef STATS
        max_wbg_size = std::max(wbg_size, max_wbg_size);
#endif
#ifdef DEBUG
#ifdef DEBUG_WORKBAG
        std::cout << "WORKBAG size: " << wbg_size << "\n";
        size_t n = 0;
        for (auto &C : workbag) {
            if (n++ >= 10) {
                std::cout << "  ... and " << wbg_size - 10
                          << " more elements\n";
                break;
            }
            std::cout << "  > {";
            if (C.done())
                std::cout << "<done>; ";

            for (unsigned i = 0; i < C.size(); ++i) {
                auto &cfg = C.get(i);
                std::cout << cfg.name();
                if (cfg.matched())
                    std::cout << ":⊤";
                if (cfg.failed())
                    std::cout << ":⊥";
                std::cout << ",";
            }
            std::cout << "}\n";
        }
#endif
#endif
        for (auto &C : workbag) {
            if (C.done()) {
                ++wbg_invalid;
                continue;
            }

            bool non_empty = false;
            size_t sz = C.size();
            StepResult status;
            for (unsigned i = 0; i < sz; ++i) {
                auto &c = C.get(i);
                switch (c.index()) {
                    case 0: /* Cfg_1 */ {
                        auto &cfg = c.get<Cfg_1>();
                        if (cfg.failed())
                            continue;

                        non_empty = true;
                        status = cfg.matched()
                                     ? StepResult::Accept
                                     : move_cfg<Cfg_1>(cfg);
                        switch (status) {
                            // this edge matched
                            case StepResult::Accept:
                                if (check_shortest(c, C)) {
                                    cfg.queueNextConfigurations(new_workbag);
                                    C.set_done();
                                    ++wbg_invalid;
                                    goto continue_next_cfgset;
                                }
                                break;
                            // neither accepted nor rejected and read the traces
                            // to the end
                            case StepResult::Done:
                                C.set_done();
                                ++wbg_invalid;
                                goto continue_next_cfgset;
                            // the edge rejected
                            case StepResult::Reject:
                                cfg.set_failed();
                                break;
                            // no result yet
                            case StepResult::None:
                                break;
                        }
                        break;
                    }
                    case 1: /* Cfg_2 */ {
                        auto &cfg = c.get<Cfg_2>();
                        if (cfg.failed())
                            continue;

                        non_empty = true;
                        status = cfg.matched()
                                     ? StepResult::Accept
                                     : move_cfg<Cfg_2>(cfg);
                        switch (status) {
                            case StepResult::Accept:
                                if (check_shortest(c, C)) {
                                    ++violations;
#ifdef OUTPUT
                                    std::cout << "\033[1;31mOBSERVATIONAL "
                                                 "DETERMINISM "
                                                 "VIOLATED!\033[0m\n"
                                              << "Traces:\n"
                                              << "  " << cfg.trace(0)->descr()
                                              << "\n"
                                              << "  " << cfg.trace(1)->descr()
                                              << "\n";
#endif
#ifdef EXIT_ON_ERROR
                                    goto end;
#endif
                                    cfg.queueNextConfigurations(new_workbag);
                                } else {
                                    // do not discard this config
                                    break;
                                }
                                // fall-through to discard this set of configs
                            case StepResult::Done:
                                C.set_done();
                                ++wbg_invalid;
                                goto continue_next_cfgset;
                            case StepResult::Reject:  // remove c from C
                                cfg.set_failed();
                                break;
                            case StepResult::None:
                                break;
                        }
                        break;
                    }
                    case 2: /* Cfg_3 */ {
                        auto &cfg = c.get<Cfg_3>();
                        if (cfg.failed())
                            continue;

                        non_empty = true;
                        status = cfg.matched()
                                     ? StepResult::Accept
                                     : move_cfg<Cfg_3>(cfg);
                        switch (status) {
                            case StepResult::Accept:
                                if (check_shortest(c, C)) {
#ifdef OUTPUT
                                    std::cout << "OD holds for traces `"
                                              << cfg.trace(0)->descr()
                                              << "` and `"
                                              << cfg.trace(1)->descr() << "`\n";
#endif
                                    cfg.queueNextConfigurations(new_workbag);
                                } else {
                                    // do not discard this config
                                    break;
                                }
                                // fall-through
                            case StepResult::Done:
                                C.set_done();
                                ++wbg_invalid;
                                goto continue_next_cfgset;
                            case StepResult::Reject:  // remove c from C
                                cfg.set_failed();
                                break;
                            case StepResult::None:
                                break;
                        }
                        break;
                    }
                    default:
                        assert(false && "Unknown configuration");
                        abort();
                }
            }

            // if the cfgset contains no cfgs that still can proceed, remove it
            if (!non_empty)
                C.set_done();

        continue_next_cfgset:
            (void)1;
        }

        // if at least one third of workbag are invalid (done) configurations,
        // clean the workbag
        if (!new_workbag.empty() || wbg_invalid >= wbg_size / 3) {
            for (auto &C : workbag) {
                if (C.done())
                    continue;
                new_workbag.push(std::move(C));
            }
            workbag.swap(new_workbag);
            new_workbag.clear();
        }

        /////////////////////////////////

        if (workbag.empty() && inputs.done()) {
#ifdef OUTPUT
            std::cout << "No more traces to come, workbag empty\n";
            if (violations == 0) {
                std::cout
                    << "\033[1;32mNO VIOLATION OF OBSERVATIONAL DETERMINISM "
                       "FOUND!\033[0m\n";
            }
#endif
            break;
        }
#ifdef DEBUG
        std::cout << "\033[1;34m--- Iteration end ---\033[0m\n";
#endif
    }

end:
#ifdef STATS
    std::cout << "Max workbag size: " << max_wbg_size << "\n";
    std::cout << "Traces #: " << traces.size() << "\n";
#endif
    std::cout << "Found violations #: " << violations << "\n";

    return violations > 0;
}
