Game Boy Window as a Giant Sprite
===========

The original Game Boy (DMG/CGB/etc) is limited to 8x8 and 8x16 sprites, or a larger meta-sprite composed from up to 40 of those smaller, individually managed sprites. There are some workarounds, but generally this limits the maximum size of sprites to a small size on an already small screen.

As an exercise, this demos using the video overlay Window as a huge pseudo-sprite (with some limitations). The priority feature and transparent pixels of regular sprites are used to create some foreground/background layering.

* Star sprites: S_PRIORITY flag set so they appear behind the Window sprite, except where it's pixels are set to color zero.

* Game Boy sprites: S_PRIORITY flag *not* set so it's above the Window sprite.

* Background layer: All pixels set to color zero.


![Game Boy video overlay window Giant Sprite demo - nyancat cassette](https://raw.githubusercontent.com/bbbbbr/gb-window-as-sprite/release/info/screenshot_anim.gif)

Built with GBDK - http://gbdk.sourceforge.net/

