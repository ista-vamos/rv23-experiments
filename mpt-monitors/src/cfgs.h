#ifndef OD_CFGS_H_
#define OD_CFGS_H_

#include <vamos-buffers/cpp/event.h>

#include <cassert>

#include "events.h"
#include "monitor.h"
#include "prefixexprs.h"

class Workbag;

enum class CfgState { OK, FAILED, MATCHED };

///
/// Configuration is the state of evaluating an edge
class Configuration {
   public:
    using TraceTy = Trace<TraceEvent>;

   protected:
    CfgState _state{CfgState::OK};
    size_t positions[2] = {0};
    TraceTy *traces[2];

   public:
    Configuration() {}
    // Configuration& operator=(const Configuration&) = default;
    Configuration(TraceTy *tr[2]) {
        traces[0] = tr[0];
        traces[1] = tr[1];
    }

    Configuration(TraceTy *tr0, TraceTy *tr1) {
        traces[0] = tr0;
        traces[1] = tr1;
    }

    bool ge(const Configuration &rhs) const {
        return positions[0] >= rhs.positions[0] &&
               positions[1] >= rhs.positions[1];
    }

    bool lt(const Configuration &rhs) const { return !ge(rhs); }

    TraceTy *trace(size_t idx) { return traces[idx]; }
    const TraceTy *trace(size_t idx) const { return traces[idx]; }

    // size_t pos(size_t idx) const { return positions[idx]; }

    void set_failed() { _state = CfgState::FAILED; }
    bool failed() const { return _state == CfgState::FAILED; }

    void set_matched() { _state = CfgState::MATCHED; }
    bool matched() const { return _state == CfgState::MATCHED; }
};

template <typename MpeTy>
class CfgTemplate : public Configuration {
   protected:
    MpeTy mPE{};

   public:
    bool canProceed(size_t idx) const {
        return !mPE.accepted(idx) && trace(idx)->size() > positions[idx];
    }

    template <size_t idx>
    bool canProceed() const {
        return !mPE.accepted(idx) && traces[idx]->size() > positions[idx];
    }

    // how many steps can we proceed?
    /*
    size_t canProceedN(size_t idx) const {
        return traces[idx]->size() - positions[idx];
    }
    */

    template <size_t idx>
    size_t canProceedN() const {
        return traces[idx]->size() - positions[idx];
    }

    void queueNextConfigurations(Workbag &) {
        /* no next configurations by default*/
    }

    template <size_t idx>
    StepResult step() {
        assert(canProceed<idx>() && "Step on invalid PE");
        assert(!failed() && !matched());

        const Event *ev = traces[idx]->get(positions[idx]);
        assert(ev && "No event");
        auto res = mPE.template step<idx>(ev, positions[idx]);

#ifdef DEBUG
#ifdef DEBUG_CFGS
        std::cout << "(𝜏" << idx << ") t" << traces[idx]->id() << "["
                  << positions[idx] << "]"
                  << "@" << *static_cast<const TraceEvent *>(ev) << ", "
                  << positions[idx] << " => " << res << "\n";
#endif
#endif

        ++positions[idx];

        switch (res) {
            case StepResult::Accept:
                if (mPE.accepted()) {
                    // std::cout << "mPE matched prefixes\n";
                    if (mPE.cond(trace(0), trace(1))) {
                        // std::cout << "Condition SAT!\n";
                        set_matched();
                        return StepResult::Accept;
                    } else {
                        // std::cout << "Condition UNSAT!\n";
                        set_failed();
                        return StepResult::Reject;
                    }
                }
                return StepResult::None;
            case StepResult::Reject:
                set_failed();
                // fall-through
            default:
                return res;
        }
    }

#if 0
  // Do as many steps as possible in this configuration
  template <size_t idx>
  StepResult stepN() {
    assert(!_failed);

    if (!canProceed<idx>())
        return StepResult::None;

    size_t N = canProceedN<idx>();
    while (N-- > 0) {
        const Event *ev = traces[idx]->get(positions[idx]);
        assert(ev && "No event");
        auto res = mPE.template step<idx>(ev, positions[idx]);

#ifdef DEBUG
#ifdef DEBUG_CFGS
        std::cout << "(𝜏" << idx << ") t" << trace(idx)->id()
                  << "[" << positions[idx] << "]"
                  << "@" << *static_cast<const TraceEvent *>(ev) << ", "
                  << positions[idx] << " => " << res << "\n";
#endif
#endif

        ++positions[idx];

        if (res != StepResult::None)
            return res;
    }
    return StepResult::None;
  }

  StepResult stepN() {
    assert((canProceed(0) || canProceed(1)) && "Step on invalid MPE");
    auto res1 = stepN<0>();
    auto res2 = stepN<1>();

    if (res1 == StepResult::Reject ||
        res2 == StepResult::Reject) {
        _failed = true;
        return StepResult::Reject;
    }

    if (res1 == StepResult::Accept ||
        res2 == StepResult::Accept) {
      if (mPE.accepted()) {
        // std::cout << "mPE matched prefixes\n";
        if (mPE.cond(trace(0), trace(1))) {
          // std::cout << "Condition SAT!\n";
          return StepResult::Accept;
        } else {
          // std::cout << "Condition UNSAT!\n";
          _failed = true;
          return StepResult::Reject;
        }
      }
    }
    return StepResult::None;
  }
#endif

    CfgTemplate() {}
    // CfgTemplate& operator=(const CfgTemplate&) = default;
    CfgTemplate(Trace<TraceEvent> *traces[2]) : Configuration(traces) {}

    CfgTemplate(Trace<TraceEvent> *t0, Trace<TraceEvent> *t1)
        : Configuration(t0, t1) {}

    CfgTemplate(Trace<TraceEvent> *traces[2], const size_t pos[2])
        : Configuration(traces) {
        positions[0] = pos[0];
        positions[1] = pos[1];
    }
};

struct Cfg_1 : public CfgTemplate<mPE_1> {
#ifdef DEBUG
#ifdef DEBUG_CFGS
    static size_t __id;
    size_t _id;

#define INIT_ID (_id = ++__id)

    std::string name() const { return "cfg_1#" + std::to_string(_id); }
#endif
#else
#define INIT_ID
#endif

    Cfg_1() { INIT_ID; };
    // Cfg_1& operator=(const Cfg_1&) = default;
    Cfg_1(Trace<TraceEvent> *traces[2]) : CfgTemplate(traces) { INIT_ID; }

    Cfg_1(Trace<TraceEvent> *t0, Trace<TraceEvent> *t1) : CfgTemplate(t0, t1) {
        INIT_ID;
    }

    Cfg_1(Trace<TraceEvent> *traces[2], const size_t pos[2])
        : CfgTemplate(traces, pos) {
        INIT_ID;
    }

    void queueNextConfigurations(Workbag &);
};

struct Cfg_2 : public CfgTemplate<mPE_2> {
#ifdef DEBUG
#ifdef DEBUG_CFGS
    static size_t __id;
    size_t _id;

#define INIT_ID (_id = ++__id)

    std::string name() const { return "cfg_2#" + std::to_string(_id); }
#endif
#else
#define INIT_ID
#endif

    Cfg_2() { INIT_ID; };
    Cfg_2(Trace<TraceEvent> *traces[2]) : CfgTemplate(traces) { INIT_ID; }

    Cfg_2(Trace<TraceEvent> *t0, Trace<TraceEvent> *t1) : CfgTemplate(t0, t1) {
        INIT_ID;
    }
    Cfg_2(Trace<TraceEvent> *traces[2], const size_t pos[2])
        : CfgTemplate(traces, pos) {
        INIT_ID;
    }
};

struct Cfg_3 : public CfgTemplate<mPE_3> {
#ifdef DEBUG
#ifdef DEBUG_CFGS
    static size_t __id;
    size_t _id;

#define INIT_ID (_id = ++__id)

    std::string name() const { return "cfg_3#" + std::to_string(_id); }
#endif
#else
#define INIT_ID
#endif
    Cfg_3() { INIT_ID; }
    Cfg_3(Trace<TraceEvent> *traces[2]) : CfgTemplate(traces) { INIT_ID; }

    Cfg_3(Trace<TraceEvent> *t0, Trace<TraceEvent> *t1) : CfgTemplate(t0, t1) {
        INIT_ID;
    }
    Cfg_3(Trace<TraceEvent> *traces[2], const size_t pos[2])
        : CfgTemplate(traces, pos) {
        INIT_ID;
    }
};

#endif
