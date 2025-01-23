# AetherNet

AetherNet is a **fast**, **easy-to-use**, and **safe** networking framework designed specifically for **game engine development**. It aims to be modular and **lightweight**, making it simple to add as a submodule to any modern C++ game engine.

## Features

- **Focused on Game Networking**: Provides classes for servers, clients, and (planned) NAT traversal/hole punching.  
- **Easy Integration**: Simple CMake-based build—drop it into your `extern/` or `libs/` folder, then add `add_subdirectory(AetherNet)` and link.  
- **High-Level API** (Roadmap): Planned abstractions for messaging, event-driven design, and reliability layers.  
- **Safety and Reliability**: Conforms to modern C++ best practices, with future support for secure connections, concurrency, and robust error handling.

## Getting Started

### 1. Cloning

```bash
git clone https://github.com/YourUsername/AetherNet.git

``` 
Or if you’re using it as a submodule in another project, run

```bash
git submodule add https://github.com/YourUsername/AetherNet.git
```