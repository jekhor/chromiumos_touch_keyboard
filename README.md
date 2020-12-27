# Touch Keyboard

This is fork of ChromiumOS touch keyboard driver found at
https://chromium.googlesource.com/chromiumos/platform2/+/19effa94d56d31397a55292771c8ea196f419f1e/touch_keyboard/

To get some details about the project, check the [README.upstream.md](README.upstream.md) file.

## Build and install

```
apt install build-essential cmake
mkdir build
cd build
cmake ../
make
sudo make install
```

Or just run the `dpkg-buildpackage -b --no-sign` command to build .deb package.

## Configuration
To create custom keyboard layout, edit the file layout.csv and place it as /etc/touch_keyboard/layout.csv.
