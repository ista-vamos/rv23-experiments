#ifndef OD_CFGSET_H
#define OD_CFGSET_H

#include "cfgs.h"

struct AnyCfg {
    unsigned short _idx{3};
    union CfgTy {
        Cfg_1 cfg1;
        Cfg_2 cfg2;
        Cfg_3 cfg3;
        ConfigurationBase none;

        CfgTy() : none() {}
        CfgTy(Cfg_1 &&c) : cfg1(std::move(c)) {}
        CfgTy(Cfg_2 &&c) : cfg2(std::move(c)) {}
        CfgTy(Cfg_3 &&c) : cfg3(std::move(c)) {}
    } cfg;

    template <typename CfgTy>
    CfgTy &get() {
        abort(); /*return std::get<CfgTy>(cfg);*/
    }
    template <>
    Cfg_1 &get() {
        return cfg.cfg1;
    }
    template <>
    Cfg_2 &get() {
        return cfg.cfg2;
    }
    template <>
    Cfg_3 &get() {
        return cfg.cfg3;
    }

    auto index() const -> auto{ return _idx; }

    AnyCfg(){};
    /*
    template <typename CfgTy> AnyCfg(const CfgTy &c) : cfg(c) {}
    AnyCfg(const AnyCfg &rhs) = default;
    AnyCfg &operator=(const AnyCfg &rhs) {
      cfg = rhs.cfg;
      return *this;
    }
    */
    template <typename CfgTy>
    AnyCfg(CfgTy &&c) : cfg(std::move(c)) {
        abort();
    }
    template <>
    AnyCfg(Cfg_1 &&c) : _idx(0), cfg(std::move(c)) {}
    template <>
    AnyCfg(Cfg_2 &&c) : _idx(1), cfg(std::move(c)) {}
    template <>
    AnyCfg(Cfg_3 &&c) : _idx(2), cfg(std::move(c)) {}

    AnyCfg(AnyCfg &&rhs) : _idx(rhs._idx) {
        switch (_idx) {
            case 0:
                cfg.cfg1 = std::move(rhs.cfg.cfg1);
                break;
            case 1:
                cfg.cfg2 = std::move(rhs.cfg.cfg2);
                break;
            case 2:
                cfg.cfg3 = std::move(rhs.cfg.cfg3);
                break;
            default:
                break;  // do nothing
        }
    };

    AnyCfg &operator=(AnyCfg &&rhs) {
        _idx = rhs._idx;
        switch (_idx) {
            case 0:
                cfg.cfg1 = std::move(rhs.cfg.cfg1);
                break;
            case 1:
                cfg.cfg2 = std::move(rhs.cfg.cfg2);
                break;
            case 2:
                cfg.cfg3 = std::move(rhs.cfg.cfg3);
                break;
            default:
                break;  // do nothing
        }
        return *this;
    }

#ifdef DEBUG
#ifdef DEBUG_CFGS
    std::string name() const {
        if (index() == 0)
            return cfg.cfg1.name();
        if (index() == 1)
            return cfg.cfg2.name();
        if (index() == 2)
            return cfg.cfg3.name();

        return "<invalid>";
    }
#endif
#endif
};

template <size_t MAX_SIZE>
struct ConfigurationsSet {
    size_t _size{0};
    bool _invalid{false};
    AnyCfg _confs[MAX_SIZE];

    /*
    void add(const AnyCfg &c) {
      assert(_size < MAX_SIZE);
      _confs[_size++] = c;
    }
    */

    void add(AnyCfg &&c) {
        assert(!_invalid);
        assert(_size < MAX_SIZE);
        _confs[_size++] = std::move(c);
    }

    void clear() {
        _size = 0;
        assert(!_invalid);
    }

    ConfigurationsSet(const ConfigurationsSet &) = delete;
    ConfigurationsSet(ConfigurationsSet &&) = default;
    ConfigurationsSet &operator=(ConfigurationsSet &&) = default;
    ConfigurationsSet() = default;

    void setInvalid() { _invalid = true; }
    bool invalid() const { return _invalid; }

    size_t size() const { return _size; }

    AnyCfg &get(unsigned i) {
        assert(i < _size);
        return _confs[i];
    }

    const AnyCfg &get(unsigned i) const {
        assert(i < _size);
        return _confs[i];
    }
};

#endif  // OD_CFGSET_H
