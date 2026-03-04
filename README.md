# BambooOS

## About

BambooOS is a hobbyist OS designed to test my knowledge and understanding of Computer Architecture and Operating Systems, utilising the Limine boot protocol.

This is the 3rd rewrite of the OS, and is not yet as feature-rich as previous versions. These included:

- Higher-half kernel
- Font rendering
- PMM and VMM
- Heap allocator
- ACPI support
- PCI enumeration
- Interrupts
- SMP support
- AHCI support
- FAT32 driver
- and more! 🚀

This repository contains the code which is currently being worked on. This is not yet stable, can change at any time, and may contain bugs, the majority of which are already known about.

## Building

To build BambooOS, simply run "make". This downloads Limine, and produces the full image to be run. To run the OS in Qemu, run "make run". To clean and remove all objects and binaries, run "make clean", and to remove the Limine download, run "make deepclean". I am also writing self-tests for the whole project - these will be run automatically whenever the OS starts.

## Support/Issues

If you have any issues or suggestions for the project, feel free to get in touch via the contact form on my website https://sebcrookes.co.uk, or create an issue.

## Testing

I am planning on writing a full suite of unit tests for this project, which will be located in the 'test' directory.

## Roadmap

The next few features which I am working on re-adding in this 3rd rewrite is support for:

- Interrupts
- AHCI
- NVMe
- FAT32 and filesystem management

And, later:
- Processes / running applications
- User accounts (with an age bracket API to simplify possible future age verification requirements)
- GUI

The OS will most likely not be POSIX compliant.

## Copyright + Licenses

Copyright © 2026 Sebastian Crookes. All rights reserved under copyright laws.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Other components not produced by me:

The font file "ter-u14n.psf", located in img/font/ was not created by me and is from "Terminus Font". It is licensed under the SIL Open Font License, Version 1.1, which can be found in the same directory, named "OFL.TXT". That file "OFL.TXT" also contains the copyright statement from the person granting the license. To find more about the font, see https://terminus-font.sourceforge.net/.

The Limine bootloader is downloaded automatically by make from GitHub to the "limine" directory, and is used when producing the final OS image. The LICENSE file can be found at limine/LICENSE (after "make" has been run), and is licensed under the BSD-2-Clause license. The LICENSE file must be displayed when distributing the final binary.
