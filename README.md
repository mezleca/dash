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
- [ ] level related stuff
    - [x] metadata: name
    - [x] metadata: description
    - [ ] metadata: cover (card)
    - [x] solid background
    - [x] progress (updated on death / finish)
    - [x] start and finish pos (automatic, custom)
    - [ ] easy (song: TBD)
    - [ ] normal (song: Bye Bye Sometimes)
    
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
- [ ] behaviour: invert screen
- [x] behaviour: camera focus
- [x] behaviour: player type
- [ ] behaviour: color transition (multiple targets)
- [ ] behaviour: change gravity (multiple targets)
- [ ] behaviour: flip screen
- [x] hud: simple gameplay progress

### SETTINGS
- [ ] custom sprite (fixed size supporting pngs, svgs and gifs)
- [x] god mode
- [x] free mode (explorer mode)
- [x] music volue
- [x] serialization

### EDITOR
- [ ] make it less ugly (theming, custom widgets, etc...)
- [x] viewport: select one or more objects at once
- [x] viewport: render rect with resize joints
- [x] viewport: move and zoom camera using mouse and scrollwheel
- [x] viewport: select objects from viewport
- [x] panel: object properties
- [x] panel: level metadata
- [x] panel: allow add, remove, modify behaviours
- [x] library
    - [x] GameObject's list
    - [x] select add GameObject from list
