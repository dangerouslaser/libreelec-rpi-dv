# LibreELEC for the Raspberry Pi 5 with Dolby Vision

This is LibreELEC.tv master (`a0fa50bc6e`, LibreELEC 13) with standard
(TV-led) Dolby Vision output on the Raspberry Pi 5. Kodi plays Profile 5,
7 (FEL and MEL, single-track) and 8 streams as Dolby Vision: a Vulkan engine
on the Pi's GPU reconstructs each picture from the hardware decoder's
frames and the RPU, and sends it with its metadata in the HDMI tunnel
frame. The TV does the display mapping.

What this tree adds to LibreELEC.tv:

| Change | Where |
|---|---|
| Vulkan for the RPi5 (Mesa's v3dv, the loader, `vulkaninfo`) | `projects/RPi/devices/RPi5/options` (`VULKAN="vulkan-loader"`) |
| The kernel's `Dolby Vision` HDMI connector property, attached by `vc4` | `projects/RPi/patches/linux/0001–0005` |
| `pi-dv-engine`: the engine, `libpi_engine.so.1` | `packages/multimedia/pi-dv-engine/` |
| An FFmpeg fix: the V4L2 request HEVC decoder no longer hangs on close after running out of CMA | `packages/multimedia/ffmpeg/patches/rpi/0002-…` |
| Kodi's build switch (`DOLBY_VISION_SUPPORT`, `-DENABLE_PIDV`) | `packages/mediacenter/kodi/package.mk` |
| Kodi: Dolby Vision from the EDID, a fenced video plane, the connector state, the engine's renderer, the profile 7 enhancement layer decoder, and a Dolby Vision group in the video settings | `projects/RPi/devices/RPi5/patches/kodi/0001–0006` |

**Build:** as LibreELEC, `PROJECT=RPi DEVICE=RPi5 ARCH=aarch64 make image`.
The engine's source is downloaded from this repository's release
(`pi-dv-engine-<version>.tar.xz`).

**Use:** flash the image, or copy its `.tar` to `/storage/.update/` and
reboot. The display mode must be the video's size: turn on "Adjust display
refresh rate" with a 4K desktop for 4K streams. Dolby Vision must be among
the player's "Allowed HDR dynamic metadata formats". During playback the
video settings' "Dolby Vision" group switches the output and its options
(the enhancement layer upscale, chroma MMR and filters at the Expert
settings level).

**Limitations:** the GUI over Dolby Vision video is legible but in wrong
colours; dual-track profile 7 isn't read; the sharper enhancement layer
upscales (bicubic, Lanczos3, Annex B) aren't real time for 4K FEL on the
Pi 5. Tested on an LG B4.

---

# LibreELEC

LibreELEC is a 'Just enough OS' Linux distribution for the award-winning [Kodi](https://kodi.tv) software on popular mediacentre hardware. Further information on the project can be found on the [LibreELEC website](https://libreelec.tv).

**Documentation**

- [CONTRIBUTING.md](CONTRIBUTING.md) — how to report issues and submit pull requests
- [STANDARDS.md](STANDARDS.md) — coding standards for build scripts and package files
- [packages/README.md](packages/README.md) — detailed guide to `package.mk` structure and variables

**Issues & Support**

Please ask questions in the [LibreELEC forum: Help & Support](https://forum.libreelec.tv/forum-3.html) or ask a member of project staff in the #libreelec IRC channel on Libera.Chat. Please report bugs via [GitHub Issues](https://github.com/LibreELEC/LibreELEC.tv/issues).

**Donations**

Contributions towards current project funding goals can be made via [OpenCollective](https://opencollective.com/libreelec/donate).

**License**

LibreELEC original code is released under [GPLv2](https://www.gnu.org/licenses/gpl-2.0.html).

**Copyright**

As LibreELEC includes code from many upstream projects it has many copyright owners; notably [OpenELEC](https://openelec.tv) which we forked from after disagreeing with project direction and management, and [OpenBricks/GeeXboX](https://github.com/OpenBricks/openbricks/blob/master/AUTHORS) the uncredited source of the original 2009 build system. LibreELEC makes no claim of copyright on any upstream code. However all original LibreELEC authored code is copyright LibreELEC.tv. Patches to upstream code have the same license as the upstream project unless specified otherwise. For a complete copyright list please checkout the source code to examine license headers. Unless expressly stated otherwise all code submitted to the LibreELEC project (in any form) is licensed under [GPLv2](https://www.gnu.org/licenses/gpl-2.0.html) and copyright is donated to the project. This approach gives the project freedom to maintain the code without the overhead of preserving contact with every submitter, e.g. GPLv3. You are free to retain copyright by adding your copyright header to each submitted code page. If you submit code that is not your own work it is your responsibility to place a header stating the copyright.
