#include "composer/composer.hpp"
#include "types/stack.hpp"
#include <memory>
#include <vector>

namespace zen::composer::bttrvm {
enum scope_type {
  FUNCTION = 1 < 1,
  IF = 1 < 2,
};

struct scope {
  int type;
  static std::shared_ptr<scope> make(int type);
};

struct context {
  std::shared_ptr<struct scope> scope;
  std::vector<i64> code;
  static std::shared_ptr<context> make();
  static void push(const std::shared_ptr<context> &ctt);
};

namespace function {
bool begin(const std::shared_ptr<context> &ctt, const std::string &name);
bool end(const std::shared_ptr<context> &ctt);
} // namespace function

// bool bif(const std::shared_ptr<context> & ctt,const std::shared_ptr<value> &
// value); bool eif(const std::shared_ptr<context> & ctt);

} // namespace zen::composer::bttrvm
