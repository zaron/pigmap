// Copyright 2010, 2011 Michael J. Nelson
//
// This file is part of pigmap.
//
// pigmap is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// pigmap is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with pigmap.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BLOCKIMAGES_H
#define BLOCKIMAGES_H

#include <stdint.h>

#include "rgba.h"



// this structure holds the block images used to build the map; each block image is a hexagonal shape within
//  a 4Bx4B rectangle, with the unused area around it set to fully transparent
//
// example of hexagon shape for B = 3, where U represents pixels belonging to the U-facing side of the block, etc.:
//
//        UU
//      UUUUUU
//    UUUUUUUUUU
//   NUUUUUUUUUUW
//   NNNUUUUUUWWW
//   NNNNNUUWWWWW
//   NNNNNNWWWWWW
//   NNNNNNWWWWWW
//   NNNNNNWWWWWW
//    NNNNNWWWWW
//      NNNWWW
//        NW
//
// when supplying your own block images, there's nothing to stop you from going "out of bounds" and having
//  non-transparent pixels outside the hexagon, but you'll just get a messed-up image, since the renderer
//  uses only the hexagon to determine visibility, etc.
//
// note that translucent blocks require the most work to render, simply because you can see what's behind them;
//  if every block in the world was translucent, for example, then every block would be considered visible
// ...so if you're editing the block images for special purposes like X-ray vision, the fastest results are
//  obtained by making unwanted blocks fully transparent, not just translucent
// ...also, any pixels in the block images with alphas < 10 will have their alphas set to 0, and similarly
//  any alphas > 245 will be set to 255; this is to prevent massive slowdown from accidental image-editing
//  cock-ups, like somehow setting the transparency of the whole image to 99% instead of 100%, etc.
//
// most block images are created by resizing the relevant terrain.png images from 16x16 to 2Bx2B, then painting
//  their columns onto the faces of the block image thusly (example is for B = 3 again):
//
//                                     a                    f
// abcdef              ab              abc                def
// abcdef            aabbcd            abcde            bcdef
// abcdef  --->    aabbccddef    or    abcdef    or    abcdef
// abcdef          abccddeeff          abcdef          abcdef
// abcdef            cdeeff            abcdef          abcdef
// abcdef              ef               bcdef          abcde
//                                        def          abc
//                                          f          a

struct BlockImages
{
	// this image holds all the block images, in rows of 16 (so its width is 4B*16; height depends on number of rows)
	// ...the very first block image is a dummy one, fully transparent, for use with unrecognized blocks
	RGBAImage img;
	int rectsize;  // size of block image bounding boxes

	// the number of block images
	static const int NUMIMAGES;

	// for every possible 8-bit block id/4-bit block data combination, this holds the offset into the image
	//  (unrecognized id/data values are pointed at the dummy block image)
	// this doesn't handle some things like fences and double chests where the rendering doesn't depend solely
	//  on the blockID/blockData; for those, the renderer just has to know the proper offsets on its own
	int blockOffsets[256 * 16];
	int getOffset(uint8_t blockID, uint8_t blockData) const {return blockOffsets[blockID * 16 + blockData];}

	// check whether a block image is opaque (this is a function of the block images computed from the terrain,
	//  not of the actual block data; if a block image has 100% alpha everywhere, it's considered opaque)
	std::vector<bool> opacity;  // size is NUMIMAGES; indexed by offset
	bool isOpaque(int offset) const {return opacity[offset];}
	bool isOpaque(uint8_t blockID, uint8_t blockData) const {return opacity[getOffset(blockID, blockData)];}

	// ...and the same thing for complete transparency (0% alpha everywhere)
	std::vector<bool> transparency;  // size is NUMIMAGES; indexed by offset
	bool isTransparent(int offset) const {return transparency[offset];}
	bool isTransparent(uint8_t blockID, uint8_t blockData) const {return transparency[getOffset(blockID, blockData)];}

	// get the rectangle in img corresponding to an offset
	ImageRect getRect(int offset) const {return ImageRect((offset%16)*rectsize, (offset/16)*rectsize, rectsize, rectsize);}
	ImageRect getRect(uint8_t blockID, uint8_t blockData) const {return getRect(getOffset(blockID, blockData));}

	// attempt to create a BlockImages structure: look for blocks-B.png in the imgpath, where B is the block size
	//  parameter; failing that, look for terrain.png and construct a new blocks-B.png from it; failing that, uh, fail
	bool create(int B, const std::string& imgpath);

	// set the offsets
	void setOffsets();

	// fill in the opacity and transparency members
	void checkOpacityAndTransparency(int B);

	// scan the block images looking for not-quite-transparent or not-quite-opaque pixels; if they're close enough,
	//  push them all the way
	void retouchAlphas(int B);

	// build block images from terrain.png
	bool construct(int B, const std::string& terrainfile, const std::string& firefile);
};

// block image offsets:
//
// 0 dummy/air (transparent) 32 brown mushroom         64 wheat level 2          96 cobble stairs asc S
// 1 stone                   33 red mushroom           65 wheat level 1          97 cobble stairs asc N
// 2 grass                   34 gold block             66 wheat level 0          98 cobble stairs asc W
// 3 dirt                    35 iron block             67 farmland               99 cobble stairs asc E
// 4 cobblestone             36 double stone slab      68 UNUSED                 100 wall sign facing E
// 5 wood                    37 stone slab             69 UNUSED                 101 wall sign facing W
// 6 sapling                 38 brick                  70 sign facing N/S        102 wall sign facing N
// 7 bedrock                 39 TNT                    71 sign facing NE/SW      103 wall sign facing S
// 8 water full/falling      40 bookshelf              72 sign facing E/W        104 UNUSED               
// 9 water level 7           41 mossy cobblestone      73 sign facing SE/NW      105 UNUSED
// 10 water level 6          42 obsidian               74 wood door S side       106 UNUSED
// 11 water level 5          43 torch floor            75 wood door N side       107 UNUSED
// 12 water level 4          44 torch pointing S       76 wood door W side       108 UNUSED
// 13 water level 3          45 torch pointing N       77 wood door E side       109 UNUSED
// 14 water level 2          46 torch pointing W       78 wood door top S        110 stone pressure plate
// 15 water level 1          47 torch pointing E       79 wood door top N        111 iron door S side
// 16 lava full/falling      48 UNUSED                 80 wood door top W        112 iron door N side
// 17 lava level 3           49 spawner                81 wood door top E        113 iron door W side
// 18 lava level 2           50 wood stairs asc S      82 ladder E side          114 iron door E side
// 19 lava level 1           51 wood stairs asc N      83 ladder W side          115 iron door top S
// 20 sand                   52 wood stairs asc W      84 ladder N side          116 iron door top N
// 21 gravel                 53 wood stairs asc E      85 ladder S side          117 iron door top W
// 22 gold ore               54 chest facing W         86 track EW               118 iron door top E
// 23 iron ore               55 redstone wire NSEW     87 track NS               119 wood pressure plate
// 24 coal ore               56 diamond ore            88 UNUSED                 120 redstone ore
// 25 log                    57 diamond block          89 UNUSED                 121 red torch floor off
// 26 leaves                 58 workbench              90 UNUSED                 122 red torch floor on
// 27 sponge                 59 wheat level 7          91 UNUSED                 123 UNUSED
// 28 glass                  60 wheat level 6          92 track NE corner        124 UNUSED
// 29 white wool             61 wheat level 5          93 track SE corner        125 UNUSED
// 30 yellow flower          62 wheat level 4          94 track SW corner        126 UNUSED
// 31 red rose               63 wheat level 3          95 track NW corner        127 snow
//
// 128 ice                   160 fence NS              192 stone button facing W 224 dispenser N
// 129 snow block            161 fence E               193 stone button facing E 225 dispenser E/S
// 130 cactus                162 fence NE              194 wall lever facing S   226 sandstone
// 131 clay                  163 fence SE              195 wall lever facing N   227 note block
// 132 reeds                 164 fence NSE             196 wall lever facing W   228 cake
// 133 jukebox               165 fence W               197 wall lever facing E
// 134 fence post            166 fence NW              198 ground lever EW
// 135 pumpkin facing W      167 fence SW              199 ground lever NS
// 136 netherrack            168 fence NSW             200 track asc S
// 137 soul sand             169 fence EW              201 track asc N
// 138 glowstone             170 fence NEW             202 track asc E
// 139 portal                171 fence SEW             203 track asc W
// 140 jack-o-lantern W      172 fence NSEW            204 orange wool
// 141 red torch S on        173 double chest N        205 magenta wool
// 142 red torch N on        174 double chest S        206 light blue wool
// 143 red torch E on        175 double chest E        207 yellow wool
// 144 red torch W on        176 double chest W        208 lime wool
// 145 red torch S off       177 chest facing N        209 pink wool
// 146 red torch N off       178 water missing W       210 gray wool
// 147 red torch E off       179 water missing N       211 light gray wool
// 148 red torch W off       180 ice surface           212 cyan wool
// 149 UNUSED                181 ice missing W         213 purple wool
// 150 UNUSED                182 ice missing N         214 blue wool
// 151 UNUSED                183 furnace W             215 brown wool
// 152 UNUSED                184 furnace N             216 green wool
// 153 pumpkin facing E/S    185 furnace E/S           217 red wool
// 154 pumpkin facing N      186 lit furnace W         218 black wool
// 155 jack-o-lantern E/S    187 lit furnace N         219 pine log
// 156 jack-o-lantern N      188 lit furnace E/S       220 birch log
// 157 water surface         189 fire                  221 lapis ore
// 158 fence N               190 stone button facing S 222 lapis block
// 159 fence S               191 stone button facing N 223 dispenser W



#endif // BLOCKIMAGES_H