// RUN: %clang_cc1 -fsyntax-only -std=c++17 -verify %s
// expected-no-diagnostics

template<typename T>
struct Result {
  T value;
  T carry;
  constexpr bool operator==(const Result<T> &Other) {
    return value == Other.value && carry == Other.carry;
  }
};

template<typename T>
constexpr Result<T> add(T lhs, T rhs, T carryin)
{
  T carryout = 0;
  if constexpr(__is_same(T, unsigned char))
    return { __builtin_addcb(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned short))
    return { __builtin_addcs(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned int))
    return { __builtin_addc(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned long))
    return { __builtin_addcl(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned long long))
    return { __builtin_addcll(lhs, rhs, carryin, &carryout), carryout };
}

static_assert(add<unsigned char>(0, 0, 0) == Result<unsigned char>{0, 0});

template<typename T>
constexpr Result<T> sub(T lhs, T rhs, T carryin)
{
  T carryout = 0;
  if constexpr(__is_same(T, unsigned char))
    return { __builtin_subcb(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned short))
    return { __builtin_subcs(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned int))
    return { __builtin_subc(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned long))
    return { __builtin_subcl(lhs, rhs, carryin, &carryout), carryout };
  else if constexpr(__is_same(T, unsigned long long))
    return { __builtin_subcll(lhs, rhs, carryin, &carryout), carryout };
}