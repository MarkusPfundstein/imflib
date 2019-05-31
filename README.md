# imflib

Goal: encode IMF App2(E) compatible MXF and create and edit IMPs.

State: Basic creation and editing works. Files pass Netflix' photon.

*Note*: For decoding IMF files, see [imf-frameserver](https://github.com/MarkusPfundstein/imf-frameserver)

The development of imflib so far has been sponsored 100% by [ODMEDIA](http://www.odmedia.com). I am looking currently for more sponsors. Please get in touch.

## Features

- create and edit App2E compatible IMPs from your video files (RGB, YUV444, YUV422, 10 & 12bit, FullRange)
- passes Netflix' photon
- supports multiple CPLs, virtual tracks etc.

## Overview

This library consists of two programs. `imfenc` and `imfgui`. 

### imfenc
`imfenc` is a command line application that lets you encode arbitrary videos to IMF App2E compatible MXF files containing JPEG2000 and PCM.

### imfgui
`imfgui` is a Qt4 app GUI application that allows you to create App2E compatible IMPs. It's not very flexible at the moment but gets the job done.

## Drawbacks

- The programs are a WIP. Therefore they might be clumsy to use :-) Please open an issue on Github if you need a certain feature implemented or if you encounter bugs.
- Audio MXF creation not yet finished. Needs to be done manually using custom as-02-wrap build from this repo (see manual.pdf)

## Installation

```
sudo apt install libtool automake autoconf cmake pkg-config yasm qt4-qmake libqt4-dev

sh install-deps.sh
autoreconf -if
export LPATH="$(pwd)/third_party"
export CPP_FLAGS="-I${LPATH}/openjpeg/include -I${LPATH}/asdcplib/include -I${LPATH}/ffmpeg/include"
export LDFLAGS="-L${LPATH}/openjpeg/lib -L${LPATH}/asdcplib/lib -L${LPATH}/ffmpeg/lib -L${LPATH}/openssl/lib"
./configure
make
```

## Run it

### imfenc

Run `sh imfenc.sh`

```
markus@ubuntu:~/projects/imflib$ sh imfenc.sh --help
Allowed Options:
  -h [ --help ]                       show help
  -i [ --in ] arg                     input file
  -o [ --out ] arg                    directory where output files shall be
                                      stored
  -t [ --temp_dir ] arg               directory for temporary files, defaults
                                      to output directory
  -f [ --force ]                      overwrite output files
  -b [ --broadcast_profile ] arg (=1) broadcast profile (1-7)
  -d [ --bitdepth ] arg (=10)         target video bit depth (8, 10 or 12)
  -r [ --sample_rate ] arg (=48000)   target audio samplerate (48000 or 96000)
  -p [ --pixel_fmt ] arg (=RGB444)    pixel format of output. (YUV444, YUV422,
                                      RGB444)
  --use_tiles                         use tiles (only broadcast profile 6 and
                                      7) [default: false]
  --nomux                             dont mux the encoded files into MXF
  --experimental_extract_audio        extracts pcm
  --threads arg                       number of threads for jpeg2000 encoding
  --full_range arg (=0)               full range color space (rgb essence
                                      only), else SMPTE 274M-2008 constraints
                                      are used
```

Example for mov file:

`sh imfenc.sh -i <YOUR_FILE>.mov -o ~/<OUTPUT_DIR> -b 6 -p RGB444 --full_range 1 --threads 16 --experimental_extract_audio`

### imfgui

Start the gui with `sh imfgui.sh`

Examples coming, for now see the file `manual.pdf` and follow the steps

## LICENSE

LGPL
