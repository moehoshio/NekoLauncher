cmake_minimum_required(VERSION 3.20)
project(NekoLcTests LANGUAGES CXX)

# =====================
# == NekoLc App Test ==
# =====================
add_subdirectory(app)

# =====================
# = NekoLc Core Test ==
# =====================
add_subdirectory(core)

# =====================
# NekoLc Minecraft Test
# =====================
add_subdirectory(minecraft)