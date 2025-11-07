# Refal Android

This project demonstrates how an Android application can be built using the [Refal](https://en.wikipedia.org/wiki/Refal) programming language.

<img src="./assets/demo.gif" height="300">

## Project Structure

- `./rawdrawandroid`: Project that allows building Android applications in C
- `./Refal-05-Standalone`: Standalone distribution of the Refal-05 compiler
- `./refalrawdraw.c`: Refal bindings for the `rawdrawandroid` API
- `./mvu.ref`: Model-Update-View framework (Elm Architecture)
- `./main.ref`: Main application logic written in Refal

## Prerequisites

- Android SDK and NDK
- `make`
- `adb` (Android Debug Bridge)
- Connected Android device or emulator

Refer to [rawdrawandroid](https://github.com/cnlohr/rawdrawandroid) for more detailed guide.

## Development

**Clone and initialize submodules:**

```shell
git clone https://github.com/butvinm/Refal-Android
cd Refal-Android
git submodule update --init --recursive
```

**Generate application keys:**

```shell
make keystore
```

**Build and upload the application to a connected device:**

```shell
make push-run-from-refal
```

**Inspect application logs:**

```shell
# Clean logs
adb logcat -c

# Capture logs
adb logcat > logcat.log

# Grep linker errors
grep "librefal.so" logcat.log
```

## Acknowledgments

This project would not be possible without [Refal-05](https://github.com/Mazdaywik/Refal-05) and [rawdrawandroid](https://github.com/cnlohr/rawdrawandroid).
