# ProcedurePlanet
This is a personal project and I will keep working on it. At now, it provide basic functionality for procedure generated planet.

An individual project aims at creating a procedural generated planet. This demo is implemented with C++/OpenGL/GLSL.

Implemented the adaptive level of detail to base planet geometry. The adaptive level of detail algorithm consists two important components: one is the high-level geometry division, which is driven by subdividing or merging triangles based on the distance between planet and camera. This process is implemented with recursively in our virtual environment function call; the low-level is implemented with openGL tessellation feature, which can divide the base planet sphere into extremely small sub-triangles in order to provide the high resolution of the terrain.

For more details: https://zhang-xiao-mu.blog/2019/01/04/planetary-landscape-generation/
