#pragma once
#include <functional>
#include <vector>

struct derivation_history_t {
  /// indicates the last compilation time in wich this producion was used
  unsigned short compilation_id{0};
  /// indicates the offset starting from the beginning the token chain
  int offset{-1};
};

inline bool CHECK_DERIVATION_HISTORY(derivation_history_t &history);
inline void CLEAR_DERIVATION_HISTORY(derivation_history_t &history);

inline bool HANDLE_TERMINAL(int);               // definined with
                                                // BEGIN_BINDINGS
constexpr inline bool HANDLE_NON_TERMINAL(int); // definined with BEGIN_BINDINGS

#define END_ILC_CODEGEN

#define BEGIN_ILC_CODEGEN                                                      \
  namespace ILC {                                                              \
  /** @brief Compilation time, indicates the ILC compilation time, should be   \
   * incremented when the chain content changes  */                            \
  static unsigned short compilation_id{0};                                     \
  /** @brief The chain to be parsed */ static std::vector<SYMBOL> chain{};     \
  /** @brief The size of the chain, should be updated with chain.size() when   \
   * @a chain content's change*/                                               \
  static size_t chain_size{0};                                                 \
  /** @brief Indicates the offset of the compilation @warning When getting the \
   * value of an index after REQUIRE_X(...) always do chain[offset-1], because \
   * offset gets incremented after REQUIRE*/                                   \
  static int offset{0};                                                        \
  }

#if defined(BEGIN_ILC_CODEGEN)

#define BEGIN_BINDINGS                                                         \
  constexpr inline bool HANDLE_NON_TERMINAL(int _SYMBOL) {                     \
    switch (_SYMBOL) {

#define BEGIN_SYMBOL_BINDING(S)                                                \
  case S:                                                                      \
    return
#define END_SYMBOL_BINDING ;

#define END_BINDINGS                                                           \
  default:                                                                     \
    return false;                                                              \
    }                                                                          \
    }                                                                          \
    inline bool HANDLE_TERMINAL(int symbol) {                                  \
      if (ILC::offset < ILC::chain_size and                                    \
          ILC::chain[ILC::offset] == symbol) {                                 \
        ILC::offset++;                                                         \
        return true;                                                           \
      }                                                                        \
      return false;                                                            \
    }                                                                          \
    inline bool CHECK_DERIVATION_HISTORY(derivation_history_t &history) {      \
      static bool found;                                                       \
      found = history.offset >= ILC::offset and                                \
              history.compilation_id == ILC::compilation_id;                   \
      if (not found) {                                                         \
        history.offset = ILC::offset;                                          \
        history.compilation_id = ILC::compilation_id;                          \
      }                                                                        \
      return found;                                                            \
    }                                                                          \
    inline void CLEAR_DERIVATION_HISTORY(derivation_history_t &history) {      \
      history.offset = -1;                                                     \
    }

#define DECLARE_PRODUCTION(NAME) inline bool NAME();

#define SET_CALLBACK(C)                                                        \
  std::function<void()> CALLBACK = C;                                          \
  CALLBACK();

#define BEGIN_PRODUCTION(NAME)                                                 \
  inline bool NAME() {                                                         \
    static derivation_history_t HISTORY;                                       \
    if (CHECK_DERIVATION_HISTORY(HISTORY))                                     \
      return false;                                                            \
    const size_t OFFSETC = ILC::offset;

#define END_PRODUCTION                                                         \
  CLEAR_DERIVATION_HISTORY(HISTORY);                                           \
  return true;                                                                 \
  }

#define TRY_REQUIRE_TERMINAL(I) HANDLE_TERMINAL(I)
#define TRY_REQUIRE_NON_TERMINAL(I) HANDLE_NON_TERMINAL(I)

#define ROLLBACK_PRODUCTION()                                                  \
  ILC::offset = OFFSETC;                                                       \
  return false;

#define REQUIRE_TERMINAL(I)                                                    \
  if (not(HANDLE_TERMINAL(I))) {                               \
    ROLLBACK_PRODUCTION()                                                      \
  }

#define REQUIRE_TERMINAL_CALLBACK(I, C)                                        \
  if (not(HANDLE_TERMINAL(I))) {                               \
    C();                                                                       \
    ROLLBACK_PRODUCTION()                                                      \
  }

#define REQUIRE_NON_TERMINAL(I)                                                \
  if (not(HANDLE_NON_TERMINAL(I))) {                           \
    ROLLBACK_PRODUCTION()                                                      \
  }

#define REQUIRE_NON_TERMINAL_CALLBACK(I, C)                                    \
  if (not(HANDLE_NON_TERMINAL(I))) {                           \
    C();                                                                       \
    ROLLBACK_PRODUCTION()                                                      \
  }

#endif