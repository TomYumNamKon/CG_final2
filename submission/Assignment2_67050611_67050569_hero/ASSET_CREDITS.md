Current hero revision: no planet, rings, rocks or cloud map are rendered. The Solar System Scope and NASA ring credits below document the earlier revision only. The hero package does not include the cloud texture.

# Assets and visual references

## Downloaded files used in the OpenGL render

**Grab wordmark**
- Source page: https://www.grab.com/th/en/
- Exact asset: https://www.grab.com/th/wp-content/themes/grabsg/public/img/grab_logo_158.png
- Local file: `Textures/FutureMe/grab-official.png`.
- Grab owns the logo/trademark. Used to identify the fictional Grab delivery
  vehicle in this educational project; no endorsement is implied.
- Original PNG is unchanged. GLSL uses its alpha as white lettering on green paint.

**Earth cloud texture — Solar System Scope / INOVE**
- Source and license statement: https://www.solarsystemscope.com/textures/
- Asset: https://www.solarsystemscope.com/textures/download/8k_earth_clouds.jpg
- Local file: `Textures/FutureMe/planet-clouds.jpg` (8192 × 4096).
- License: Creative Commons Attribution 4.0 International (CC BY 4.0):
  https://creativecommons.org/licenses/by/4.0/
- Original JPG is unchanged. Its cloud intensity is sampled with a longitude
  offset and contrast remapping on the fictional planet's spherical mesh.
- The cloud pack is based on NASA imagery/data, as described by its author.

## Original geometry and procedural shading

All active OBJ models were built for this project by
`tools/generate_future_me.py` and `tools/cinematic_scene.py` with Codex assistance.
Hull panels, EVA suit, cargo and nozzle interiors are vertex
geometry loaded by the C++ OBJ loader. Material wear and plasma
exhaust are procedural shaders. No downloaded spacecraft/astronaut mesh is used.

## Visual inspiration

- User's ImageGen concept: `docs/references/future-me-cinematic-concept.png`.
  This is an AI concept reference, not the submitted render and not a background
  image loaded by the OpenGL program.
- *Interstellar* (2014): user-requested lighting/mood reference. No movie frame
  or movie asset is included or used as a texture.
- NASA Cassini rings: https://science.nasa.gov/mission/cassini/science/rings/
- GrabFood service: https://www.grab.com/th/en/food/
