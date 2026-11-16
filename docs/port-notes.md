# Port Notes

Changes this game needed beyond the generated config:

- `config\game.toml`, by hand: `D3DDevice_SetPalette`, `D3DDevice_DrawIndexedVerticesUP` and four DirectSound buffer
  methods (`SetPitch`, `PlayEx`, `SetDopplerFactor`, `SetRolloffFactor`) that no 5849 signature matched; the
  DirectSound ones are named from the image's `.XTLID` table (the SDK now finds `Pause` itself).
- `sdk-patches\xboxxbe-recomp.patch`: host versions of `DrawIndexedVerticesUP` and the four DirectSound methods; a
  fixed-function stage with no texture that reads `D3DTA_TEXTURE` ends the stage cascade, as XDK D3D does (without it
  every single texture quad drew white); `XBE_VERBOSE` lets a release build log info lines; a `widescreen` setting;
  for renderer extensions, `ShaderHooks::Position` (HLSL after the vertex position is set), `Extension::Vertices`
  (each draw's assembled vertices) and `gpu::ReadsConstant`; an extension's replacement is drawn before the SDK's own
  texture pack image; anisotropic filtering forced on textures without mip levels too (it was only forced where a
  texture had them, which no texture of this game does); multisampled draws (2X, 4X) on the hardware's smaller
  grid, which puts Hector's 4X stencil shadow volume back in place (found by Neguchads); another region's release in
  an executable of its own, which the others start for its game files (`GameInfo::releases`, `xbe_releases()`); the
  console language a title reads (`GameInfo::language`); the settings file written over, its other keys kept; a
  target telling whether its picture is the CPU's (`Target::cpu`). The SDK's mixer plays every channel of a voice by
  itself now (cutscene audio is a 5.1 stream with the voices in the centre channel). `kernel::g_file_opened`, a
  game's hook on each file the title opens; a replacement uploaded into the texture of the one before it when it has
  its size. Ray tracing (`runtime\gpu\raytrace.h`, `rt_*.hlsl`): the world's draws captured by their vertex shaders
  into an acceleration structure each frame, ambient occlusion, bounce lighting and reflections traced through it,
  and a material's `gpu::Material::gloss` for reflections. `XBE_MUTE` keeps a test's game silent;
  `XBE_D3D12_DRED` logs where the GPU stopped when the device is lost, `XBE_D3D12_DEBUG` the debug layer's errors.
- Releases are the SDK's own (`release.bat`): the game code is `Castlevania.dll` (`CastlevaniaEU.dll` for the
  European release), which the exe builds on the player's PC on its first start (the SDK's
  `runtime\app\game_code.cpp`, behind a progress window) from their own `default.xbe`, with the config whose `library`
  is that DLL. A DLL whose stamp does not match the exe and xbe is built again.
- Two releases: the European `default.xbe` is the same game built later for five languages, its code and data a
  little shifted (9 KB more code; data 0x2C40 to 0x7980 bytes further on). It is a second image of its own
  (`add_xbe_image(CastlevaniaEU eu/default.xbe game_eu.toml ...)`, its config written by `xbe_symbols`): the same
  sources compiled with `CV_EU`, every address they use taken from `src\common\release.h`, `src\game_eu` the hooks
  (each includes its `src\game` file). The European addresses came from matching the two xbes
  (`tools\release_map.py`): functions by their bytes with addresses and call offsets masked (15,454 of 20,373
  matched), data by what the instructions of matched functions name, and the few reached only through pointers
  (Hector's attributes and gold, the camera's view) checked in play and against the USA release's memory at the same
  moment. The European game resets the device for its present interval (`D3DDevice_Reset`, which the USA release never
  calls and the 5849 signatures do not find), named in its config by hand. The SDK's `GameInfo::releases`
  (`xbe_releases()` in CMake) lists each release's xbe SHA-1, certificate and executable: an executable whose game
  files hold another release's xbe starts that release's executable with its arguments, waits for it and ends with
  its exit code (its log carries on, `--handed-over`). `GameInfo::language` is the console language the kernel gives
  the game: the European release's `game_language`, English for the USA one, whose disc has only placeholders for the
  other languages' files. The SDK's settings file is written over rather than replaced, so a key the game does not
  know (the launcher's own) is kept.
- The game's copy: the SDK patch has the exe take a copy whose certificate was patched (title name, allowed media,
  region, ratings) once those are put back as in the build's own xbe, which CMake reads them from
  (`kImageCertificate`, beside `kImageSha1`); the game then plays the restored xbe. A disc image may also be a whole
  disc dump of either layout (the game partition at `0x18300000` on XGD1 discs, `0xFD90000` on XGD2). The launcher
  checks the same way, the xbe inside each disc image included, with the SDK's disc reader.
- The graphics card: the SDK patch adds a `gpu` setting, a card's name as Windows has it
  (`gpu = "Intel(R) Iris(R) Xe Graphics"`). The game draws with that one if it has Direct3D 12, else, as with none
  named, with the fastest (`EnumAdapterByGpuPreference`), and the log says which. The launcher lists the cards that
  can, the fastest first.
- `src\common\menu.cpp`: the menu key's state is read every frame instead of waiting for its key messages, which an
  overlay or another program's hotkey can take first; the messages are still swallowed so F10 never puts the window
  in menu mode.
- `src\launcher`: the launcher, Direct3D 11 and Dear ImGui, no game code; it checks `default.xbe`, and the one on
  each disc image, against the SHA-1 the build wrote for `Castlevania.exe`.
- `src\common\texpack.cpp`: PCSX2 texture packs. A P8 texture's indices are put back into GS memory order, 16x16
  (8 bit) or 32x16 (4 bit, indices under 16) blocks in the GS's column layout, and hashed with XXH3-64 block by
  block as PCSX2 does; its palette is hashed as the GS holds it (R G B A, alpha halved). The pair names the picture.
  PNGs load on worker threads through WIC with their PS2 alpha (128 opaque) doubled, halved to the size set and
  each level below made with their colour weighted by alpha (the colour under a PS2 picture's transparent pixels
  is often a key colour, a green, which an even mean runs into the edges of what is drawn). The SDK patch adds
  `Extension::Replace` and `Replaced`, `gpu::PaletteIndices`, and the backend draws a replacement with every level.
  Eight Xbox textures are found under the PS2's names (`kAliases`): on the PS2 each used part of its texture, the
  rest holding whatever else was in GS memory, which PCSX2 hashed with it, where the Xbox has that part blank. Their
  used parts are the same pictures, found by palette and by content against the pack, and each was looked at.
- `src\game\hooks.cpp`: widescreen culling. The camera (`sub_00012CD0`, a PS2 libvu0 style camera) makes its
  view screen matrices and the clip data objects are culled against; with widescreen on, the clip data's x scale
  (`0x012FC500`, `0x012FC5C0`, `0x012FC640`, `0x012FC660`) is made 3/4, so objects in the wider view are drawn
  instead of popping in at the edges. The matrices the scene is drawn with are untouched.
- `src\common\widescreen.cpp`, a renderer extension:
  - Widescreen. The game projects with a PS2 style view screen matrix (`c68`, or `c85` with the world matrix) and
    maps PS2 screen space to console pixels with `c94`/`c95`. Each draw into the back buffer is squeezed to 3/4 of its
    width around the centre: for draws that use the camera that is a wider field of view, for 2D draws a 4:3 shape.
    Draws over the whole width stay whole when they are untextured (fades), take a few texels across it (a message's
    dimmed panel) or read a render target the GPU drew (the frame pass); one whose picture the CPU wrote is a picture
    (the European release's intro decodes every other frame of its movie into a target of the game's, which once
    kept it whole every other frame). Menus, the map and the save screens draw their backdrop
    in pieces; the pieces at the frame's edges have those edges pulled out to the screen's (a vertex on the edge is
    not squeezed), unless the 3D world (1000 vertices of it) or a 4:3 picture over the whole frame came first: the
    title's logo is in pieces at the edges, over its castle. `CV_WIDE_TRACE=<n>` logs every draw into the back buffer
    of one frame in n, where it lands and what was made of it. Only draws into the device's back buffer are squeezed:
    the game composes an area's name card in another 640x480 target and copies it out to a texture, and squeezed
    there the card lost its left end.
    3D the game draws into such a target and then lays over the whole frame (room 5.19's breakable pillar, lit
    green; an enemy's after-images) is the exception: once a target's picture has been laid over the frame, its
    3D there is squeezed too and the picture laid over whole, or what lies past the 4:3 edge is not in it (the
    pillar was cut there as the camera turned; telemetry `wide.apart`).
  - Overscan. The last pass of each frame draws a copy of the frame back with its rows -16 to 496 over rows 0 to 480
    (PS2 448 lines plus border shrunk into the NTSC frame); the European release's rows -16 to 490 and columns -20 to
    660 over the 640, its picture narrowed by 640/680. What reads past the copy clamps into streaks at the edges, and
    the copy's first row and column are left out by the game's 2D draws, a pixel right and down on the PS2's grid
    (a fade misses them: a light line at the top). The pass is drawn where its texels are, the copy's pixel a pixel up
    and left on the frame, so the copy fills the frame pixel for pixel. The European intro's movie is drawn the same
    way; squeezed to 4:3, the columns it reads past its texture are left out (`ShaderHooks::Output`).
