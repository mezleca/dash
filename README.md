# DASH

Platformer Game inspired by Geometry Dash.

## TODO
### CORE
- [x] basic gameplay loop (cube / bird mode, songs, pause, etc...)
- [x] integrate box2d for physics
- [x] integrate imgui-ui for game interface
- [x] game levels
    - [x] serialize / deserialize using json
	- [x] data (objects, etc...)
	- [x] metadata: name
	- [ ] metadata: description
	- [ ] metadata: cover
- [x] sort game objects by z-index before rendering

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
- [ ] level: easy (TBD)
- [ ] level: normal (Bye Bye Sometimes)

### SETTINGS
- [ ] custom sprite (fixed size supporting pngs, svgs and gifs)
- [ ] music volume
- [ ] serialize / deserialize settings data

### EDITOR
  - [ ] add/remove gameobjects
  - [ ] move gameobjects using 2d gizmo
  - [ ] interface to change gameobject properties
  - [ ] interface to change level properties
