# HATs Protocol

HATs (Hub of All Things) protocol implementation for ElizaOS C++ framework.

This protocol provides unified data source management for multiple data formats including CSV, JSON, and extensible support for XML, databases, and APIs.

## Key Features

- Multi-format data source support (CSV, JSON, etc.)
- Type-safe data handling with automatic type inference
- Data processing pipelines (filter, transform, sort, aggregate)
- Multi-source data loading and merging
- Centralized data source management

## Implementation

The C++ implementation is located in `cpp/hats/` and provides:
- Header file: `include/elizaos/hats.hpp`
- Implementation: `cpp/hats/src/hats.cpp` 
- Unit tests: `cpp/hats/test/test_hats.cpp`
- Documentation: `cpp/hats/README.md`

See the detailed documentation in `cpp/hats/README.md` for usage examples and API reference.
