# EngineC
An educational game engine written in C. It is heavily inspired by the Kohi Game Engine (https://github.com/travisvroman/kohi).

## Why C
I chose C as my language of choice for this project because I wanted to lower the language complexity to a minimum to keep a simple codebase. The goal here is not to make cool abstractions and idomatic OOP, the complexity should be in the code itself.

## Goals
This main goal of this project is to make a stable, usable engine. Once this project reaches a usable state, more precise goals can be determined. Until then, performance or features are not the main focus.

## Roadmap
Until the first stable release, all commits that are not considered as "side-tangent" or bugfixes will be committed to the `master` branch. This is done to be able to easily track the progress of the project in its early states.

The first steps of this project will be to create basic layers, like platform, memory management, windowing, etc.
Once these layers are ready, focus will be set to the rendering part of the engine. Occasional peeks to other engine parts will occur (like resource loading, threading, etc.)

## Building from source
This project uses [premake5](https://github.com/premake/premake-core) to generate a build system.

### Prerequisites
- Premake5 (https://github.com/premake/premake-core)
- Vulkan SDK (https://vulkan.lunarg.com)

### Building
After cloning the repository, use premake5 to generate a build system. For example, to generate a VS2022 solution:
```
premake5 vs2022
```
For available build systems, run `premake5 --help`.

You can then use the generated build system to build the project.

## Coding Style/Guidelines
- 130 characters per line
- No line break before brackets
- Doxygen comments following this format:
```c
/**
 * @brief This is a brief message.
 * 
 * Explanation of the function (if needed)).
 * 
 * @param[in] param1 Description of param1.
 * @param[out] param2 Description of param2.
 * @param[in,out] param3 Description of param3.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 * OR
 * @return Description of return value.
 */
```
