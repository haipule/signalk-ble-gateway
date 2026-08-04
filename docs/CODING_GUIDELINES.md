# Coding guidelines

- Prefer asynchronous, non-blocking implementations.
- Build small modules with clear documented interfaces.
- Encapsulate hardware, network, and time dependencies for testing.
- Use modern C++, RAII, strong types, and `const` correctness.
- Handle errors explicitly and define timeouts, retries, and resource limits.
- Keep manufacturer logic, keys, and Signal K paths out of firmware.
- Keep Signal K internal APIs out of firmware dependencies.
- Version and document public protocol changes.
- Comments explain decisions and edge cases, not obvious code.
