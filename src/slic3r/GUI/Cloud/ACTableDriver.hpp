#ifndef ACTABLEDRIVER_HPP
#define ACTABLEDRIVER_HPP

#include <map>
#include <type_traits>

#include <assert.h>

template <typename Key, typename FuncType> class TableDriver {
private:
  typedef FuncType AnyFunction;
  typedef std::map<Key, AnyFunction> TalbeType;

public:
  void Register(const Key &key, const AnyFunction &func) {
    assert(m_tb.find(key) == m_tb.end());
    m_tb[key] = std::move(func);
  }
  template <typename rettype = void, typename... Args>
  rettype Execute(const Key &key, Args &&...args) {
    typedef std::function<rettype(Args && ...)> functype;
    auto it = m_tb.find(key);
    if (it == m_tb.end()) {
      throw std::invalid_argument("error key");
      return (rettype)0;
    }
    return it->second(std::forward<Args>(args)...);
  }

private:
  TalbeType m_tb;
};

#endif //!ACTABLEDRIVER_HPP
