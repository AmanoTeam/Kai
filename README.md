# Kai

A command-line utility to download contents from M3U8 playlists.

## Usage

### Quick Start

The most basic usage is as follows:

```bash
$ kai -u "http://example.com/playlist.m3u8"
```

This will:

* Select the video stream with the highest resolution and download it.
* If there are any audio or subtitle streams attached to the video, they will be downloaded as well.
* The output file will be saved as `media.mkv` in the current working directory.

By default, Kai will mux media streams using a built-in implementation of FFmpeg. If you want to force the usage of the FFmpeg CLI tool installed on your system, pass the `--prefer-ffmpeg-cli` flag:

```bash
$ kai \
    --url <url> \
    --prefer-ffmpeg-cli
```

You can also send custom HTTP headers with the `-H`/`--header` flag:

```bash
$ kai \
    --url <url> \
    --header "User-Agent: ..." \
    --header "Authorization: ..."
```

You can see a list of all available flags with `kai -h`.

### Loading M3U8 Playlists

You can load any M3U8 playlist with the `-u`/`--url` flag. It supports loading playlists from HTTP URLs and local files.

Loading from HTTP URLs:

```bash
$ kai -u "http://example.com/playlist.m3u8"
```

Loading from local files:
 
 ```bash
$ kai -u "/path/to/playlist.m3u8"
```

You can also set the base URL of the M3U8 playlist with the `--base-url` flag:

```bash
$ kai --base-url "https://example.com" -u "./master.m3u8"
```

This can be useful in cases where you have already downloaded an `.m3u8` file from somewhere else and want to parse it with Kai without explicitly loading from the original URL.

### Listing Available Streams

There are two ways of listing the streams of an M3U8 playlist: `-S`/`--list-streams` and `--dump`.

The first one will show a human-readable listing:

Command:

```bash
$ kai -S -u "https://kartatz.github.io/hls-example/master.m3u8"
```

Output:

```
Playlist #0
  Metadata:
    Type: Master Playlist

Media #0
  Metadata:
    Type: Audio
    Identification: audio_aac
    Name: eng
    Language: eng
    Options: (default = yes, autoselect = yes, forced = no)
    Duration: 00:10:00
    Segments: 61 (~9 seconds)

Variant Stream #0
  Metadata:
    Bandwidth: 140800
    Codecs: avc1.64001f,mp4a.40.2
    Resolution: 880x480
    Frame rate: 30
    Duration: 00:10:00
    Audio: eng (ID = "audio_aac")
    Segments: 60 (~10 seconds)
    Size: ~10.07 MB

Variant Stream #1
  Metadata:
    Bandwidth: 281600
    Codecs: avc1.64001f,mp4a.40.2
    Resolution: 1280x720
    Frame rate: 30
    Duration: 00:10:00
    Audio: eng (ID = "audio_aac")
    Segments: 60 (~10 seconds)
    Size: ~20.14 MB
```

The other one is most useful when parsing M3U8 playlists with third-party programs, as it outputs a JSON tree:

Command:

```bash
$ kai --dump -u "https://kartatz.github.io/hls-example/master.m3u8"
```

Output:

```json
{
  "type": {
    "id": 1,
    "name": "Master Playlist"
  },
  "streams": [
    {
      "type": {
        "id": 13,
        "name": "Media Stream"
      },
      "mtype": {
        "id": 2,
        "name": "AUDIO"
      },
      "group-id": "audio_aac",
      "language": "eng",
      "assoc-language": null,
      "name": "eng",
      "default": true,
      "autoselect": true,
      "forced": false,
      "instream-id": null,
      "characteristics": null,
      "channels": null,
      "duration": 600,
      "average-duration": 9,
      "segments": 61,
      "uri": "https://kartatz.github.io/hls-example/audio.m3u8",
      "media": [ /* ... */ ]
    },
    {
      "type": {
        "id": 14,
        "name": "Variant Stream"
      },
      "bandwidth": 140800,
      "average-bandwidth": null,
      "program-id": null,
      "codecs": "avc1.64001f,mp4a.40.2",
      "resolution": {
        "width": 880,
        "height": 480
      },
      "frame-rate": 30,
      "duration": 600,
      "average-duration": 10,
      "audio": {
        "name": "eng",
        "group-id": "audio_aac"
      },
      "video": null,
      "subtitles": null,
      "closed-captions": null,
      "segments": 60,
      "size": 10560000,
      "uri": "https://kartatz.github.io/hls-example/video-sd.m3u8",
      "media": [ /* ... */ ]
    },
    {
      "type": {
        "id": 14,
        "name": "Variant Stream"
      },
      "bandwidth": 281600,
      "average-bandwidth": null,
      "program-id": null,
      "codecs": "avc1.64001f,mp4a.40.2",
      "resolution": {
        "width": 1280,
        "height": 720
      },
      "frame-rate": 30,
      "duration": 600,
      "average-duration": 10,
      "audio": {
        "name": "eng",
        "group-id": "audio_aac"
      },
      "video": null,
      "subtitles": null,
      "closed-captions": null,
      "segments": 60,
      "size": 21120000,
      "uri": "https://kartatz.github.io/hls-example/video-hd.m3u8",
      "media": [ /* ... */ ]
    }
  ]
}
```

### Selecting Streams to Download

There are two flags that can be used for selecting specific streams to download: `--select-stream` and `--select-media`.

The `--select-stream` flag can be used to select streams based on their position in the master playlist and also by resolution. It is mainly intended for selecting video streams.

The `--select-media` flag is used to select anything that is not a video, like audio and subtitles. It can be used to select streams based on their position in the master playlist.

#### Selecting Video Streams

Here is an example playlist:

```
Playlist #0
  Metadata:
    Type: Master Playlist

Media #0
  Metadata:
    Type: Audio
    Identification: audio_aac
    Name: eng
    Language: eng
    Options: (default = yes, autoselect = yes, forced = no)
    Duration: 00:10:00
    Segments: 61 (~9 seconds)

Variant Stream #0
  Metadata:
    Bandwidth: 140800
    Codecs: avc1.64001f,mp4a.40.2
    Resolution: 880x480
    Frame rate: 30
    Duration: 00:10:00
    Audio: eng (ID = "audio_aac")
    Segments: 60 (~10 seconds)
    Size: ~10.07 MB

Variant Stream #1
  Metadata:
    Bandwidth: 281600
    Codecs: avc1.64001f,mp4a.40.2
    Resolution: 1280x720
    Frame rate: 30
    Duration: 00:10:00
    Audio: eng (ID = "audio_aac")
    Segments: 60 (~10 seconds)
    Size: ~20.14 MB
```

This playlist contains 2 video streams. The first one (`Variant Stream #0`) is in 480p, while the other one (`Variant Stream #1`) is in 720p.

By default, when not explicitly selected, Kai chooses to download the variant stream with the highest bandwidth, which most of the time will be the video stream with the highest resolution.

But what if you want to download the 480p variant or any other resolution?

You can use `--select-stream` by providing it the stream position, resolution, or wordings like `worst`, `medium`, and `best`.

```bash
# Select by position
$ kai --select-stream "0" -u <url>

# Select by resolution
$ kai --select-stream "480p" -u <url>

# You can also use "worst", "medium" or "best"
$ kai --select-stream "worst" -u <url>
```

#### Select Audio and Subtitle Streams

You can select audio and subtitle streams with the `--select-media` flag. Unlike `--select-stream`, you can specify this argument as many times as you want.

It accepts a numeric value (which represents the position the media stream appears in the master playlist) and also a wildcard character (`*`).

```bash
# Select by position
$ kai --select-media "0" -u <url>
 
# Select all streams (audio, subtitles, etc)
$ kai --select-media "*" -u <url>
```

### Muxer Choice

Kai uses a built-in implementation of FFmpeg to mux media streams. However, sometimes you might want to avoid using it.

The internal implementation is more limited compared to the FFmpeg CLI tool; It supports only a limited set of codecs and may fail to mux media streams in some edge cases.

To work around this, you can force Kai to use the FFmpeg CLI tool installed on your system with the `--prefer-ffmpeg-cli` flag.

```bash
$ kai --prefer-ffmpeg-cli -u <url>
```

### Concurrent Downloading of Segments

Since M3U8 playlists deliver content through fragmented media streams (also called media segments), downloading an entire stream can take a lot of time, especially very long ones, which may contain thousands of media segments.

By default, Kai downloads media segments concurrently (i.e., multiple segments are downloaded simultaneously). You can control this behavior with the `-c`/`--concurrency` flag.

```bash
$ kai -c <number> -u <url>
```

This flag accepts a numeric value that represents how many media segments should be downloaded simultaneously. It defaults to the number of CPU cores available on your machine.

### Proxying Requests

Kai supports setting a custom proxy for network requests with the `-x`/`--proxy` flag.

```bash
# SOCKS5 proxy
$ kai -x socks5://<hostname>:<port> -u <url>

# SOCKS4 proxy
$ kai -x socks4://<hostname>:<port> -u <url>

# HTTP(S) proxy
$ kai -x http://<hostname>:<port> -u <url>
```

## Installation

The easiest way to install Kai is by downloading a pre-compiled binary from the [releases](https://github.com/AmanoTeam/Kai/releases/latest) page.

Below is a listing of some of the supported platforms and their download links:

### Linux

* [Kai for Linux (arm64)](https://github.com/AmanoTeam/Kai/releases/latest/download/aarch64-unknown-linux-gnu.tar.xz)
* [Kai for Linux (arm)](https://github.com/AmanoTeam/Kai/releases/latest/download/arm-unknown-linux-gnueabihf.tar.xz)
* [Kai for Linux (x86)](https://github.com/AmanoTeam/Kai/releases/latest/download/i386-unknown-linux-gnu.tar.xz)
* [Kai for Linux (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-unknown-linux-gnu.tar.xz)

### Windows

* [Kai for Windows (arm64)](https://github.com/AmanoTeam/Kai/releases/latest/download/aarch64-w64-mingw32.zip)
* [Kai for Windows (arm)](https://github.com/AmanoTeam/Kai/releases/latest/download/armv7-w64-mingw32.zip)
* [Kai for Windows (x86)](https://github.com/AmanoTeam/Kai/releases/latest/download/i686-w64-mingw32.zip)
* [Kai for Windows (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-w64-mingw32.zip)

### FreeBSD

* [Kai for FreeBSD (arm64)](https://github.com/AmanoTeam/Kai/releases/latest/download/aarch64-unknown-freebsd.tar.xz)
* [Kai for FreeBSD (x86)](https://github.com/AmanoTeam/Kai/releases/latest/download/i386-unknown-freebsd.tar.xz)
* [Kai for FreeBSD (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-unknown-freebsd.tar.xz)

#### OpenBSD

* [Kai for OpenBSD (arm64)](https://github.com/AmanoTeam/Kai/releases/latest/download/aarch64-unknown-openbsd.tar.xz)
* [Kai for OpenBSD (arm)](https://github.com/AmanoTeam/Kai/releases/latest/download/arm-unknown-openbsd.tar.xz)
* [Kai for OpenBSD (x86)](https://github.com/AmanoTeam/Kai/releases/latest/download/i386-unknown-openbsd.tar.xz)
* [Kai for OpenBSD (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-unknown-openbsd.tar.xz)

#### NetBSD

* [Kai for NetBSD (x86)](https://github.com/AmanoTeam/Kai/releases/latest/download/i386-unknown-netbsdelf.tar.xz)
* [Kai for NetBSD (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-unknown-netbsd.tar.xz)

#### Dragonfly BSD

* [Kai for Dragonfly BSD (x86_64)](https://github.com/AmanoTeam/Kai/releases/latest/download/x86_64-unknown-dragonfly.tar.xz)