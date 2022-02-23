# V-Patch for L.A Noire Complete Edition
*This project is not affiliated with Take-Two or Rockstar Games*

A while ago I wanted to replay L.A Noire on my new setup, only to find out that the game was locked to 30 FPS and didn't support my monitor's aspect ratio. With this fact in mind, I started to search for fixes, only to learn that most of them sucked. 

After determining that all the available fixes were either unintuitive to use for the average user or lacking in features, I sought out to make my own patch, mainly looking to tackle the 30 FPS cap and the aspect ratio limitations. Later in development it became clear to me that what I had made was worth publishing for others to use... so here we are.

## Features

### Total FPS Uncap (60+ FPS) & Variable Framerate

The biggest drawback of all available fixes for me was the fact that they would only raise "the FPS cap" to 60 frames per second. Not only that, playing with these fixes would sometimes make the game run slow if you happened to drop a few frames below 60 FPS.

Usually, raising "the cap" would be done by increasing a value in the game's memory, that would have the effect of increasing the frame limit. However, this method had a big disadvantage. Since the value used to cap the framerate was also used for game logic, in case your PC could not produce a constant 60 FPS, the game simulation would run slower.

This means if you set your frame cap to 60, but your PC could only produce 30 FPS, the game would run at half the right speed. And since the game can often drop below 60 FPS even on a fairly good setup, you would get occasional slowdowns. Not good! :/

Therefore, I looked for a new method of increasing the frame cap... or even better, removing it all together. I ended up patching out the frame synchronization logic, which had the effect of totally uncapping the framerate. Then, in order to make the game run at the correct speed, I hooked a small function which updates the game speed according to the current frame time. 

This means that you can now run the game at any framerate. It doesn't matter if you are running 60 FPS or 160, the game will always run at the correct speed.

Of course, the fix is not perfect, since some things in the game logic are still tied to the framerate. However, it's better than anything else available currently ;)

<table>
    <tr>
      <td><a href="https://www.youtube.com/watch?v=VFsFdUMLC8M"><img src="https://img.youtube.com/vi/VFsFdUMLC8M/0.jpg"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">Variable framerate showcase [YouTube]</div></td>
    </tr>
</table>

**Disclaimer: Some missions or activities may still be unbeatable when playing with a high framerate. If you encounter such an issue, disable the FPS unlock in the config file (lanvp.ini in the game folder), restart the game, pass the current section, re-enable the FPS unlock in the config and restart the game again. Please make sure to report the problem on the Steam forums, ModDB comments, or via GitHub issues so that we can work on a fix.**

#### Fixed car braking

Another drawback of using the 60 FPS patches was that cars would brake a lot slower. If you tried to go even higher than 60, cars would stop braking at all, effectively making the game unplayable.

This turned out to be a much easier fix that I had imagined, since it simply revolves around changing the braking modifier in the car dynamics logic, according to the framerate. Cars now brake at the correct speed, which also has the side effect of making the case **"The Golden Butterfly"** beatable again.

Here's a comparison of the braking speeds, 30 FPS vs uncapped:

<table>
    <tr>
      <td><a href="https://www.youtube.com/watch?v=L0e7pzqmdjk"><img src="https://img.youtube.com/vi/L0e7pzqmdjk/0.jpg"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">Car braking speed comparison [YouTube]</div></td>
    </tr>
</table>

### Ultrawide support

While I don't own an ultrawide monitor, it is clear to me that the game has some issues with ultrawide resolutions, due to the FOV being too small. And so, in order to remove the need for special widescreen tools, I decided to implement FOV correction fix right into the patch.

The principle through which it achieves the correction is quite simple. Since the game is made for 16:9 resolutions, the value determining the camera FOV stores the horizontal angle. Normally this works OK. However, on ultrawide resolutions your horizontal length is much longer than vertical length, meaning that the top and bottom of the screen would effectively get cut off.

The fix simply takes the current camera fov, calculates the vertical FOV by assuming the aspect ratio is 16:9, and then multiplies the resulting value by the current, real, aspect ratio. 

> fovHorizontal = (fovHorizontal * (9/16)) * aspect

<table>
    <tr>
      <td><a href="https://i.imgur.com/K3rgi4v.png"><img src="https://i.imgur.com/K3rgi4v.png"/></a></td>
      <td><a href="https://i.imgur.com/qEBNAb7.png"><img src="https://i.imgur.com/qEBNAb7.png"/></a></td>
    </tr>
    <tr>
      <td colspan="2"><div style="text-align: center; font-style: italic">Comparison of various aspect ratios with FOV correction.</div></td>
    </tr>
</table>

Cutcenes remain unaffected, since they are already ultrawide to begin with.

#### Custom FOV

In case you are still not happy with the FOV, the patch allows you to set a custom FOV multiplier in the configuration ini file. This can help if the game feels too zoomed in or out.

<table>
    <tr>
      <td><a href="https://i.imgur.com/rRrbAYq.png"><img src="https://i.imgur.com/rRrbAYq.png"/></a></td>
      </tr>
    <tr>
      <td><div style="text-align: center; font-style: italic">FOV Multiplier 2.0</div></td>
    </tr>
</table>

### Unlocked aspect ratios (16:10, 4:3, 5:4, etc.)

By default, if you try running the game on a resolution slimmer than 16:9, you will encounter black bars at the top and bottom of the screen. Since I don't have a 16:9 monitor, this made for an unpleasant playing experience.

Luckily, the fix was quite simple, since all that was needed was to patch out the aspect ratio check. Even without the FOV correction, the game looks alright on such resolutions, which makes me wonder why the limitation exists in the first place. 

<table>
    <tr>
      <td><a href="https://i.imgur.com/S9wtcaE.png"><img src="https://i.imgur.com/S9wtcaE.png"/></a></td>
      <td><a href="https://i.imgur.com/DtCNr4P.png"><img src="https://i.imgur.com/DtCNr4P.png"/></a></td>
    </tr>
    <tr>
      <td colspan="2"><div style="text-align: center; font-style: italic">L.A. Noire running in a 5:4 resolution without black bars and with FOV correction.</div></td>
    </tr>
</table>

### Borderless window

Recently, I discovered the convenience of running games in a borderless window mode, and so... I just couldn't resist adding support for it as well. This option is enabled by default, since it also disables v-sync. 

### Custom resolutions

A common issue with non-standard monitor sizes (for the year 2011 :) ) is that the desired resolution does not appear in the game settings. That's why the patch allows you to set a custom resolution in the config file. 

By default, this is now set to your current monitor's resolution, which prevents these sorts of issues in the first place.

## Windows Installation

Installing the patch is very simple:

1. Get the latest release from [GitHub](https://github.com/VaanaCZ/LANVP/releases)
2. Extract the archive.
3. Place dinput8.dll into the game folder (same location as LaNoire.exe)

Done. No special setup required. 

## Linux (Lutrs & Steam Proton) Installation:

Same as above except requires winecfg to override/accept the dinput8 dll. 

1. Copy the extracted dinput8.dll into your game's folder (same as above, where LaNoire.exe is located).
2. If you are using Lutris => (I assume you have L.A. Noire setup in Lutris already), click on the Wine icon -> Winetricks. Proceed with Step 5.
3. If you are using Steam  => Run `protontricks --gui` in a terminal (you may have to install protontricks from your distro's repo. Consult your package manager.
4. Select 'L.A. Noire 110800' and wait until a window pops-up (As long as the terminal says 'Executing mkdir' wait. It may show an error regarding 64-bit/32-Bit Prefix, but it'll take a while. Just click on 'Ok'. 
5. Select 'Run winecfg' (if you don't see that option, open 'Install an application' and cancel out. Now winecfg should be visible.
6. Select the 'Libraries' tab and add 'dinput8.dll' from the 'New override for library' dropdown menu.

If 'dinput8(native, builtin)' is visible in the list, then you've successfully installed the patch on Linux.

## Supported versions

V-Patch has been developed and tested on L.A Noire builds **2617** and **2663** (current version), including both Steam & R* Launcher releases.

## Configuration

When first launched, V-Patch creates a configuration file inside the L.A. Noire game folder. By default, it bears the name *lanvp.ini*. Here you can enable/disable or change the features mentioned above.

## Known issues

- Car steering is more sensitive than it should be.
- Certain graphical menu transitions (e.g. when switching outfits) fade out/in slower.
- When force_resolution is enabled, changing the resolution via the in-game settings breaks the GUI. It is recommended not to change the resolution in-game or disable force_resolution in the config before doing so.

## To-do list

- ASI loader to load other people's plugins.
- Ability to increase/decrease the LOD or render distance.
- Look into restoring cut features (Trolley&Bus riding) as optional settings.
- Fix problems with "The Quarter Moon Murders" and "The Set Up"
- Look into R\* Cam and possible HUD-less option.

## Changelog

### v1.1 (July 4th, 2021)
- Fixed mouse sensitivity in map menu.
- Improved GUI scaling for different aspect ratios.
- Fixed major issues with resolution enforcement.
- Added option to skip the intro logos when the game is launched.
- Added ability to set a custom FPS cap.
- Previously "unsupported" resolutions should now appear in the game settings.
- Added V-Patch version text into main menu.

### v1.1a (September 11th, 2021)
- Added option to enforce DirectX 11. Set to true by default if the user has a D3D11 capable GPU.
- Fixed bug where resolution wouldn't be enforced if the config was not yet generated.

## Credits

Thanks to the following people for taking the time to help with testing V-Patch:
- iJuhan
- PixellSnow
- Pastelblue
- Comot
- GaroK
- BazookaDad
