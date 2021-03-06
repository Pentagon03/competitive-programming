#include <cstdio>
#include <vector>
#include <algorithm>

const int N = 3e5 + 10;
const int mod = 998244353;

struct CentroidNode {
  std::vector<int> cnt;
  std::vector<std::vector<int>> child_cnt;
} nodes[N];

std::vector<int> edges[N];
int depth[N], a[N], cnt[N], n, x;

void dfs(int u, int p = -1) {
  depth[u] = p == -1 ? 0 : depth[p] + 1;
  for (auto &v: edges[u]) if (v != p) {
    dfs(v, u);
  }
}

namespace centroid {
int size[N], mark[N], dist[N];
int total, mins, root;

void get_center(int u, int p = -1) {
  int mx = 0; size[u] = 1;
  for (auto &v: edges[u]) if (v != p && !mark[v]) {
    get_center(v, u);
    size[u] += size[v];
    if (size[v] > mx) mx = size[v];
  }
  if (total - size[u] > mx) mx = total - size[u];
  if (mx < mins) mins = mx, root = u;
}

std::pair<int, int> buffer[N];
int sz;

void dfs(int u, int p) {
  size[u] = 1;
  dist[u] = dist[p] + 1;
  buffer[sz++] = {u, dist[u]};
  for (auto &v: edges[u]) if (v != p && !mark[v]) {
    dfs(v, u);
    size[u] += size[v];
  }
}

int bit[N];
template<int sign>
void count(int l, int r) {
  std::sort(buffer + l, buffer + r, [&](std::pair<int, int> &a, std::pair<int, int> &b) {
    return depth[a.first] < depth[b.first] || (depth[a.first] == depth[b.first] && a.first < b.first);
  });
  int m = 0;
  for (int i = l; i < r; ++i) {
    if (buffer[i].second > m) m = buffer[i].second;
  }
  for (int i = 0; i <= m; ++i) bit[i] = 0;
  for (int i = l; i < r; ++i) {
    int ret = 0;
    for (int y = std::min(m, x - buffer[i].second); y >= 0; y -= ~y & y + 1) {
      ret += bit[y];
    }
    a[buffer[i].first] += ret * sign;
    for (int y = buffer[i].second; y <= m; y += ~y & y + 1) bit[y]++;
  }
}

void work(int u, int tot) {
  total = tot; mins = tot * 2;
  get_center(u);
  mark[u = root] = true;
  sz = 0;
  buffer[sz++] = {root, 0};
  dist[root] = 0;
  for (auto &v: edges[u]) if (!mark[v]) {
    int lower = sz;
    dfs(v, u);
    count<-1>(lower, sz);
  }
  count<1>(0, sz);
  for (auto &v: edges[u]) if (!mark[v]) {
    work(v, size[v]);
  }
}
}

using int64 = long long;
using uint32 = unsigned int;
using uint64 = unsigned long long;

namespace ntt {
// if mod is not close to 2^(word_bits-1), it's faster to use comment lines
template <class word, class dword, class sword, word mod, word root>
class Mod {
public:
  static constexpr word mul_inv(word n, int e = 6, word x = 1) {
    return e == 0 ? x : mul_inv(n, e - 1, x * (2 - x * n));
  }

  static constexpr word inv = mul_inv(mod);
  static constexpr word r2 = -dword(mod) % mod;
  static constexpr int word_bits = sizeof(word) * 8;
  static constexpr int level = __builtin_ctzll(mod - 1);

  static word modulus() {
    return mod;
  }
  static word init(const word& w) {
    return reduce(dword(w) * r2);
  }
  static word reduce(const dword& w) {
    word y = word(w >> word_bits) - word((dword(word(w) * inv) * mod) >> word_bits);
    return sword(y) < 0 ? y + mod : y;
    //return word(w >> word_bits) + mod - word((dword(word(w) * inv) * mod) >> word_bits);
  }
  static Mod omega() {
    return Mod(root).pow((mod - 1) >> level);
  }

  Mod() = default;
  Mod(const word& n): x(init(n)) {};
  Mod& operator += (const Mod& rhs) {
    //this->x += rhs.x;
    if ((x += rhs.x) >= mod) x -= mod;
    return *this;
  }
  Mod& operator -= (const Mod& rhs) {
    //this->x += mod * 3 - rhs.x;
    if (sword(x -= rhs.x) < 0) x += mod;
    return *this;
  }
  Mod& operator *= (const Mod& rhs) {
    this->x = reduce(dword(this->x) * rhs.x);
    return *this;
  }
  Mod operator + (const Mod& rhs) const {
    return Mod(*this) += rhs;
  }
  Mod operator - (const Mod& rhs) const {
    return Mod(*this) -= rhs;
  }
  Mod operator * (const Mod& rhs) const {
    return Mod(*this) *= rhs;
  }
  word get() const {
    return reduce(this->x) % mod;
  }
  Mod inverse() const {
    return pow(mod - 2);
  }
  Mod pow(word e) const {
    Mod ret(1);
    for (Mod a = *this; e; e >>= 1) {
      if (e & 1) ret *= a;
      a *= a;
    }
    return ret;
  }
  word x;
};

template <class T>
inline void sum_diff(T& x, T &y) {
  auto a = x, b = y;
  x = a + b, y = a - b;
}
}

// transform with dif, itransform with dft, no need to use bit_rev
namespace ntt_fast {
template <typename mod_t>
void transform(mod_t* A, int n, const mod_t* roots, const mod_t* iroots) {
  const int logn = __builtin_ctz(n), nh = n >> 1, lv = mod_t::level;
  auto one = mod_t(1), im = roots[lv - 2];
  mod_t dw[lv - 1]; dw[0] = roots[lv - 3];
  for (int i = 1; i < lv - 2; ++i) {
    dw[i] = dw[i - 1] * iroots[lv - 1 - i] * roots[lv - 3 - i];
  }
  dw[lv - 2] = dw[lv - 3] * iroots[1];
  if (logn & 1) for (int i = 0; i < nh; ++i) {
    ntt::sum_diff(A[i], A[i + nh]);
  }
  for (int e = logn & ~1; e >= 2; e -= 2) {
    const int m = 1 << e, m4 = m >> 2;
    auto w2 = one;
    for (int i = 0; i < n; i += m) {
      auto w1 = w2 * w2, w3 = w1 * w2;
      for (int j = i; j < i + m4; ++j) {
        auto a0 = A[j + m4 * 0] * one, a1 = A[j + m4 * 1] * w2;
        auto a2 = A[j + m4 * 2] * w1,  a3 = A[j + m4 * 3] * w3;
        auto t02p = a0 + a2, t13p = a1 + a3;
        auto t02m = a0 - a2, t13m = (a1 - a3) * im;
        A[j + m4 * 0] = t02p + t13p; A[j + m4 * 1] = t02p - t13p;
        A[j + m4 * 2] = t02m + t13m; A[j + m4 * 3] = t02m - t13m;
      }
      w2 *= dw[__builtin_ctz(~(i >> e))];
    }
  }
}

template <typename mod_t>
void itransform(mod_t* A, int n, const mod_t* roots, const mod_t* iroots) {
  const int logn = __builtin_ctz(n), nh = n >> 1, lv = mod_t::level;
  const auto one = mod_t(1), im = iroots[lv - 2];
  mod_t dw[lv - 1]; dw[0] = iroots[lv - 3];
  for (int i = 1; i < lv - 2; ++i) {
    dw[i] = dw[i - 1] * roots[lv - 1 - i] * iroots[lv - 3 - i];
  }
  dw[lv - 2] = dw[lv - 3] * roots[1];
  for (int e = 2; e <= logn; e += 2) {
    const int m = 1 << e, m4 = m >> 2;
    auto w2 = one;
    for (int i = 0; i < n; i += m) {
      const auto w1 = w2 * w2, w3 = w1 * w2;
      for (int j = i; j < i + m4; ++j) {
        auto a0 = A[j + m4 * 0], a1 = A[j + m4 * 1];
        auto a2 = A[j + m4 * 2], a3 = A[j + m4 * 3];
        auto t01p = a0 + a1, t23p = a2 + a3;
        auto t01m = a0 - a1, t23m = (a2 - a3) * im;
        A[j + m4 * 0] = (t01p + t23p) * one; A[j + m4 * 2] = (t01p - t23p) * w1;
        A[j + m4 * 1] = (t01m + t23m) * w2;  A[j + m4 * 3] = (t01m - t23m) * w3;
      }
      w2 *= dw[__builtin_ctz(~(i >> e))];
    }
  }
  if (logn & 1) for (int i = 0; i < nh; ++i) {
    ntt::sum_diff(A[i], A[i + nh]);
  }
}

template <typename mod_t>
void convolute(mod_t* A, int n, mod_t* B, int m, bool cyclic=false) {
  const int s = cyclic ? std::max(n, m) : n + m - 1;
  const int size = 1 << (31 - __builtin_clz(2 * s - 1));
  mod_t roots[mod_t::level], iroots[mod_t::level];
  roots[0] = mod_t::omega();
  for (int i = 1; i < mod_t::level; ++i) {
    roots[i] = roots[i - 1] * roots[i - 1];
  }
  iroots[0] = roots[0].inverse();
  for (int i = 1; i < mod_t::level; ++i) {
    iroots[i] = iroots[i - 1] * iroots[i - 1];
  }
  std::fill(A + n, A + size, 0); transform(A, size, roots, iroots);
  const auto inv = mod_t(size).inverse();
  if (A == B && n == m) {
    for (int i = 0; i < size; ++i) A[i] *= A[i] * inv;
  } else {
    std::fill(B + m, B + size, 0); transform(B, size, roots, iroots);
    for (int i = 0; i < size; ++i) A[i] *= B[i] * inv;
  }
  itransform(A, size, roots, iroots);
}
}

using mod_t = ntt::Mod<uint32, uint64, int, 998244353, 3>;

mod_t fac[N], ifac[N];
mod_t A[1 << 20], B[1 << 20];

int main() {
  scanf("%d%d", &n, &x);
  for (int i = 1; i < n; ++i) {
    int u, v;
    scanf("%d%d", &u, &v);
    --u, --v;
    edges[u].push_back(v);
    edges[v].push_back(u);
  }
  dfs(0);
  centroid::work(0, n);
  for (int i = 0; i < n; ++i) cnt[a[i]]++;
  fac[0] = ifac[0] = 1;
  for (int i = 1; i <= n; ++i) {
    fac[i] = fac[i - 1] * i;
    ifac[i] = fac[i].inverse();
  }
  for (int i = 0; i < n; ++i) A[i] = fac[i] * cnt[i];
  for (int i = 0; i < n; ++i) B[i] = ifac[n - 1 - i];
  ntt_fast::convolute(A, n, B, n);
  for (int i = 0; i < n; ++i) {
    printf("%d ", (A[i + n - 1] * ifac[i]).get());
  }
  puts("");
  return 0;
}
