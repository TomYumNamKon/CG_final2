"""Produce a standalone C++ submission plus its shaders, OBJs and PNG.
Run after generating assets and capturing the final image.
"""
from pathlib import Path
import shutil,zipfile
root=Path(__file__).resolve().parents[1]
out=root/'submission/Assignment2_67050611_67050569'
out.mkdir(parents=True,exist_ok=True)
source=(root/'src/Assignment2_67050611.cpp').read_text(encoding='utf-8')
for lib in ['Mesh','Window']:
 header=(root/f'src/Libs/{lib}.h').read_text(encoding='utf-8')
 impl=(root/f'src/Libs/{lib}.cpp').read_text(encoding='utf-8').replace(f'#include "{lib}.h"','')
 source=source.replace(f'#include "Libs/{lib}.h"',header+'\n'+impl)
source=source.replace('#include "ProjectPaths.h"','')
source=source.replace('fs::path(OPENGL_STARTER_SHADER_DIR).parent_path()','fs::path(__FILE__).parent_path()')
(out/'Assignment2_67050611.cpp').write_text(source,encoding='utf-8')
for folder in ['Models','Textures','Shaders']:
 shutil.copytree(root/folder/'FutureMe',out/folder/'FutureMe',dirs_exist_ok=True,ignore=shutil.ignore_patterns('*_v1.*'))
shutil.copy2(root/'captures/Assignment2_67050611_67050569.png',out)
shutil.copy2(root/'docs/FUTURE_ME.md',out/'README.md')
shutil.copy2(root/'docs/CLASSROOM_COMMENT.txt',out)
shutil.copy2(root/'tools/generate_future_me.py',out/'generate_future_me.py')
shutil.copy2(root/'vcpkg.json',out)
(out/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.21)
project(FutureMe LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
add_executable(FutureMe Assignment2_67050611.cpp)
target_include_directories(FutureMe PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(FutureMe PRIVATE OpenGL::GL glfw GLEW::GLEW glm::glm)
''')
archive=out.with_suffix('.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
 for p in out.rglob('*'):
  if p.is_file():z.write(p,p.relative_to(out.parent))
print(archive)
