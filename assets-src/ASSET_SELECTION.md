# Homestead Runtime Asset Selection

This list defines the licensed source-art sheets that may contribute pixels to
`data.pak`. Source PNG files remain local under `assets-src/`; the
runtime package must contain only the selected regions, not complete source
files.

## Player

Use one fixed farmer appearance. Character customization is out of scope.

- `Player/Player_Base/Player_Base_animations.png`
- `Player/Head/Hair_1/Hair_1_Brown.png`
- `Player/Chest/Farmer_Shirt/Farmer_Shirt_1_White_and_Brown.png`
- `Player/Legs/Farmer_Pants/Farmer_Pants_1_White_and_Brown.png`
- `Player/Feet/Shoes_1_Brown.png`

Only idle, walk, hoe, and watering frames required by gameplay are selected
from the aligned layer sheets. Unused actions and duplicate directions must not
be copied into the runtime atlas.

`player-frames.tsv` defines the selected 64x64 player cells and corresponding
64x64 tool cells. The right-facing frames are mirrored at runtime for left-facing
movement, avoiding a duplicate copy in the atlas.

## Tools

- `Player/Tools/Iron/Iron_Tools.png`

Select only the hoe and watering-can artwork. The sword and all fishing, bow,
torch, and lantern sheets are excluded.

## Ground and Farm Tiles

- `Tiles/Grass/Grass_1_Middle.png`
- `Tiles/Grass/Path_Middle.png`
- `Tiles/Grass/Path_Decoration.png`
- `Tiles/FarmLand/FarmLand_Tile.png`
- `Tiles/FarmLand/FarmLand_Wet_Tile.png`

Select one 16x16 grass base, the minimum path edge set, and dry/wet farmland states.
The other grass palettes, blob test images, beach, cave, cliff, bridge,
pavement, deck, waterfall, and animated-water sets are excluded.

## Crops

- `Crops/Crops.png`

Select one compact vegetable crop with seed-bag icon, growth stages, and
harvest icon. A second crop may be added only during the content pass if the
measured atlas budget permits it. `Crops_2.png` is an overlapping subset and is
excluded, as are berries, grapes, and fruit trees.

## Buildings and Environment

- `Buildings/Buildings/Houses/Wood/House_1_Wood_Green_Red.png`
- `Trees/Big_Oak_Tree.png`
- `Outdoor decoration/Fences.png`
- `Outdoor decoration/Flowers.png`
- `Outdoor decoration/Scarecrows.png`
- `Outdoor decoration/Signs.png`

Use one farmhouse appearance, one mature oak, and only a few individual
decoration tiles needed by the final map. Other houses, unique buildings,
interiors, animals, water decorations, break animations, particles, and colour
variants are excluded.

## Inventory Icons

- `Icons/Outline/Tool_Icons_Outline.png`

Select only icons for the hoe, watering can, chosen seed, and chosen harvest.
The crop sheet supplies its own seed and harvest icons. The no-outline
duplicates, general resource/food sheets, and all unrelated icons are excluded.

## UI and Font

- `UI/Cute_Fantasy_Font_5x7.png`
- `UI/Pointer_Click_Anim.png`
- `UI/UI_Premade.png`
- `UI/UI_Pop_Up.png`
- `UI/UI_Bars.png`
- `UI/UI_Button_Icons.png`
- `UI/UI_Selectors.png`
- `UI/UI_Sliders.png`

Use the white 5x7 font because it contains upper/lowercase letters, numbers,
and punctuation and can be tinted by the existing sprite shader. The black 5x9
font is excluded because it requires a pixel-conversion step before tinting.
`UI_ALL.png`, the book, loading icon, crosshairs, ribbons, large colour/button
variant sheets, and duplicate font sheets are excluded. Large approved UI
sheets are source references only until exact panels and controls are chosen in
the UI stage.

## Explicitly Excluded Categories

- NPCs, enemies, combat equipment, projectiles, and caves/mining
- Animals, mounts, fishing, boats, markets, shops, and dialogue content
- Weather effects and seasonal variants
- Character customization and alternate colours
- Animated water decorations and environmental destruction animations
- Unselected UI themes, duplicate fonts, and complete UI compilation sheets
- Any source file or sprite region not named by this document

## Packer Follow-up

`manifest.tsv` is the machine-readable source allowlist. `sprites.tsv` contains
the exact static regions currently approved for the runtime atlas. Large source
sheets are reduced to individual tiles or objects; they are never copied whole.
Player
animation regions and tool overlays are defined by `player-frames.tsv`. The
packer must fail when a listed source is missing or a rectangle is out of bounds,
and must never scan the licensed-art directory implicitly.
