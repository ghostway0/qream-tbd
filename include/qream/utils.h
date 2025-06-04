#define TRYV(...)                                  \
  ({                                               \
    auto res = (__VA_ARGS__);                      \
    if (!res.ok()) return std::move(res).status(); \
    std::move(*res);                               \
  })

#define TRY(...)               \
  do {                         \
    auto res = (__VA_ARGS__);  \
    if (!res.ok()) return res; \
  } while (0)

#define RETURN_IF_OK(status_expr)     \
  do {                                \
    auto _status = (status_expr);     \
    if (_status.ok()) return _status; \
  } while (0)
