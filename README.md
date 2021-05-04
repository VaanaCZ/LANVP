# V-Patch for L.A Noire Complete Edition

A while ago I wanted to replay a L.A Noire on my new setup, only to find out that the game was locked to 30 FPS and didn't support my monitor's aspect ratio (16:10). With this fact in mind, I started to search for fixes, only to learn that most of them sucked. 

After determening that all of the available fixes were either unituitive to use for the average user or lacking in features, I sought out to make my own patch, mainly looking to tackle the 30 FPS cap and the aspect ratio limitations. Later in development it became clear to me that what I had made was worth publishing for others to use... so here we are.

### Total FPS Uncap (60+ FPS) & Variable Framerate

The biggest drawback of all available fixes for me was the fact that they would only raise "the FPS cap" to 60 frames per second. Not only that, playing with these fixes would sometimes make the game run slow if you happened to drop a few frames bellow 60 FPS.

Usually, raising "the cap" would be done by increasing a value in the game's memory, that would have the effect of increasing the frame limit. However, this method had a big disadvantage. Since the value used to cap the framerate was also used for game logic, in case your PC could not produce a constant 60 FPS, the game simulation would run slower.

This means if you set your frame cap to 60, but your PC could only produce 30 FPS, the game would run at half the right speed. And since the game can often drop bellow 60 FPS even on a fairly good setup, you would get occasional slowdowns. Not good! :/

Therefore, I looked for a new method of increasing the frame cap... or even better, removing it all together. I ended up patching out the frame synchronization logic, which had the effect of totally uncapping the framerate. Then, in order to make the game run at the correct speed, I hooked a small function which updates the game speed according to the current frame time. 

This means that you can now run the game at any framerate. It doesn't matter if you are running 60 FPS or 160, the game will always run at the correct speed.

Of course, the fix is not perfect, since some things in the game logic are still tied to the framerate. However, it's better than anything else available currently ;)

[![Variable framerate showcase](http://img.youtube.com/vi/VFsFdUMLC8M/0.jpg)](http://www.youtube.com/watch?v=VFsFdUMLC8M "L.A. Noire - Total FPS Unlock, Variable framerate test")

#### Fixed car braking

Another drawback of using the 60 FPS patches was that cars would break a lot slower. If you tried to go even higher than 60, cars would stop breaking at all, effectively making the game unplayable.

This turned out to be a much easier fix that I had imagined, since it simply revolves around changing the braking modifier in the car dynamics logic, according to the framerate. Cars now break at the correct speed, which also has the side-effect of making the case "The Golden Butterfly" beat-able again.

Here's a comparison of the braking speeds, 30 FPS vs uncapped:

[![](http://img.youtube.com/vi/L0e7pzqmdjk/0.jpg)](http://www.youtube.com/watch?v=L0e7pzqmdjk "L.A. Noire - Total FPS Unlock, Car braking test")

### Ultrawide support

While I don't own an ultrawide monitor, it is clear to me that the game has some issues with ultrawide resolutions, due to the FOV being too small. And so, in order to remove the need for special widescreen tools, I decided to implement FOV correction fix right into the patch.

The principle through which it achieves the correction is quite simple. Since the game is made for 16:9 resolutions, the value determening the camera FOV stores the horizontal angle. Normally this works OK. However, on ultrawide resolutions your horizontal length is much longer than vertical length, meaning that the top and bottom of the screen would effectively get cut off.

The fix simply takes the current camera fov, calculates the vertical FOV by assuming the aspect ratio is 16:9, and then multiplies the resulting value by the current, real, aspect ratio. 

> fovHorizontal = (fovHorizontal * (9/16)) * aspect

[![](https://i.imgur.com/K3rgi4v.png)](https://i.imgur.com/K3rgi4v.png)
[![](https://i.imgur.com/qEBNAb7.png)](https://i.imgur.com/qEBNAb7.png)

Cutcenes remain unaffected, since they are already ultrawide to begin with.

#### Custom FOV

In case you are still not happy with the FOV, the patch allows you to set a custom FOV multiplier in the configuration ini file. This can help if the game feels too zoomed in or out.

[![](https://i.imgur.com/rRrbAYq.png)](https://i.imgur.com/rRrbAYq.png)

### Unlocked aspect ratios (16:10, 4:3, 5:4, etc.)

By default, if you try running the game on a resolution slimmer than 16:9, you will encounter black bars at the top and bottom of the screen. Since I don't have a 16:9 monitor, this made for an unpleasant playing experience.

Luckily, the fix was quite simple, since all that was needed was to patch out the aspect ratio check. Even without the FOV correction, the game looks alright on such resolutions, which makes me wonder why the limitation exists in the first place. 

[![](https://i.imgur.com/S9wtcaE.png)](https://i.imgur.com/S9wtcaE.png)
[![](https://i.imgur.com/DtCNr4P.png)](https://i.imgur.com/DtCNr4P.png)

### Borderless window

Recently, I discovered the convenience of running games in a borderless window mode, and so... I just couldn't resist adding support for it as well. This option is enabled by default, since it also disables v-sync. 

### Custom resolutions

A common issue with non-standard monitor sizes (for the year 2011 :) ) is that the desired resolution does not appear in the game settings. That's why the patch allows you to set a custom resolution in the config file. 

By default, this is now set to your current monitor's resolution, which prevents these sorts of issues in the first place.

## Installation

Installing the patch is very simple:

1. Grab the latest release of GitHub: 
2. Extract the archive.
3. Place dinput8.dll inside the game folder (same location as LaNoire.exe)

Done. Everything is contained within a single 137 kB dll file. No special setup required. 

**This needs to be completed**
