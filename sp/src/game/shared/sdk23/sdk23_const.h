// SDK23 shared definitions
#ifndef SDK23_CONST_H
#define SDK23_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum RustedSkinType {
	SKIN_CLEAN=0,
	SKIN_RUSTED
};

/*
* Do not question the cube enums
* At least it's better than misspelling schroedinger as... "schoedinger"
* ~hacker
*/
enum CubeType {
	CUBE_WEIGHTED_STORAGE = 0,
	CUBE_WEIGHTED_COMPANION,
	CUBE_DISCOURAGEMENT_REDIRECTION,
	CUBE_SPHERE_CUBE, // Edgeless safety cube didn't seem right in an enum... ~hacker
	CUBE_ANTIQUE,
	CUBE_QUANTUM, // Imagine misspelling schroedinger ~hacker
	CUBE_CUSTOM
};

enum PaintType {
	PAINT_REPULSION = 0,
	PAINT_ADHESION,
	PAINT_PROPULSION,
	PAINT_CONVERSION,
	PAINT_NONE
};

#endif // SDK23_CONST_H