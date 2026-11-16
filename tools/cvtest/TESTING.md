# Testing

- `python -I tools\cvtest\suite.py` is the regression suite: every feature of the port by its tests, listed in
  [SUITE.md](SUITE.md) with what each checks and what only a person can check, run one
  after another on this platform: the scenarios below on the dev build, then the release packages in `out\release`
  as a player installs them (`install.py` with each disc, `launcher_test.py`, `discs.py`). Scenarios that come after
  the same new game play in sessions, one game each (`cvtest.py --session`): the new game once, then each scenario
  from the courtyard where it starts Hector (warped back, healed, menus closed, the settings the one before changed
  put back), each still a test with its own report; `--alone` plays every scenario in a game of its own. `--linux`
  (on Windows) then runs it on Linux in WSL; `--quick` runs a shorter list, `--only` and `--from` some of it. A
  test that needs what the PC lacks (a disc, the HD pack, a package) is skipped, and a run with one skipped is not a
  pass. This PC's discs and WSL setup are in `out\test\suite.json` (its format at the top of `suite.py`). A full run
  that passed on a commit without changes records it (`out\test\suite-<platform>.json`, with the packages'
  SHA-256), which `--check` reads: a release needs both platforms' for the commit released. A feature added gets a
  test there.
- `python -I tools\cvtest\cvtest.py tools\cvtest\scenarios\newgame.json` plays a scenario in a test folder of its own
  (`out\test\<scenario>-<time>`, its own settings and saves) through `CV_PAD_LIVE`, checks the game state after
  every step (`src\common\telemetry.cpp`: map and room, Hector's position, HP, the camera, menus), the log and
  the screen, and writes `report.md` with a screenshot per step. Scenario format: the top of `cvtest.py`.
  `bot.json` has the test bot (`src\common\bot.cpp`) walk Hector to points and fight the knights of room 5.12.
  `--release eu` plays it on the European release (`CastlevaniaEU.exe`, its disc image in `assets`): every scenario
  here passes on both, and `--set game_language=5` (3 German, 4 French, 6 Italian) plays it in another language.
- `python -I tools\cvtest\cvtest.py --record <route> [--save <name>]` starts the game for play (the play folder's
  settings, keyboard and controllers) and records it; once the game is closed the recording becomes
  `tools\cvtest\routes\<route>.json`, a leg per room with the points walked, the jumps and B presses (doors,
  save points), the menus and message boxes as they were answered, and the fights that were won. A scenario step
  `{"route": "<route>"}` plays it back with the bot: `route.json`, and `first-save.json` (a new game to the first
  save room, a save there, the pause menu and the map). `keep-save.json` keeps that save, and `load-save.json`
  continues from it: one release's save loads in the other (both releases' saves are in `hdd\E\UDATA\4b4e002d`, the
  same title), which they do both ways.
- `--judge` has every screenshot judged by GPT-6 Luna through the Codex CLI (`codex exec`, on its sign-in;
  `tools\cvtest\judge.py`): what the screen shows, whether it rendered right, whether Hector is in it, about 4 s a
  picture. A likely defect, or a scene the game state disagrees with, is a warning in the report.
  `--judge=jev-latest` is quicker for long runs such as sweeps: Jev (TypeSafe, the key in `TYPESAFE_API_KEY` or a
  `.env` file named by `TYPESAFE_ENV_FILE`) screens every shot in about 0.35 s and Luna looks again at those it
  flags (`--review=none` for Jev alone). `tools\cvtest\bakeoff.py` builds the labelled set (shots from test runs,
  rendering defects planted in some) that Jev, GPT-6 Luna and Haiku 5.5 were compared on.
- `perf.json` stands 5 s in the heaviest rooms with the SDK's timings (`XBE_PERF`) and ends its report with them:
  the frame rate, the renderer's milliseconds a frame, the game thread's in D3D calls and waiting on the renderer,
  the draws. A scenario's `"perf": true` times any run; `"shader_cache": "<name>"` keeps its compiled shaders for
  the next run with that name (a test starts cold otherwise). `saveroom.json` stands ten seconds in the Abandoned
  Castle's save room, the costliest room the ray tracing has met (its hundreds of glow sprites), and `saveflick.json`
  shoots it six times two frames apart for its flicker; `flesh.json` takes the Garibaldi Temple's flesh room and
  two beside it, which the bounce rays find no light in; `f10room.json` and `qualityswap.json` open the settings
  menu while playing (the latter changes the ray tracing quality in it) and go on to the next rooms. The save room
  in motion: `savemove.json` (Hector walks each way), `savepan.json` (the camera panned left and right on the right
  stick, eight shots two frames apart each way), `saveslow.json` (panned at a sixth of the stick) and `saveslow1.json`
  (the same, the shots one frame apart); `cmp\noise\savecells.py`, `savesame.py` (two runs of the same path at the
  same frames: their difference is the lighting's randomness) and `saveboil.py` (the motion taken out block by block)
  read them.
- A step's `"ask"` puts a question about its screenshot to GPT-6 Luna, with named answers an `expect` can check.
  `buttons.json` asks which controller's icons show at the courtyard door and on the controller settings page;
  `--set button_layout=1` runs it with the PS2 layout (`--set` changes any setting for a run).
- `CV_TEXPACK_MISSES=1` logs each texture the HD pack has no picture for, once, as a pack would name it; set to a
  folder instead, it also saves each there as a PNG under that name, with the PS2's alpha as PCSX2 dumps it, so an
  upscaled copy in `textures` replaces it. `CV_TEXPACK_CENSUS=1` logs every texture the game uploads, once, with
  its format, size and what the pack made of it (a picture, none, or not looked for: the pack has P8 pictures only).
  `CV_TEXTURES=<folder>;<folder>` reads pictures from those folders instead of `textures`, a picture in an earlier
  one in place of one with the same name in a later one: pictures tried out beside a pack without changing it.
- `CV_MODS=<folder>` looks for the translations in that folder instead of `mods` beside the game (the suite's
  `translation`, `translation-nodub` and `translation-eu` play the Brazilian Portuguese pack from the folder
  `out\test\suite.json` names as `translation`). The game logs the pack's files as each is first read in place of
  the disc's, and where its texts went.
- `tools\texpack\upscale.py make` makes a picture for each texture `CV_TEXPACK_MISSES` saved: upscaled 4x, the
  pack's scale, by an AI model (RealESRGAN_x4plus; Python with PyTorch, its docstring has the setup). Where the pack
  has the PS2's picture of a texture the Xbox changed in part, as for the save screens' HARD DISK and the HUD's
  icons, it keeps that picture and takes from the upscale only what the Xbox changed: the parts that differ
  throughout from the pack's picture shrunk (not only along edges, as wherever the pack's art was redrawn), in most of
  the texture's colourings; and where in the result a strong colour lies over a texel not of it, the texture's own
  colour, as the pack's upscale of a sheet of icons runs each into its neighbours: the line the HUD draws under
  "Player" took the pure green of the letters above it in the colouring it is drawn with (found by pictures of one
  colour each, in `CV_TEXTURES`, telling which colouring each part of the HUD is drawn with: the frame and that line
  223ff5, "Player" 2c928c, the HP bar 66d55d). Cut-out shapes and text are upscaled with their colour weighted by
  alpha, so the key colour under their transparent texels (often a green) does not run into their edges; soft
  effects take half of the model's colour and half of a smooth resize, the model giving smoke a texture it does not
  have. `CV_UPSCALE_CPU=1` keeps the model off the GPU, as while a game has it. `upscale.py calibrate` measures
  models against the pack's own pictures of the same textures (`kAliases`' and those): of
  RealESRGAN_x4plus, UltraSharpV2, GTAV textures (RGT, and its dither model) and GameAI 2.0, RealESRGAN_x4plus came
  closest by far (31.2 dB on average, 45 dB on one, the others 26.7 to 28.7), the pack likely made with it, so its
  pictures look like the pack's. They are the game's art, as a pack is: they stay out of this repository. The
  menus' text is pictures too (the words, digits and panels of the pause menu's pages, the title screen's menu, the
  loading and save screens: 71 sheets the pack never covered, seen with `CV_TEXPACK_MISSES` on cvtest's
  `pausemenu`), so the same run makes them sharp, with the pack's own redrawn pieces placed in where it has them;
  `tools\texpack\clean_menu_bars.py` then redraws the pause menu's button bars' ends, painted ragged in the game's
  sheets, as a smooth fade (its docstring says which two sheets draw a bar, and how).
- `widescreen.json` keeps the screens widescreen reaches the sides of: the pause menu, the map, and with a route
  step's `"box_shots": true` each box on the first-save recording (the save point's, and the Memorial Ticket's
  dimmed panel when the first enemy drops it).
- `filters.json` checks that render scale and anisotropic filtering change the picture, with the HD pack and without
  (`--set texture_pack=false`). Hector stands on the drawbridge while each is changed live, as the in-game menu
  changes it (the live entry `set=<setting>=<value>`; one read only at the start is an error there), and a step's
  `"measure"` takes the edge strength of parts of the view (`screen.edges.<name>`; `shots["<shot>"]` has an earlier
  step's screen). With the pack 50% halves it, 200% adds two fifths and filtering a half or more; without it 50%
  takes nearly half, while 200% and filtering take a tenth to a third away, the shimmer of textures without mip
  levels smoothed. Two shots with the same settings are the same.
- `moai.json` walks a recording (`routes/moai.json`) to the far end of 5.0 and asks whether the Moai floats there and
  what the pickup's notice says: the Moai with `--set lament_save=true`, nothing without it.
- `trevor.json` and `crazy.json` type `@TREVOR` and `@CRAZY` on the name screen of a game never beaten and play into
  the first courtyard: the game's mode (`mode`, 2 and 1, with the save's `name`) and Trevor's hearts and sub-weapons
  in the shots. `newgame.json` checks that a plain name gives mode 0.
- `lockon.json` locks on (Black, the game's Lock-On ON/OFF) to the Executioner of room 5.2 with him off the middle of
  a widescreen: its marker, drawn from the HUD's sheet at him, stays where the game puts it while the HUD goes to the
  screen's corners. The telemetry's `wide` says how far widescreen moved the marker and the HUD as last drawn. With
  `--release eu --set game_language=5` it plays in Spanish, whose HUD sheet ("PS" for HP) is a picture of its own.
- `lock-twin.json` plays one controller seen twice in the PS2 layout (the SDK's virtual controller and its twin,
  `XBE_VIRTUAL_PAD=2`) against the two knights of room 5.12: locked on, R1 and L1 keep the enemy locked on (the
  telemetry's `lock`, the actor), L2 changes it and R2 lets it go.
- `python -I tools\cvtest\launcher_test.py <release .zip or .tar.xz, or a build folder> <USA disc> <European disc>`
  installs a release as `install.py` does, with one disc and then both, and drives the launcher by window messages (on
  Linux, on a virtual display) where it says its controls are (`CV_LAUNCHER_STATE`): with one disc no list beside
  **Play**; with both the list, the European release chosen and saved, **Play** starting `CastlevaniaEU`, which
  finds its disc; the home page in Russian; and each executable started by itself following `release`, or playing
  its own release without one. A game started is ended once its log says which disc it plays.
- `python -I tools\cvtest\install.py out\release\CastlevaniaCOD-PC-1.0.0.zip <your .iso, or an extracted disc>`
  installs a release as a player would and plays it: the zip extracted into a new folder in TEMP (outside the
  repository, where the game could find this PC's `assets` or `textures`), the game linked into its `assets`, every
  program and DLL in it checked to load from the package or Windows alone (the Visual C++ runtime does not count, a
  fresh Windows has none), then `Castlevania.exe` started in an environment of Windows' own variables only, its
  first start building the game code, and `newgame.json` (or the scenario given) played there. The report is
  `out\test\install-<scenario>-<time>`, the checks and the first start before the steps.
- `tools\run_test.ps1 -Name x -At 10,20 -Env "CV_PAD_SCRIPT=@tools\scripts\newgame.txt"` runs the game, screenshots
  it at the given seconds into `tools\shots`, and closes it. It copies the screen, so keep the game window in front;
  give `CV_PAD_SCRIPT` a full path, the game runs from `out\build\release`.
- `CV_PAD_SCRIPT` presses buttons on a timeline (see `src\common\keyboard.cpp`).
- `XBE_VERBOSE=1` logs info lines to `logs\castlevania.log`; with `XBE_CAPTURE_DIR` set, F3 saves a frame with its
  draw list (each draw's state, its textures and shaders), as does the live entry `0:capture` at a step's start.
