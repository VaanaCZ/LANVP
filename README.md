# V-Patch for L.A Noire Complete Edition
*This project is not affiliated with Take-Two or Rockstar Games*

V-Patch is a lightweight, library-based, drop-in patch intended to fix common issues of the game L.A. Noire on PC. It also allows to the user customise certain technical aspects of the game, such as setting a custom FoV or disabling the company logos which show upon starting the game. 

## Features

### Total FPS Uncap (60+ FPS) & Variable Framerate

The main feature of V-Patch is the abillity to completely remove the default 30 FPS limit. Unlike other FPS patches which can only change the limit to 60, V-Patch allows the game to be run completely uncapped by implementing a better FPS cap removal method, which also doesnt suffer from side-effects such as the game simulation occasionally running at incorrect speeds.

In addition, V-Patch does not suffer from certain FPS-related bugs such as slow braking or the infamous pencil bug.

FPS-related fixes implemented by V-Patch:
- Fixes car braking force. (The Golden Butterfly)
- Fixed pencil. (The Set Up)
- Fixed pigeons.

<table>
    <tr>
      <td><a href="https://www.youtube.com/watch?v=VFsFdUMLC8M"><img src="https://img.youtube.com/vi/VFsFdUMLC8M/0.jpg"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">Variable framerate showcase [YouTube]</div></td>
    </tr>
</table>

<table>
    <tr>
      <td><a href="https://www.youtube.com/watch?v=L0e7pzqmdjk"><img src="https://img.youtube.com/vi/L0e7pzqmdjk/0.jpg"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">Car braking speed comparison [YouTube]</div></td>
    </tr>
</table>

**Disclaimer: Some missions or activities may still be unbeatable when playing with a high framerate. If you encounter such an issue, disable the FPS unlock in the config file (lanvp.ini in the game folder), restart the game, pass the current section, re-enable the FPS unlock in the config and restart the game again. Please make sure to report the problem on the Steam forums, ModDB comments, or via GitHub issues so that we can work on a fix.**

### Unlocked aspect ratios (16:10, 4:3, 5:4, etc.)

By default, the game has a limited set of allowed resolutions and any non 16:9 aspect ratios show up with black bars. V-Patch removes this limitation allowing the user to specify any resolution in the game's menu. V-Patch also removes black bars and implements UI scaling correction, which ensures the game is fully playable in any resolution.

<table>
    <tr>
      <td><a href="https://i.imgur.com/S9wtcaE.png"><img src="https://i.imgur.com/S9wtcaE.png"/></a></td>
      <td><a href="https://i.imgur.com/DtCNr4P.png"><img src="https://i.imgur.com/DtCNr4P.png"/></a></td>
    </tr>
    <tr>
      <td colspan="2"><div style="text-align: center; font-style: italic">L.A. Noire running in a 5:4 resolution without black bars and with FOV correction.</div></td>
    </tr>
</table>

**Note: In addition, setting any custom resolution using the "-res W H" command-line argument is now possible**

### Ultrawide support

V-Patch implements correct UI and FoV correction in order to allow the user to play the game in any ultrawide resolution. Unlike other with patches, the FoV correction does not break cutscenes, meaning the cinematic experience remains untouched. 

<table>
    <tr>
      <td><a href="https://i.imgur.com/K3rgi4v.png"><img src="https://i.imgur.com/K3rgi4v.png"/></a></td>
      <td><a href="https://i.imgur.com/qEBNAb7.png"><img src="https://i.imgur.com/qEBNAb7.png"/></a></td>
    </tr>
    <tr>
      <td colspan="2"><div style="text-align: center; font-style: italic">Comparison of various aspect ratios with FOV correction.</div></td>
    </tr>
</table>

### Other features

In addition to these fixes, V-Patch also lets you customize technical aspects of the game in the following manner:

- **Borderless window** - The patch allows the user to start the game in borderless.
- **Skipping company logos** - For a faster startup, the user can disable the logos and legal information which shows up after starting the game.
- **Custom FoV** - Via an option in the config, the user can set a custom FoV multiplier.
- **Custom FPS limit** - If needs be, the patch allows the user to set a custom FPS cap.

<table>
    <tr>
      <td><a href="https://i.imgur.com/rRrbAYq.png"><img src="https://i.imgur.com/rRrbAYq.png"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">FOV Multiplier 2.0</div></td>
    </tr>
</table>

## Windows Installation

Installing the patch is very simple:

1. Get the latest release from [GitHub](https://github.com/VaanaCZ/LANVP/releases)
2. Extract the archive.
3. Copy the contents into the game's folder (same location as LaNoire.exe). Do not forget to extract the plugins folder!

Done. No special setup required.

## Linux (Lutris & Steam Proton) Installation

todo: Ultimate-ASI lader

Same as above except requires winecfg to override/accept the dinput8 dll. 

1. Copy the extracted dinput8.dll into your game's folder (same as above, where LaNoire.exe is located).
2. If you are using Lutris => (I assume you have L.A. Noire setup in Lutris already), click on the Wine icon -> Winetricks. Proceed with Step 5.
3. If you are using Steam  => Run `protontricks --gui` in a terminal (you may have to install protontricks from your distro's repo. Consult your package manager.
4. Select 'L.A. Noire 110800' and wait until a window pops-up (As long as the terminal says 'Executing mkdir' wait. It may show an error regarding 64-bit/32-Bit Prefix, but it'll take a while. Just click on 'Ok'. 
5. Select 'Run winecfg' (if you don't see that option, open 'Install an application' and cancel out. Now winecfg should be visible.
6. Select the 'Libraries' tab and add 'dinput8.dll' from the 'New override for library' dropdown menu.

If 'dinput8(native, builtin)' is visible in the list, then you've successfully installed the patch on Linux.

## Supported versions

V-Patch works on any legal version of L.A. Noire (both Steam & R* Launcher) including builds **2617**, **2663** and **2675** (current version).

## Configuration

In order to customize the settings of V-Patch, navigate to the game's directory and find the plugins folder. Here you can find a by the name of *lanvp.ini* in which you can enable/disable or change the features mentioned above.

## Known issues

- Car steering is more sensitive than it should be.
- On very high FPS (100+), the chandelier in the mission "The Quarter Moon Murders" can break. If you encounter this issue, temporarily disable the patch or lower your FPS limit to below 100.

## Compilation

Use MSVC 2022 or newer.

## Changelog

### v1.1b (June 30th, 2022)
- Added support for new L.A Noire version 2675.
- Resolved an issue where dinput8.dll couldn't be found on 32-bit systems.
- More verbose Windows API error messages.

### v1.1a (September 11th, 2021)
- Added option to enforce DirectX 11. Set to true by default if the user has a D3D11 capable GPU.
- Fixed bug where resolution wouldn't be enforced if the config was not yet generated.

### v1.1 (July 4th, 2021)
- Fixed mouse sensitivity in map menu.
- Improved GUI scaling for different aspect ratios.
- Fixed major issues with resolution enforcement.
- Added option to skip the intro logos when the game is launched.
- Added ability to set a custom FPS cap.
- Previously "unsupported" resolutions should now appear in the game settings.
- Added V-Patch version text into main menu.

## Credits

Thanks to ThirteenAG for the Ultimate ASI Loader.

Thanks to the following people for taking the time to help with testing V-Patch:
- iJuhan
- PixellSnow
- Pastelblue
- Comot
- GaroK
- BazookaDad
