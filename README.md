# LibreELEC for the Raspberry Pi 5 with Dolby Vision

This is LibreELEC.tv master (LibreELEC 13) with standard (TV-led) Dolby
Vision output and stereoscopic 3D playback on the Raspberry Pi 5. Kodi plays Profile 5,
7 (FEL and MEL, single-track) and 8 streams as Dolby Vision: a Vulkan engine
on the Pi's GPU reconstructs each picture from the hardware decoder's
frames and the RPU, and sends it with its metadata in the HDMI tunnel
frame. The TV does the display mapping.

What this tree adds to LibreELEC.tv:

| Change | Where |
|---|---|
| Vulkan for the RPi5 (Mesa's v3dv, the loader, `vulkaninfo`) | `projects/RPi/devices/RPi5/options` (`VULKAN="vulkan-loader"`) |
| The kernel's `Dolby Vision` HDMI connector property, attached by `vc4` | `projects/RPi/patches/linux/0001–0005` |
| `pi-dv-engine`: the engine, `libpi_engine.so.3` | `packages/multimedia/pi-dv-engine/` |
| An FFmpeg fix: the V4L2 request HEVC decoder no longer hangs on close after running out of CMA | `packages/multimedia/ffmpeg/patches/rpi/0002-…` |
| Kodi's build switch (`DOLBY_VISION_SUPPORT`, `-DENABLE_PIDV`) | `packages/mediacenter/kodi/package.mk` |
| Kodi: Dolby Vision from the EDID, a fenced video plane, the connector state, the engine's renderer, the profile 7 enhancement layer decoder, a Dolby Vision group in the video settings, and subtitles composited into the Dolby Vision signal | `projects/RPi/devices/RPi5/patches/kodi/0001–0007` |
| FFmpeg: H.264 MVC decoding (both views of Blu-ray 3D and 3D camcorder streams), Dom Cobley's series backported to 9.0 | `packages/multimedia/ffmpeg/patches/rpi/0003-…` |
| Kodi: stereoscopic 3D, Dom Cobley's series: HDMI frame-packed, side-by-side and top-and-bottom modes with an eye per video plane, H.264 MVC and MV-HEVC, Blu-ray 3D, 3D subtitles at the disc's depth | `projects/RPi/devices/RPi5/patches/kodi/0008–0046` |
| Kodi: the Dolby Vision options as settings in Player / Videos, which every video starts with | `projects/RPi/devices/RPi5/patches/kodi/0047` |
| Kodi: the GUI (OSD, menus) composited into the Dolby Vision signal, as the subtitles are, in the areas it covers | `projects/RPi/devices/RPi5/patches/kodi/0048` |
| LibreELEC's update menu offers this repository's releases, from `releases.json`, instead of LibreELEC's | `projects/RPi/devices/RPi5/patches/LibreELEC-settings/` |

**Build:** as LibreELEC, `PROJECT=RPi DEVICE=RPi5 ARCH=aarch64 make image`.
The engine's source is downloaded from this repository's release
`engine-<version>` (`pi-dv-engine-<version>.tar.xz`).

**Use:** flash the image, or copy its `.tar` to `/storage/.update/` and
reboot. The display mode must be the video's size: turn on "Adjust display
refresh rate" with a 4K desktop for 4K streams. Dolby Vision must be among
the player's "Allowed HDR dynamic metadata formats". The "Dolby Vision"
group in Player / Videos holds the options every video starts with: the
enhancement layer and the infoframe, and at the Expert settings level the
enhancement layer upscale, chroma MMR and filters. During playback the
video settings' "Dolby Vision" group switches the output and changes the
options for that video; "Set as default for all media" there stores them
in Player / Videos.

**Updates:** LibreELEC / Updates offers this repository's builds in two
channels: DV-13.0 (the releases) and DVbeta-13.0 (the pre-releases: Linux
7.4 with VRR, from the `kernel-7.4` branch). A build checks the channel it
belongs to for a newer one, and installs it at the next reboot while
"Automatic updates" is on auto; set it to manual to choose a channel and a
build. Builds before 0.4.1 and 0.5.0-beta.5 ask LibreELEC's update server
instead: to update one from the menu, set "Automatic updates" to manual,
turn on "Show custom channels" and enter
`raw.githubusercontent.com/dangerouslaser/libreelec-rpi-dv/main` as a
custom channel.

Subtitles and the GUI over the video (the OSD, menus) are composited into
the Dolby Vision signal, in their own colours.

**3D:** set "Playback mode of stereoscopic 3D video" (Player / Videos) to
"Preferred mode", leave "Stereoscopic 3D mode" (System / Display) disabled
with its preferred mode "Same as movie", and turn on "Adjust display
refresh rate". The 3D modes are 1080p, which Kodi only switches to from a
1920x1080 desktop; 4K Dolby Vision streams then need the 4K modes in the
whitelist. A 3D display gets the frame-packed 1080p24 mode where it offers
one; full side-by-side and top-and-bottom sources are converted to it.
H.264 MVC is decoded in software.

**Limitations:** with the OSD open, 4K FEL plays about 16 of 24 pictures a
second (the GUI's composite and drawing take the GPU over the budget);
dual-track profile 7 isn't read; the
sharper enhancement layer upscales (bicubic, Lanczos3, Annex B) aren't
real time for 4K FEL on the Pi 5; field-coded H.264 MVC (3D camcorder
recordings) crashes Kodi when it is deinterlaced. Tested on an LG B4, which
has no 3D modes: 3D was checked as side-by-side output there.

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
