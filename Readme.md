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

## How it works

MySQL aggregate UDFs follow a four-function lifecycle that MySQL calls once per query (or once per group when used with `GROUP BY`):

| Function | Called when | What it does |
| -------- | ----------- | ------------ |
| `product_init` | Query starts | Allocates state, validates argument count, coerces input to `REAL` |
| `product_clear` | Each new group begins | Resets accumulator to `1.0` |
| `product_add` | Each row in the group | Multiplies the accumulator by the row's value; skips NULLs |
| `product` | Group is complete | Returns the accumulated result |
| `product_deinit` | Query finishes | Frees allocated state |

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

The compiled library is `product.so`.

## Installation

### Manual

1. Copy the library to MySQL's plugin directory:
   ```bash
   sudo cp build/product.so $(mysql --help | grep 'plugin-dir' | awk '{print $2}')
   ```

2. Register the function in MySQL:
   ```sql
   CREATE AGGREGATE FUNCTION product RETURNS REAL SONAME 'product.so';
   ```

3. To unload it later:
   ```sql
   DROP FUNCTION product;
   ```

### Docker test server

The `test_server/` directory contains a Docker Compose setup that spins up a MySQL instance with the plugin pre-loaded — useful for quick testing without touching a local MySQL installation.

1. Build the library and copy it into the plugin mount:
   ```bash
   mkdir build && cd build && cmake .. && make && cd ..
   cp build/product.so test_server/plugin/
   ```

2. Start the container:
   ```bash
   cd test_server
   docker compose up -d
   ```

3. Connect and load the function:
   ```bash
   docker compose exec mysql mysql -uroot -proot test
   ```

   ```sql
   SOURCE /entrypoint.sh;  -- or run load_function.sql manually
   CREATE AGGREGATE FUNCTION product RETURNS REAL SONAME 'product.so';
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

-- Works inside expressions
SELECT order_id, product(quantity) * base_price AS total
FROM order_lines
GROUP BY order_id;
```

NULL values in the column are skipped. A group containing only NULLs returns `1.0`.

## Reference

- [YouTube — MySQL UDF walkthrough](https://www.youtube.com/watch?v=ldjdUZDKAFA)

The full specification for writing MySQL UDFs is in the official docs:
[MySQL 8.4 — Adding a Loadable Function](https://dev.mysql.com/doc/extending-mysql/8.4/en/adding-loadable-function.html)

Relevant sections:

- **`UDF_INIT`** — struct passed to every lifecycle function; its `ptr` field is the standard way to carry per-query state across `_init` → `_clear` → `_add` → main → `_deinit` calls
- **`UDF_ARGS`** — struct describing the arguments MySQL passes to each call; `args->args[n]` is a `char*` that MySQL sets to `NULL` for SQL NULLs, which is how `product_add` detects and skips them
- **Naming rules** — for an aggregate function named `xxx`, MySQL expects exactly five symbols exported from the `.so`: `xxx_init`, `xxx_clear`, `xxx_add`, `xxx`, and `xxx_deinit`
