![Revo](./assets/banner-dark.avif#gh-dark-mode-only)
![Revo](./assets/banner-light.avif#gh-light-mode-only)

<p align="center">
  <img src="https://img.shields.io/badge/License-MIT-red?style=for-the-badge" alt="MIT License" />
  <img src="https://img.shields.io/badge/Status-WIP-orange?style=for-the-badge" alt="Work In Progress" />
  <img src="https://img.shields.io/github/stars/o2161405/Revo?style=for-the-badge&color=gold" alt="GitHub Stars" />
</p>

Revo is an experimental code virtualizer for 32-bit PowerPPC executables, geared towards Wii homebrew.

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

When compiling with devkitPPC, you must specify the section in your linker script. You can either use the setup provided in `tests/`, or if you want to do it manually (using devkitPPC's new linking system):

1. Copy the `libogc_common.ld` and `rvl.ld` files from your devkitPPC installation to your project root:
    ```bash
    cp "$DEVKITPPC/powerpc-eabi/lib/"{libogc_common.ld,rvl.ld} .
    ```

2. Open `libogc_common.ld` and add the Revo section after `.text`:
    ```
    .revo_text : ALIGN_WITH_INPUT {
		*(.revo_text)
		*(.revo_text.*)
		. = ALIGN(32);
	} :text = 0
    ```

3. Open `rvl.ld` and update the include line to use the local copy of `libogc_common.ld`:
    ```
    INCLUDE ../libogc_common.ld
    ```

4. Edit your `Makefile` and replace your compiler flags with:
    ```
    CFLAGS	 = -g -O2 -Wall -ffunction-sections -fno-jump-tables -fno-optimize-sibling-calls $(MACHDEP) $(INCLUDE)
    CXXFLAGS =	$(CFLAGS) -std=gnu++20 -fno-exceptions -fno-rtti -fno-devirtualize
    ASFLAGS	 =	$(MACHDEP) -Wa,-mregnames
    LDFLAGS	 =	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map -T ../rvl.ld -Wl,-q
    ```

Finally, build your input file:
```bash
make
```

To virtualize your compiled binary, simply invoke Revo with your file:
```bash
./Revo --input example.elf
```