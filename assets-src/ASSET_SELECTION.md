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
- `Tiles/FarmLand/FarmLand_Tile.png`
- `Tiles/FarmLand/FarmLand_Wet_Tile.png`

Select one 16x16 grass base and one path tile. Dry and wet farmland each use
the 16 cardinal-neighbor variants needed to connect user-tilled shapes. Wet
variants are transparent overlays drawn on the matching dry variant.
The other grass palettes, blob test images, beach, cave, cliff, bridge,
pavement, deck, waterfall, and animated-water sets are excluded.

## Crops

- `Crops/Crops.png`

Select wheat, carrot, tomato, potato, corn, and cabbage, each with a
seed-bag icon, four growth stages, and harvest icon. `Crops_2.png` is an
overlapping subset and remains excluded, along with unused vegetables, berries,
strawberries, grapes, and fruit trees.

## Buildings and Environment

- `Buildings/Buildings/Houses/Wood/House_1_Wood_Base_Red.png`
- `Buildings/Houses_Interiors/Wood_Floor_Tiles.png`
- `Buildings/Houses_Interiors/Interior_Walls.png`
- `Buildings/House_Decor/Beds.png`
- `Buildings/House_Decor/Doors.png`
- `Trees/Big_Oak_Tree.png`
- `Outdoor decoration/Fences.png`
- `Outdoor decoration/Flowers.png`
- `Outdoor decoration/Signs.png`

Use one compact wooden farmhouse, one wood floor tile, one wall section, one bed,
one door, one mature oak, and only the individual fence, flower, and sign regions
referenced by the two final maps. Other houses, unique buildings, interior
furniture, animals, water decorations, break animations, particles, and colour
variants are excluded.

## Inventory Icons

- `Icons/Outline/Tool_Icons_Outline.png`

Select only icons for the hoe, watering can, chosen seed, and chosen harvest.
The crop sheet supplies its own seed and harvest icons. The no-outline
duplicates, general resource/food sheets, and all unrelated icons are excluded.

## UI and Font

- `UI/Cute_Fantasy_Font_5x7.png`
- `UI/Pointer_Click_Anim.png`
- `UI/UI_Frames.png`

Use the white 5x7 font because it contains upper/lowercase letters, numbers,
and punctuation and can be tinted by the existing sprite shader. The black 5x9
font is excluded because it requires a pixel-conversion step before tinting.
Only the letters, digits, and colon currently used by runtime text are selected.
Two 32x32 orange slot-frame cells provide normal and selected inventory states.
The normal frame is also divided into nine reusable slices for scalable inventory
and market panels; no additional complete UI sheet is packed.
`UI_ALL.png`, premade panels, the book, loading icon, crosshairs, ribbons,
large colour/button variant sheets, and duplicate font sheets are excluded.

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
