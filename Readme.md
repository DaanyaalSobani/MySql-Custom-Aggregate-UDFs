# MySQL Custom Aggregate Function

This project is the source code for a simple aggregate function that can be loaded into MySQL.

## Overview

This custom aggregate function extends MySQL's built-in functionality by providing [describe the specific functionality your aggregate function provides].

## Prerequisites

- MySQL Server (version X.X or higher)
- C/C++ compiler (for building the UDF)
- MySQL development headers and libraries

## Installation

1. Clone this repository:
   ```bash
   git clone [your-repository-url]
   ```

2. Build the aggregate function:
   ```bash
   make
   ```

3. Copy the compiled library to MySQL's plugin directory:
   ```bash
   sudo cp [library-name].so /usr/lib/mysql/plugin/
   ```

4. Load the function into MySQL:
   ```sql
   CREATE AGGREGATE FUNCTION [function_name] RETURNS [return_type] SONAME '[library-name].so';
   ```

## Usage
