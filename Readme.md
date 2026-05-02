# MySQL Custom Aggregate UDFs

Custom aggregate functions for MySQL, implemented as User-Defined Functions (UDFs) in C++.

## Functions

### `product(value)`

Multiplies all non-NULL values in a group and returns the result — the multiplicative equivalent of `SUM()`.

```sql
SELECT product(value) FROM t;
```

| Behaviour | Detail |
| --------- | ------ |
| Return type | `DOUBLE` |
| Argument | Any numeric column or expression |
| NULL handling | NULL values are skipped (not treated as zero) |
| Empty group | Returns `1.0` (multiplicative identity) |

### `median(value)`

Returns the median of all non-NULL values in a group. For an even-sized group, returns the average of the two middle values.

```sql
SELECT median(value) FROM t;
```

| Behaviour | Detail |
| --------- | ------ |
| Return type | `DOUBLE` |
| Argument | Any numeric column or expression |
| NULL handling | NULL values are skipped |
| Empty group | Returns `NULL` |
| Memory | Buffers all non-NULL values for the group; O(n) per group |

### `mode(value)`

Returns the most frequently occurring non-NULL value in a group. If multiple values tie for most frequent, an arbitrary one of them is returned.

```sql
SELECT mode(value) FROM t;
```

| Behaviour | Detail |
| --------- | ------ |
| Return type | `DOUBLE` |
| Argument | Any numeric column or expression |
| NULL handling | NULL values are skipped |
| Empty group | Returns `NULL` |
| Memory | Hash map keyed by distinct values; O(d) per group where `d` is distinct value count |

## How it works

MySQL aggregate UDFs follow a five-function lifecycle that MySQL calls once per query (or once per group when used with `GROUP BY`). For an aggregate named `xxx`:

| Function | Called when | What it does |
| -------- | ----------- | ------------ |
| `xxx_init` | Query starts | Allocates per-query state, validates argument count, coerces input types |
| `xxx_clear` | Each new group begins | Resets the accumulator |
| `xxx_add` | Each row in the group | Folds the row's value into the accumulator; skips NULLs |
| `xxx` | Group is complete | Returns the accumulated result |
| `xxx_deinit` | Query finishes | Frees state allocated in `_init` |

Each UDF in this repo follows that pattern with its own state shape:

| UDF | State held in `UDF_INIT->ptr` | Final step |
| --- | ----------------------------- | ---------- |
| `product` | `double` accumulator (init `1.0`) | Return accumulator |
| `median` | `std::vector<double>` of all values | `nth_element` to find middle, average two middles if even count |
| `mode` | `std::unordered_map<double, uint64_t>` of value → count | Linear scan for max count |

## Prerequisites

```bash
apt-get install -y cmake libmysqlclient-dev build-essential
# For ARM cross-compilation only:
apt-get install -y crossbuild-essential-arm64
```

## Building

Output is placed in a `build/` subdirectory. The CMake build supports native compilation and cross-compilation for a different target architecture.

**Native (matches your host machine):**

```bash
mkdir build && cd build
cmake ..
make
```

**Cross-compile for ARM (aarch64) from an x86 host:**

```bash
mkdir build-arm && cd build-arm
cmake .. -DBUILD_ARM=ON
make
```

**Cross-compile for x86_64 from an ARM host:**

```bash
mkdir build && cd build
cmake .. -DBUILD_X86=ON
make
```

The compiled libraries are `product.so`, `median.so`, and `mode.so` — one per UDF.

## Installation

### Manual

1. Copy the libraries you want to use into MySQL's plugin directory:
   ```bash
   PLUGIN_DIR=$(mysql --help | grep 'plugin-dir' | awk '{print $2}')
   sudo cp build/product.so build/median.so build/mode.so "$PLUGIN_DIR"
   ```

2. Register each function in MySQL:
   ```sql
   CREATE AGGREGATE FUNCTION product RETURNS REAL SONAME 'product.so';
   CREATE AGGREGATE FUNCTION median  RETURNS REAL SONAME 'median.so';
   CREATE AGGREGATE FUNCTION mode    RETURNS REAL SONAME 'mode.so';
   ```

3. To unload one later:
   ```sql
   DROP FUNCTION product;
   ```

### Docker test server

The `test_server/` directory contains a Docker Compose setup that spins up a MySQL instance with the plugin pre-loaded — useful for quick testing without touching a local MySQL installation.

1. Build the libraries and copy them into the plugin mount:
   ```bash
   mkdir build && cd build && cmake .. && make && cd ..
   cp build/product.so build/median.so build/mode.so test_server/plugin/
   ```

2. Start the container:
   ```bash
   cd test_server
   docker compose up -d
   ```

3. Connect and load the functions:
   ```bash
   docker compose exec mysql mysql -uroot -proot test
   ```

   ```sql
   SOURCE /entrypoint.sh;  -- or run load_function.sql manually
   CREATE AGGREGATE FUNCTION product RETURNS REAL SONAME 'product.so';
   CREATE AGGREGATE FUNCTION median  RETURNS REAL SONAME 'median.so';
   CREATE AGGREGATE FUNCTION mode    RETURNS REAL SONAME 'mode.so';
   ```

   The container's entrypoint handles the plugin directory automatically — if MySQL's actual plugin path differs from `/usr/lib/mysql/plugin`, it creates a symlink so the mounted `.so` is found regardless.

## Usage

```sql
-- Multiply all values in a column
SELECT product(price) FROM items;

-- Per-group product with GROUP BY
SELECT category, product(multiplier)
FROM adjustments
GROUP BY category;

-- Median latency per endpoint
SELECT endpoint, median(latency_ms)
FROM requests
GROUP BY endpoint;

-- Most common rating per product
SELECT product_id, mode(rating)
FROM reviews
GROUP BY product_id;
```

NULL values are skipped by every aggregate. A group containing only NULLs returns the identity for `product` (`1.0`) and `NULL` for `median` and `mode`.

## Reference

- [YouTube — MySQL UDF walkthrough](https://www.youtube.com/watch?v=ldjdUZDKAFA)

The full specification for writing MySQL UDFs is in the official docs:
[MySQL 8.4 — Adding a Loadable Function](https://dev.mysql.com/doc/extending-mysql/8.4/en/adding-loadable-function.html)

Relevant sections:

- **`UDF_INIT`** — struct passed to every lifecycle function; its `ptr` field is the standard way to carry per-query state across `_init` → `_clear` → `_add` → main → `_deinit` calls
- **`UDF_ARGS`** — struct describing the arguments MySQL passes to each call; `args->args[n]` is a `char*` that MySQL sets to `NULL` for SQL NULLs, which is how `_add` detects and skips them
- **Naming rules** — for an aggregate function named `xxx`, MySQL expects exactly five symbols exported from the `.so`: `xxx_init`, `xxx_clear`, `xxx_add`, `xxx`, and `xxx_deinit`
- **`initid->maybe_null`** — set to `1` in `_init` for `median` and `mode` so MySQL knows the result can be NULL on empty groups; `product` leaves it default since it always returns `1.0`
