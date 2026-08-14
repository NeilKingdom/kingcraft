```
 _    _                             __ _
| | _(_)_ __   __ _  ___ _ __ __ _ / _| |_
| |/ / | '_ \ / _` |/ __| '__/ _` | |_| __|
|   <| | | | | (_| | (__| | | (_| |  _| |_
|_|\_\_|_| |_|\__, |\___|_|  \__,_|_|  \__|
              |___/
```

---

# About

You can never have too many crummy Minecraft clones; and so, I've gone ahead and made my own. I started this
project with the intent of learning modern C++, OpenGL, linear algebra, and general game programming concepts.
To avoid feature creep I've listed my "definition of done" below.

### Definition of Done

- [ ] Ability to add and remove blocks
- [ ] Distance-based fog effect
- [ ] Inventory system
- [ ] Day/night cycle
- [ ] Semi-dynamic shadows via shaders
- [ ] Biomes
- [ ] Heads up Display (HUD)
- [x] Proceduraly generated terrain
- [x] Heightmaps using perlin noise
- [x] Dynamic window scaling
- [x] ImGui for debugging and modifying settings at runtime
- [x] Gravity and collision checking

### Goals

| Goal | Explanation |
|------|-------------|
| Utilize C++20 features | The goal is to be as idiomatic as possible. This includes using references, smart pointers, classes, iterators, etc. where appropriate. |
| Minimize dependencies | To maximize my understanding of OpenGL, I utilize as few 3rd party dependencies as possible. I use my own libraries for texture loading (libimc) and math calculations (quikmafs). I do use the OpenGL Extension Wrangler (GLEW), because It'd just be dumb to write out all the function entry points myself. I also use ImGui, primarily just for debugging purposes. |
| Portability | Currently, the project only compiles for Linux with an X11 backend. I'd like to at least add support for Windows, but may end up supporting Mac and/or Wayland as well. |
| Acceptable FPS | This is an entirely arbitrary metric. I'd like to see the game running at a capped framerate of 60FPS on my Thinkpad X220 and running uncapped at ~250FPS on my PC (AMD Ryzen 7 5700X and Nvidia 1080 Ti). |
| Documentation | The project will be documented using Doxygen. Since this is a learning tool, and since I'd like to make it extensible for anybody who wants to fork the project, adding documentation is crucial. |

# Dependencies

As mentioned in the About section, this library relies primarily on libraries that I've written myself, as well
as ImGui. I've made a fork of [Nemertingas](https://github.com/Nemirtingas/imgui)' fork of ImGui, which
includes an X11 backend since I did not want to rely on GLFW or GLUT for this project. I've stripped out all
the functionality that was not required from his fork.

# Usage

N/A

---

# Acknowledging Our Lord

✝ May these works glorify and honour Christ, our Lord ✝

_“So whether you eat or drink or whatever you do, do it all for the glory of God.”_ – 1 Corinthians 10:31
