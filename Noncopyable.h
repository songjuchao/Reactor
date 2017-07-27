#ifndef SJC_NONCOPYABLE_H
#define SJC_NONCOPYABLE_H

namespace sjc
{

class noncopyable {
protected:
  noncopyable() = default;
  ~noncopyable() = default;

private:
  noncopyable(const noncopyable&) = delete;
  // noncopyable& operator =(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

};

} // sjc

#endif // SJC_NONCOPYABLE_H
