# SDK23
A fork of Mapbase with many additional gameplay elements and improvements to bring Source-SDK2013 to 2023.

*NOTE: This fork was abandoned after being partially implemented, I no longer want to work with source engine*
*PLEASE GIVE CREDIT IF YOU USE ANY OF THIS CODE*

## Changes/Additions
- Added FStop mechanic via `weapon_camera`
- Add `env_portal_beam` (red thermal discouragement beam from `portal 2` with `P2CE` mechanics)
- Add `env_portal_laser` (entity which emmits `env_portal_laser`)
- Add `env_laser_catcher` (entity which changes state when it is hit by an `env_portal_laser`)
- Add `prop_weighted_cube`
- Add schrodinger's cube (cube which "teleports" an `env_portal_laser` which hits it to another linked cube) (available as additional `prop_weighted_cube` type)

## Planned Changes/Additions
- Add `prop_testchamber_door`
- Add `prop_wall_entity` (hardlight bridge)
- Add `prop_wall_projector` (projector which emits `prop_wall_entity`)
- Add `trigger_faithplate`
- Add functional portals
- Add PBR Shading (from [`here`](https://developer.valvesoftware.com/wiki/Adding_PBR_to_Your_Mod))
- SSAO (from [`here`](https://github.com/BSVino/DoubleAction/commit/0ded18e5d52ef199e766f5f703c9ab65ecf649a3))
- Add `prop_tractor_beam`

## Broken elements
- `prop_laser_relay` exists but is nonfunctional
- `env_laser_catcher` does not animate
- RGB colours only apply to the beam and do not currently change the particle colours (will be fixed)
