#ifndef OD_PES_H_
#define OD_PES_H_

#include <vamos-buffers/cpp/event.h>

#include <cassert>

#include "events.h"

enum class StepResult { None = 1, Accept = 2, Reject = 3, Done = 4 };

std::ostream &operator<<(std::ostream &s, const StepResult r);

struct PE1 : public PrefixExpression {
    StepResult step(const Event *ev, size_t pos) {
        const auto *e = static_cast<const TraceEvent *>(ev);

        switch ((Kind)e->get_kind()) {
            case Kind::InputL:
            case Kind::OutputL:
#ifndef NDEBUG
                state = 1;
#endif
                match_pos = pos;
                // M.append(MString::Letter(pos, pos));
                return StepResult::Accept;
            case Kind::Dummy:
                return StepResult::None;
            default:
                assert(state == 0);
                return StepResult::Reject;
        }
        abort();
    }
};

struct PE2 : public PrefixExpression {
    StepResult step(const Event *ev, size_t pos) {
        const auto *e = static_cast<const TraceEvent *>(ev);

#ifdef ASSUME_SYNC_TRACES
        switch ((Kind)e->get_kind()) {
            case Kind::OutputL:
#ifndef NDEBUG
                state = 1;
#endif
                // M.append(MString::Letter(pos, pos));
                match_pos = pos;
                return StepResult::Accept;
            default:
                assert(state == 0);
                return StepResult::None;
        }
#else  // ASSUME_SYNC_TRACES

        switch ((Kind)e->get_kind()) {
            case Kind::OutputL:
            case Kind::End:
#ifndef NDEBUG
                state = 1;
#endif
                // M.append(MString::Letter(pos, pos));
                match_pos = pos;
                return StepResult::Accept;
            case Kind::Dummy:
                return StepResult::None;
            default:
                assert(state == 0);
                return StepResult::Reject;
        }
    }
#endif  // ASSUME_SYNC_TRACES
    };

    struct PE3 : public PrefixExpression {
        StepResult step(const Event *ev, size_t pos) {
            const auto *e = static_cast<const TraceEvent *>(ev);

#ifdef ASSUME_SYNC_TRACES
            switch ((Kind)e->get_kind()) {
                case Kind::InputL:
#ifndef NDEBUG
                    state = 1;
#endif
                    match_pos = pos;
                    // M.append(MString::Letter(pos, pos));
                    return StepResult::Accept;
                default:
                    assert(state == 0);
                    return StepResult::None;
            }

#else  // ASSUME_SYNC_TRACES

        switch ((Kind)e->get_kind()) {
            case Kind::InputL:
            case Kind::OutputL:
            case Kind::End:
#ifndef NDEBUG
                state = 1;
#endif
                match_pos = pos;
                // M.append(MString::Letter(pos, pos));
                return StepResult::Accept;
            case Kind::Dummy:
                return StepResult::None;
            default:
                assert(state == 0);
                return StepResult::Reject;
        }

#endif  // ASSUME_SYNC_TRACES
        }
    };

    struct mPE_1 {
        PE1 _exprs[2];
        bool _accepted[2] = {false, false};

        bool accepted(size_t idx) const { return _accepted[idx]; }
        bool accepted() const { return _accepted[0] && _accepted[1]; }

        StepResult step(size_t idx, const Event *ev, size_t pos) {
            assert(idx < 2);
            auto res = _exprs[idx].step(ev, pos);
            if (res == StepResult::Accept)
                _accepted[idx] = true;
            return res;
        }

        template <size_t idx>
        StepResult step(const Event *ev, size_t pos) {
            assert(idx < 2);
            auto res = _exprs[idx].step(ev, pos);
            if (res == StepResult::Accept)
                _accepted[idx] = true;
            return res;
        }

        template <typename TraceT>
        bool cond(TraceT *t1, TraceT *t2) const {
            return *static_cast<TraceEvent *>(t1->get(_exprs[0].match_pos)) ==
                   *static_cast<TraceEvent *>(t2->get(_exprs[1].match_pos));
        }
    };

    struct mPE_2 {
        PE2 _exprs[2];
        bool _accepted[2] = {false, false};

        bool accepted(size_t idx) const { return _accepted[idx]; }
        bool accepted() const { return _accepted[0] && _accepted[1]; }

        StepResult step(size_t idx, const Event *ev, size_t pos) {
            assert(idx < 2);
            auto res = _exprs[idx].step(ev, pos);
            if (res == StepResult::Accept)
                _accepted[idx] = true;
            return res;
        }

        template <size_t idx>
        StepResult step(const Event *ev, size_t pos) {
            assert(idx < 2);
            auto res = _exprs[idx].step(ev, pos);
            if (res == StepResult::Accept)
                _accepted[idx] = true;
            return res;
        }

        template <typename TraceT>
        bool cond(TraceT *t1, TraceT *t2) const {
            return *static_cast<TraceEvent *>(t1->get(_exprs[0].match_pos)) !=
                   *static_cast<TraceEvent *>(t2->get(_exprs[1].match_pos));
        }
    };

    struct mPE_3 {
        PE3 _exprs[2];
        bool _accepted[2] = {false, false};

        bool accepted(size_t idx) const { return _accepted[idx]; }
        bool accepted() const { return _accepted[0] && _accepted[1]; }

        template <size_t idx>
        StepResult step(const Event *ev, size_t pos) {
            assert(idx < 2);
            auto res = _exprs[idx].step(ev, pos);
            if (res == StepResult::Accept)
                _accepted[idx] = true;
            return res;
        }

        template <typename TraceT>
        bool cond(TraceT *t1, TraceT *t2) const {
            auto *e1 = static_cast<TraceEvent *>(t1->get(_exprs[0].match_pos));
            auto *e2 = static_cast<TraceEvent *>(t2->get(_exprs[1].match_pos));
            return *e1 != *e2 && (
                    (((Kind)e1->get_kind() != Kind::OutputL) ||
                     (((Kind)e2->get_kind() != Kind::OutputL) &&
                      ((Kind)e2->get_kind() != Kind::End)))  &&
                    (((Kind)e2->get_kind() != Kind::OutputL) ||
                     (((Kind)e1->get_kind() != Kind::OutputL) &&
                      ((Kind)e1->get_kind() != Kind::End)))
                    );
        }
    };

#endif
