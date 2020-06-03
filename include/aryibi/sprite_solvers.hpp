#ifndef ARYIBI_SPRITE_SOLVERS_HPP
#define ARYIBI_SPRITE_SOLVERS_HPP

namespace aryibi::sprites {

/// Solves an 8-directional sprite atlas contained in a texture chunk.
/// Accepts both horizontally and vertically-stored sprite atlases.
Sprite solve_directional(TextureChunk const&, direction::Direction dir);

/// Solves a normal tile from a TextureChunk, which literally means "copy the data from this
/// TextureChunk to a Sprite".
Sprite solve_normal(TextureChunk const&);

/// Solves a RPGMaker A2 autotile from a set of 8 connections (Depicting what the tile is connected
/// to).
Sprite solve_rpgmaker_a2(TextureChunk const&, Tile8Connections const& connections);

/// Solves a RPGMaker A4 wall autotile from a set of 4 connections (Depicting what the tile is
/// connected to). RPGMaker A4 walls only work with CONVEX shapes, so trying to do an inner corner
/// will result in a broken autotile.
Sprite solve_rpgmaker_a4_wall(TextureChunk const&, Tile4Connections const& connections);

} // namespace aryibi::sprites

#endif // ARYIBI_SPRITE_SOLVERS_HPP