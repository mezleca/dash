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
    - [x] metadata: description
    - [ ] metadata: cover (card)
    
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
- [x] viewport: select one or more objects at once
- [x] viewport: render rect with resize joints
- [x] viewport: move and zoom camera using mouse and scrollwheel
- [x] panel: object properties
- [x] panel: level metadata
- [x] mode
    - [x] Move
    - [x] Select
- [ ] library
    - [ ] GameObject's list
    - [ ] select GameObject from list
    - [ ] add selected GameObject into world
