![Revo](./assets/banner-dark.avif#gh-dark-mode-only)
![Revo](./assets/banner-light.avif#gh-light-mode-only)

<p align="center">
  <img src="https://img.shields.io/badge/License-MIT-red?style=for-the-badge" alt="MIT License" />
  <img src="https://img.shields.io/badge/Status-WIP-orange?style=for-the-badge" alt="Work In Progress" />
  <img src="https://img.shields.io/github/stars/o2161405/Revo?style=for-the-badge&color=gold" alt="GitHub Stars" />
</p>

Revo is an experimental code virtualizer for 32-bit PowerPC executables, geared towards Wii homebrew.

Revo operates under a "Bring Your Own Protections" model; while the code does offer some example mutation and virtualization features, the security of virtualized apps relies on implementing novel, closed-source features.

## Building
### Requirements
- g++ >= 16.1 (via [MSYS2](https://www.msys2.org/) for Windows users)
- ninja 1.3 ([Link](https://ninja-build.org/))
- Python 3.6

### How to build
Generate the build file:
```bash
python3 configure.py
```

Then run ninja to build:
```bash
ninja
```

## Usage
Revo relies on a custom section to detect input functions. The easiest way to manage this is using the API:
```c++
#include <Revo/api.h>

// use the VIRTUALIZE macro for functions you want to virtualize
VIRTUALIZE int main() {
    // your code as usual
    return 0;
}
```

If you're writing assembly files, declare the section name at the top of your file:
```
.section .revo_text, "ax", @progbits
```

In your Makefile, add `-q` to your `LDFLAGS`.

**Tip**: adding these compiler flags are recommended:
```
-ffunction-sections -fno-jump-tables -fno-optimize-sibling-calls -fno-exceptions -fno-rtti -fno-devirtualize
```

Then compile your program normally and run Revo with your `input.elf` file in the same directory:
```bash
./Revo
```

## Contributing
I'm not accepting contributors at this time; this is a solo project intended for my personal portfolio, so I won't be looking at pull requests or reviewing anyone else's code.

You are welcome to fork and add features (which is the intended security model of this project anyways), but I'm intentionally not looking at third-party additions to prevent any subconscious biases about design decisions.

Issues for feature requests and bug reports remain open as always.
