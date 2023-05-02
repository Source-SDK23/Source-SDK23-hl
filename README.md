# Source-SDK23
A fork of Mapbase with many additional gameplay elements and improvements.

## Changes/Additions
- Added FStop mechanic via `weapon_camera`
- Add `env_portal_beam` (red thermal discouragement beam from `portal 2` with `P2CE` mechanics)
- Add `env_portal_laser` (entity which emmits `env_portal_laser`)
- Add `env_laser_catcher` (entity which changes state when it is hit by an `env_portal_laser`)

## Planned Changes/Additions
- Add `prop_weighted_cube`
- Add `prop_schrodinger_cube` (cube which "teleports" an `env_portal_laser` which hits it to another linked cube)
- Add `prop_wall_entity` (hardlight bridge)
- Add `prop_wall_projector` (projector which emits `prop_wall_entity`)
- Add `trigger_faithplate`
- Add functional portals
- Add PBR Shading (from [`here`](https://developer.valvesoftware.com/wiki/Adding_PBR_to_Your_Mod))
- SSAO (from [`here`](https://github.com/BSVino/DoubleAction/commit/0ded18e5d52ef199e766f5f703c9ab65ecf649a3))
- Add `prop_tractor_beam`