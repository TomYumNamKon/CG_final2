# FUTURE ME — Grab rider hero portrait

Students: 67050611 / 67050569. Add full names to the Classroom comment.

This is a real C++17 / OpenGL 3.3 scene. The concept image is a design reference,
not a background image or a substitute for geometry. The submitted capture is
800 × 600, read directly from the OpenGL framebuffer.

## Current scene

- Original indexed OBJ geometry: armored scout craft, hollow turbine nozzles,
  framed cargo container, EVA rider with a gold visor, ribbed pressure joints, chest controller and stitched straps.
- Close side/front camera focuses on the rider and vehicle. No planets, rings,
  rocks or stars are drawn; the background is a restrained dark gradient.
- Cook–Torrance GGX material lighting, Schlick Fresnel, procedural surface wear.
- One warm directional key light shared by the material shader and shadow camera.
- 4096 × 4096 shadow map with 5 × 5 PCF, local SSAO contact shading.
- Cool rim/engine lights and broad fill reveal shadowed surfaces.
- A 1600 × 1200 HDR scene resolves into an 800 × 600 final image.
- Separate HDR highlight extraction and eight separable Gaussian passes produce
  restrained lens diffusion around bright highlights.
- Two 40-step ray-marched plasma volumes are clipped against the actual scene
  depth; flow animates in the live program and remains deterministic in captures.
- Heat shimmer is a small screen-space distortion confined to the plasma.
- Official Grab alpha wordmark displayed large on two green cargo panels.

This is a cinematic real-time approximation, not a UE5/path-traced renderer or
an 8K output. Lens diffusion is post-processing; space is not filled with fog.
Fill lights and analytic environment reflections are artistic approximations.
Models remain original procedural meshes, not photogrammetry or scanned assets.

## Run in the original workspace

```powershell
cmake --preset win-msvc-2022
cmake --build build/win-msvc-2022 --config Release
./build/win-msvc-2022/Release/OpenGLStarter.exe
./build/win-msvc-2022/Release/OpenGLStarter.exe --capture
```

If dependencies are already installed but vcpkg installation is restricted,
configure with `-DVCPKG_MANIFEST_INSTALL=OFF`.

- Left-drag: orbit camera.
- Space: start/pause the bounded approach flight.
- R: reset camera and flight to the submission composition.
- H: toggle the optional typography overlay (off by default).
- B: toggle diffusion/bloom. K: toggle cast shadows.
- F12: save PNG. Escape: exit.

CLI: `--capture`, `--approach`, `--overlay`, `--no-overlay`, `--no-shadows`,
`--no-bloom`, `--output path.png`.

## Deliverables

`submission/Assignment2_67050611_67050569_hero.zip` contains the current
standalone `Assignment2_67050611.cpp`, shaders, required OBJ meshes/textures,
captured PNG, generation scripts, asset credits and Classroom comment.
The older ZIP is retained as the previous revision.

For a fresh extracted package with vcpkg installed:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/YOUR_NAME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/Release/FutureMe.exe
```

Keep Models, Textures and Shaders alongside the source. GLFW, GLEW, GLM and stb
are external dependencies; no other project-specific C++ files are required.

To rebuild original geometry, run `python tools/generate_future_me.py` with
Pillow installed. It imports `tools/cinematic_scene.py`. Existing downloaded
textures remain unchanged. The generator currently uses Windows Arial fonts
for the optional text graphics. Re-capture before running `tools/package_submission.py`.

## Credits and submission

See `ASSET_CREDITS.md` in the package or `docs/ASSET_CREDITS.md` in the workspace
for the official Grab wordmark, historical asset attribution and
visual references. Paste `CLASSROOM_COMMENT.txt` and add both names.
Submit only through Google Classroom before 19 October 2026, 23:59 Bangkok time.
Nothing is uploaded to Classroom automatically.
