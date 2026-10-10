# DASH

Platformer Game inspired by Geometry Dash.

# TODO
## CORE
- [x] basic gameplay loop (cube / bird mode, songs, pause, etc...)
- [x] integrate box2d for physics
- [x] integrate imgui-ui for game interface
- [x] sort game objects by z-index before rendering
- [x] serialization

## GAME
- [ ] levels
    - [x] data (objects, etc...)
    - [x] metadata: name
    - [ ] metadata: description
    - [ ] metadata: cover
    
### GAMEPLAY
- [x] object: spike
- [x] object: platform
- [ ] object: trigger
- [ ] object: end
- [x] object: static texture
- [x] player: cube mode
- [x] player: bird mode (aka ship)
- [ ] player: wave mode
- [ ] player: spider mode
- [ ] behavior: invert screen
- [ ] behavior: flip screen
- [ ] behavior: camera control (offset, zoom)
- [x] level: progress (updated on death / finish)
- [ ] level: easy (TBD)
- [ ] level: normal (Bye Bye Sometimes)

### SETTINGS
- [ ] custom sprite (fixed size supporting pngs, svgs and gifs)
- [x] god mode
- [x] free mode (explorer mode)
- [x] music volue
- [x] serialization

### EDITOR
- [ ] add/remove gameobjects
- [ ] move gameobjects using 2d gizmo
- [ ] interface to change gameobject properties
- [ ] interface to change level properties
