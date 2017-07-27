#ifndef SJC_COPYABLE_H
#define SJC_COPYABLE_H

namespace sjc
{

// Any derived class of copyable should be a value type.
class copyable {
protected:
  copyable() = default; // 避免直接使用该类
  ~copyable() = default;
};

};

#endif // SJC_COPYABLE_H
