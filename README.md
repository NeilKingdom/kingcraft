```
 _    _                             __ _
| | _(_)_ __   __ _  ___ _ __ __ _ / _| |_
| |/ / | '_ \ / _` |/ __| '__/ _` | |_| __|
|   <| | | | | (_| | (__| | | (_| |  _| |_
|_|\_\_|_| |_|\__, |\___|_|  \__,_|_|  \__|
              |___/
```

✝️ This software was written in the name of the __Father__ and of the __Son__ and of the __Holy Spirit__; Amen.

# About

You can never have too many crummy Minecraft clones; and so, I've gone ahead and made my own. This project
was written exclusively as a learning opportunity for me, so don't expect great things. That being said, I
have self-imposed some goals and features which will mark the completion of the project once reached. You can
view these self-imposed goals and features below.

### Goals

| Goal | Explanation |
|------|-------------|
| Written in C++ 20 | The goal is to be as idiomatic as possible. This includes using references, smart
pointers, classes, iterators, etc. where possible. |
| As few dependencies as possible | To maximize my understanding of OpenGL, I utilize as few 3rd party
dependencies as possible. I use my own libraries for texture loading (libimc) and linear algeabra (liblac). I
do use the OpenGL Extension Wrangler (GLEW), because It'd just be dumb to write out all the function entry
points myself. I also use ImGui, primarily just for debugging purposes. |
| Portable | Currently, the project only compiles for Linux with an X11 backend. I'd like to at least add
support for Windows, but may end up supporting Mac and/or Wayland as well. |
| Acceptable FPS | This is entirely arbitrary, and I've need even decided what I'd deem acceptable in my own
standards yet. At a bare minimum, the application should reach 60 FPS and be able to lock at that frame rate.
I'd like to reach an FPS of at least 250 with no optimizations turned on. |
| Documentation | The project will be documented using Doxygen. Since this is a learning tool, and since I'd
like to make it extensible for anybody who wants to fork the project, adding documentation is crucial. |

### Feature list

- [x] Infinite terrain
- [x] Heightmaps using perlin noise algorithm
- [x] Frustum culling
- [x] Dynamic scaling proportional to window size
- [x] ImGui window for debugging and modifying settings at runtime
- [ ] Ability to add or remove blocks
- [ ] Fog
- [ ] Inventory system
- [ ] Gravity and collision checking
- [ ] Day/night cycle
- [ ] Biomes
- [ ] Heads up Display (HUD)

# Dependencies

As mentioned in the About section, this library relies primarily on libraries that I've written myself, as well
as ImGui. I've made a fork of [Nemertingas'](https://github.com/Nemirtingas/imgui) fork of ImGui, which
includes an X11 backend since I did not want to rely on GLFW or GLUT for this project. I've stripped out all
the functionality that was not required from his fork. As for the other libraries, you'll require libimc and
liblac.

# Installation

I'll make a binary one day, but until I do that, you'll need to build everything from scratch. Begin by
cloning this repo, followed by [liblac](https://github.com/NeilKingdom/liblac) and
[libimc](https://github.com/NeilKingdom/libimc). Build liblac and libimc by running the following command:

```console
sudo make install
```

The build target for both of these is Linux (specifically Arch), so these will be copied to /usr/lib. Next,
clone this repo and the ImGui submodule:

```
git clone --recurse-submodules https://github.com/NeilKingdom/kingcraft.git kingcraft
```

Run make one more time and execute the kingcraft binary that was produced.
