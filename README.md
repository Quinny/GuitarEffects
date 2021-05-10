# Guitar Effects Processor

A low latency guitar effects processor in C++ suitable for running on smaller
boards (e.g. a raspberry pi).

![unit in case](case/full.JPG)

# How it works

## High level overview

The [web UI](https://github.com/Quinny/GuitarEffects/tree/master/web/static)
calls into the [web server](https://github.com/Quinny/GuitarEffects/tree/master/web)
which then manipulates the [pedal board](https://github.com/Quinny/GuitarEffects/blob/master/web/pedal_board.h).

## Sound card interface

The [AudioTransformer](https://github.com/Quinny/GuitarEffects/blob/master/audio_transformer.h) class uses [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/index.html) to interface with the sound card.

The values read from the soundcard are then streamed into the provided
transformation function and written back out to the output device.

## Pedals

[pedal.h](https://github.com/Quinny/GuitarEffects/blob/master/pedal.h) defines
an interface for all pedals to conform to.

### Transformation

The root of it is the `Transform` function which takes an input signal, performs
any kind of transformation, and returns an output. All input and output values
should remain in the range [-1, 1] otherwise you'll produce some really gnarly
popping and cracking.

Many of the pedals utilize the [Q](https://github.com/cycfi/Q) library for some
signal processing primitives (e.g. filters, compression, etc.). Some other
utility classes can be found in the [fx](https://github.com/Quinny/GuitarEffects/tree/master/fx)
directory of this repo.

### Knobs

The `Describe` function advertises the current state of the pedal, primarily
the knobs. These knob name and values will be used to display in the UI
(covered later).

`AdjustKnob` provides a knob from the UI and relies on the implementation to
make the required changes to the pedal. These changes should be applied in a way
such that subsequent calls to `Describe` will reflect these changes (otherwise
the UI won't update).

### Registration

Each pedal should register itself via the `REGISTER_PEDAL` macro. This connects
the pedal to the [PedalRegistry](https://github.com/Quinny/GuitarEffects/blob/master/pedal_registry.h) so that other components can find it without explicitly knowing
about its existence.

## Pedal Board

The [PedalBoard](https://github.com/Quinny/GuitarEffects/blob/master/web/pedal_board.h)
essentially just wraps a list of pedals and chains their `Transform` functions.

## Web Server

The [web server](https://github.com/Quinny/GuitarEffects/blob/master/web/main.cpp)
uses the [crow](https://github.com/ipkn/crow) library to expose endpoints that
manipulate the pedal board.

The [PedalRegistry](https://github.com/Quinny/GuitarEffects/blob/master/pedal_registry.h)
is used here to automatically expose newly added pedals. You only need to add
an include statement within the [handlers](https://github.com/Quinny/GuitarEffects/blob/master/web/handlers.h) file to trigger the registration.

## Web UI

The web UI uses [React](https://reactjs.org). The bulk of the logic lives in
[app.jsx](https://github.com/Quinny/GuitarEffects/blob/master/web/static/app.jsx).

# Building the server

1. First clone the repo: `git clone --recurse-submodules https://github.com/Quinny/GuitarEffects`

1. Then install RtAudio. The steps here depend slightly on the platform you are
building on. See [install.txt](https://github.com/thestk/rtaudio/blob/master/install.txt) from the RtAudio repo. After following all the steps there run `make install` in the RtAudio directory so that the library files can be linked properly.

1. Install boost using your package manager (e.g. `brew`, `apt-get`, `yum`, etc).
If you Google "Install boost using `<package_manager>` you should find the
corresponding package name.

1. Install sdl2 using your package manager (e.g. `brew`, `apt-get`, `yum`, etc.).
If you Google "Install sdl on `<platform>` you should find more detailed
instructions.

1. Run `make server` from this repo's root.

1. Open a github issue if I missed a required dependency.

# Running the server

From the `web` directory, run `sudo ../bin/server` (sudo is required to run on
port 80).

# Setting up on a Raspberry Pi

My current setup is as follows:

1. A raspberry pi 4 connected to a [7 inch touch screen](https://www.amazon.com/Raspberry-Pi-7-Touchscreen-Display/dp/B0153R2A9I/)
1. A [Behringer audio interface](https://www.amazon.com/BEHRINGER-Audio-Interface-1-Channel-UM2/dp/B00EK1OTZC) connected to the PI's USB port
1. My guitar is plugged into the audio interface input port, and then the output
of the audio interface runs into my amp (or into the start of your physical pedal chain)
1. The server program from this repo runs on the raspberry pi
1. Chromium runs and connects to `localhost` so that I can interface with the
pedals from the touch screen

You can mix and match whatever components you want, any audio interface with
an input and output port should work, and you can run the server program on
any device you can plug an audio interface into. I've run this setup using my
Macbook with similar results (though bulky and less portable).

Note that since the server program exposes an HTTP server you can actually
navigate to the IP address of your PI from any device connected to the same
wifi and control your pedal chain from there.
