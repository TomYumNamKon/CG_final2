# FUTURE ME — Orbital delivery / 2086

Concept: in the future I am still a GrabFood rider, but my delivery route crosses
space. An original green space scooter carries a suited courier and an insulated
food box toward a fictional ringed destination, Kepler Port 04.

Students: **67050611 / 67050569**. Add both full names before submitting.
Deadline supplied by the assignment: **19 October 2026, 23:59 (Bangkok)**.

## Assets and references

All 22 OBJ meshes were created specifically for this scene by vertex construction
in `tools/generate_future_me.py`: ellipsoids, tubes, boxes, swept fins and annuli.
The renderer actually loads these OBJ files through the existing Mesh loader.
No downloaded third-party model or image texture is used. Cargo lettering and
poster typography are generated text textures; space background is procedural GLSL.
Geometry and code were developed with Codex assistance; review and understand them
before submission and follow your course's AI-use rules.

Visual/concept references consulted:

1. GrabFood Thailand — brand/service reference for green courier equipment,
   food delivery and the GrabFood name. Lettering is typeset, not a downloaded logo.
   https://www.grab.com/th/en/food/
2. NASA Science, Cassini: Rings — ringed-planet visual inspiration.
   https://science.nasa.gov/mission/cassini/science/rings/
   The destination is fictional; the rendered planet is not a NASA image or a
   scientifically accurate Saturn model.

## Rendering

- C++17, OpenGL 3.3 core, VAO/VBO/EBO indexed triangles.
- World-space Cook–Torrance GGX shading with Schlick Fresnel, roughness and metallic response; inverse-transpose normal matrix.
- Warm directional key, cool fill/rim and two attenuated engine lights.
- 4096 × 4096 depth shadow map with 5 × 5 PCF and slope-dependent bias.
  Shadows apply to the courier and vehicle; distant planet/rings are excluded
  from the local shadow volume. An analytic sphere intersection casts the planet shadow onto its rings. Ring shadows on the planet are not modeled.
- HDR scene rendered at 1600 × 1200, sampled down to an exact 800 × 600 output.
- Bloom, ACES-style tone mapping, gamma correction, subtle vignette.
- Fixed composition, deterministic stars and geometry; no random frame changes.
- Window resizing letterboxes the fixed 4:3 render; PNG always stays 800 × 600.

## Run the workspace

```powershell
cmake --preset win-msvc-2022
cmake --build build/win-msvc-2022 --config Release
./build/win-msvc-2022/Release/OpenGLStarter.exe
./build/win-msvc-2022/Release/OpenGLStarter.exe --capture
```

If dependencies are already installed and vcpkg installation is restricted,
configure with `-DVCPKG_MANIFEST_INSTALL=OFF`.

Left-drag: orbit camera. Space: start/pause approach flight. R: reset composition and flight. H: toggle typography.
B: toggle bloom. K: toggle shadows. F12: save PNG. Escape: exit.

CLI: `--capture`, `--no-overlay`, `--no-shadows`, `--no-bloom`,
`--output path.png`, `--approach` (fixed mid-flight preview). `--capture` renders three frames, saves and exits.
Clean artwork: `--capture --no-overlay --output captures/clean.png`.

Existing `src/main.cpp` and original shaders are retained. The CMake target now
builds `src/Assignment2_67050611.cpp`. The original OBJ loader was retained with
index validation and unsigned hash arithmetic fixes.

## Portable submission

`submission/Assignment2_67050611_67050569.zip` contains a single amalgamated
`Assignment2_67050611.cpp` (Mesh and Window implementations included), shaders,
OBJ assets, textures, reference notes, comment draft and the captured PNG.
The source is self-contained except for GLFW, GLEW, GLM and stb dependencies.
Keep its Models, Textures and Shaders folders alongside the C++ source.

Build extracted submission with a vcpkg toolchain, for example:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/Users/YOUR_NAME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/Release/FutureMe.exe
```

Submit via **Google Classroom only**. Upload the named C++ file and PNG plus
required shaders/assets (ZIP if your instructor accepts ZIP). Paste the reference
comment and add both full names. Nothing has been uploaded automatically.

To regenerate in the original workspace, install Pillow and run
`python tools/generate_future_me.py`, recapture, then
`python tools/package_submission.py`. The generator copy in the submission is
for provenance; its paths assume the original workspace's tools directory.

## Detail revision — 15 October 2026

- Profile-built tapered hull, swept fins, panels and fasteners.
- Beveled insulated cargo case with metal edge guards, locks and rear GrabFood label.
- Engine nozzle petals, plumbing, heat vents and segmented exhaust.
- Pressure-suit fabric, harnesses, oxygen hoses, knee joints, boot treads and fingers.
- Physically inspired GGX materials separate coated paint, titanium, brass, cloth,
  rubber and visor. Fine procedural wear and relief are filtered by the final render.
- SSAO adds local contact shading; a softer fill exposes details inside cast shadows.
- Fictional inhabited planet has procedural land, oceans, clouds, polar ice,
  atmospheric limb and night-side lights; a modeled orbital terminal is ahead.
- Nose direction is calculated from ship origin to planet center. Space advances
  the ship along that vector, including its shadow and engine-light positions.
  Flight stops after a bounded approach; this is not a landing simulation.
- Deterministic screenshots use the starting pose unless --approach is passed.
- No full path tracing or image-based environment map: glossy reflections use
  analytic lighting and a simplified reflection of the destination planet.

Previous composition is preserved as captures/version1.png in the original workspace.
