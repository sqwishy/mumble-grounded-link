Prototype support for Grounded in Mumble's positional audio. Tested on version
"1.2.6.4236 REL" using a copy of the game from "PC Game Pass" in the "XBox"
store thing from Microsoft on Windows -- not to be confused with their game
console by the same name.

Instead of a loadable plugin for Mumble, this is an executable that does
basically what a plugin would do; read from Grounded and send player
position and facing vectors to Mumble. Except it does this via the IPC provided
by the Link plugin in Mumble.

It shouldn't be difficult to make a proper plugin instead, but I won't have
access to this game after this week so I won't be getting around to it.


## Usage

1. Start Mumble.

   In Mumble's settings (as of 1.4), enable the "Link" plugin and check "Link
   to Game and Transmit Position" both under the "Plugins" page.

2. Start Grounded, start a game, pick a character, and load in.

3. Then start this program. It won't be able to read positions until after you
   load in to your duder in the game.

   If this program is spitting out lines of numbers like twenty times a second
   then it should be working.


## Limitations

- No context or identity support.

- This program uses the game's camera as your actor's position (and facing
  vectors). So if you switch from first to third person (which happens
  temporarily if you open your inventory), your voice will be projected from
  your camera's position.

  The correct vectors are probably in memory nearby, but the camera vectors
  weren't noticable or distracting enough to divert me from my unyielding ambition
  of finding toenail clippings and turning them into chandeliers in my free time.


## Compiling

I installed the C compiler and WinSDK from Visual Studio 2022 and open
"Developer PowerShell for VS 2022" or "Developer Command Prompt for VS 2022" so
that the compiler and stuff is in PATH.

In the build directory of a checkout of this repository, run `nmake` to build
using the Makefile provided. On success it compiles a `GroundedLink.exe` file
in that build directory.

## Licence

public domain [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/)
