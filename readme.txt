                +-8888888b.   .d8888b.  8888888888 888b    888-+
                | 888   Y88b d88P  Y88b 888        8888b   888 |
                | 888    888 888    888 888        88888b  888 |
            +---+ 888   d88P 888        8888888    888Y88b 888 +---+
          +-|---+ 8888888P"  888  88888 888        888 Y88b888 +---|-+
          | |   | 888        888    888 888        888  Y88888 |   | |
          | |   | 888        Y88b  d88P 888        888   Y8888 |   | |
        +-|-+   +-888         "Y8888P88 8888888888 888    Y888-+   +-|-+
   +----+ +----------------------------------------------------------+ +----+
   |    PGEN v1.5.1 Sega Genesis/Megadrive emulator for the Playstation 2   |
   |                                                                        |
   +-+            PS2 Specific code by Nick Van Veen (Sjeep)              +-+
   ++|    Based on source code from Generator, coded by James Ponder      |++
   ++|                  Readapted by AKuHAK                               |++
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                          Introduction                              |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+ 
   | PGEN is a Sega Genesis/Megadrive emulator for the PS2.  It began       |
   | as a port of Generator, an open source emulator for Linux, written     |
   | by James Ponder. Since then, PGEN has evolved to be much more than     |
   | that. The compatibility rate is very high and the speed is practically |
   | perfect with stereo sound.                                             |
   |                                                                        |
   | PGEN is freeware and can be downloaded at                              |
   |              https://bitbucket.org/AKuHAK/pgen/src/                   |
   |                            Please do not                               |
   | ask for ROMs or distribute this program with ROMs. We do not condone   |
   | such behavior.                                                         |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                    Current features in v1.5:                       |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | - Compatible with .SMD, .BIN and .GEN rom format                       |
   | - High speed emulation with stereo sound (at 48Khz)                    |
   | - Two types of sprite rendering: Cell and Line.                        |
   |   Cell is faster but less accurate.                                    |
   | - Pal/NTSC rom autodetect                                              |
   | - Supports SRAM saves and saved states (which are compressed)          |
   | - Save manager to manipulate PGEN save data                            |
   | - Very configurable (and options get saved to memory card or HDD)      |
   | - Quick savestate function, does not write to memcard                  |
   | - Screen positioning                                                   |
   | - Dual-shock controller analog stick support                           |
   | - Cool GUI with customizable music                                     |
   | - Automatic ROM detection (no FILES.TXT needed!)                       |
   | - Support for multiple levels of rom directories                       |
   | - Supports Joliet filesystem - long filenames, no file number limit    |
   | - Support for swapping ROM cd's without restarting the emulator        |
   | - Load zipped rom files                                                |
   | - Linear filtering for rendered display                                |
   | - Load ROMS from a PS2 HDD, and save/load saved states to a PS2 HDD    |
   | - Boot PGEN via M R Brown's exploit, and load roms from a memory card  |
   | - Support for the PS2 multi-tap for 4 player games                     |
   | - DMS3 Dev.olution mode compatibile                                    |
   +-+                                                                    +-+         
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                           To-do List:                              |++         
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | - Re-write rendering engine (to get 100% speed in ALL games)           |
   | - Fix any remaining bugs                                               |
   | - Change base emultaor to improve compatability                        |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|             Booting PGEN and getting ROM's in place                |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | PGEN can be loaded from a CDROM, HDD, USB, Memory card. It can also    |
   | load roms from any of those four devices. The method for booting PGEN  |
   | and getting the roms into place varies between each of the devices,    |
   | and each method is described below:                                    |
   |                                                                        |
   | CDROM                                                                  |
   | -----                                                                  |
   | NOTE: In order to boot PGEN or load roms from a CDROM, you will need   |
   | to know how to boot CDR's on your PS2. I can not provide help in this  |
   | area, please seek information on this from a PS2 related website or    |
   | forum such as http://www.ps2newz.net                                   |
   |                                                                        |
   | Prepare a CD compilation using your favourite burning software. For    |
   | this compilation, you must place the PGEN files (PGEN.ELF and          |
   | SYSTEM.CNF) in the root of the compilation. If you will be placing roms|
   | on the compilation (which isnt necessary, as you may wish to place the |
   | roms on a HDD or memory card instead), then place them in one or       |
   | several sub-directories in the root of the compilation. You must make  |
   | sure that the number of files and directories inside the root directory|
   | does not exceed 32. For this reason it is recommended that you have a  |
   | directory called "Roms" in the root of your compilation, and then place|
   | all your roms inside this directory. The 32 file/dir limitation only   |
   | applies for the root directory. Once your compilation has been         |
   | prepared, burn as a MODE2/XA disc.                                     |
   |                  You can get example of CD disk at                     |
   |      https://bitbucket.org/AKuHAK/pgen/downloads/pgen_test.zip         |
   |                                                                        |
   | Memory Card                                                            |
   | -----------                                                            |
   | PGEN can be loaded from memory card by using Free MC Boot.             |
   |                                                                        |
   | HDD (Hard Disk Drive)                                                  |
   | ---------------------                                                  |
   | PGEN can be loaded from a HDD installed in your PS2, and can load roms |
   | stored on the HDD. You can load PGEN from internal hard disk drive by  |
   | using HDD software, BB Navigator or simply uLaunchelf :)               |
   |                                                                        |
   | Notes about the PGEN rom list:                                         |
   | ------------------------------                                         |
   |                                                                        |
   | The PGEN romlist is built the same way for each of the 3 supported     |
   | devices, according to the layout of the filesystem for each device.    |
   | This means that you can have sub-directories which span off the root   |
   | directory, and you can use directories to categorise your rom files.   |
   |                                                                        |
   | PGEN will use the filenames of roms when building the romlist - it     |
   | will not scan rom headers to get the rom name (this would take far too |
   | much time). This means that if a rom's filename was "Sonic the Hedgehog|
   | 2 (JUE) [!].smd", it will appear as "Sonic the Hedgehog 2 (JUE) [!]"   |
   | in the romlist. We suggest using the GoodGEN utility to rename your    |
   | roms to use the standard rom naming convention. You can download the   |
   | GoodGEN utility from the PGEN website.                                 |
   |                                                                        |
   | NOTE: Roms MUST have either a ".bin", ".smd", ".gen" or ".zip"         |
   |       extension to be recognised by PGEN.                              |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                      Default In-Game Controls                        |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   |   Genesis - PS2                                                        |
   |   -------------                                                        |
   |   D-Pad   - D-Pad or left analog stick                                 |
   |   Start   - Start                                                      |
   |   A       - Square                                                     |
   |   B       - X                                                          |
   |   C       - Circle                                                     |
   |   X       - L1                                                         |
   |   Y       - R1                                                         |
   |   Z       - R2                                                   |
   |   Mode    - L2                                                         |
   |                                                                        |
   |   Select on PS2 controller = enter ingame menu                         |
   |                                                                        |
   | NOTE: If you are having problems with 6-button gamepad emulation, or   |
   |       just want to disable it, hold down Mode when starting a game to  |
   |       forcefully disable 6-button gamepad emulation.                   |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                           PGEN Usage                               |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | NOTICE: You CAN NOT use saved states from any PGEN version older than  |
   |         1.1. The staved state format has changed as                    |
   |         of version 1.1 and any old saved states are incompatible. You  |
   |         wont need to delete your old saves however, you can still use  |
   |         them with the older PGEN releases.                             |
   |                                                                        |
   | GUI controls:                                                          |
   | -------------                                                          |
   | While navigating the GUI, use the up and down arrows on the d-pad to   |
   | change selection, X to confirm selection and triangle to return to the |
   | previous dialog. At any time in the GUI you may change the video mode. |
   | Hold down all shoulder buttons (L1 + L2 + R1 + R2) and then press      |
   | START to change to NTSC or SELECT to change to PAL.                    |
   |                                                                        |
   | Rom list:                                                              |
   | ---------                                                              |
   | Use the d-pad to navigate through the rom list. Up/Down scroll one     |
   | item at a time, Left/Right scroll one page at a time. L1 places the    |
   | selection at the top of the romlist, L2 places the selection at the    |
   | bottom. If you scroll past the top rom, the selection will warp to the |
   | bottom rom, and vice-versa. Once you have made your selection press X. |
   | If a directory was selected, the romlist will change into that         |
   | directory. If a rom was selected, emulation of that rom will start.    |
   |                                                                        |
   |    Swapping rom CD's                                                   |
   |    =================                                                   |
   |    You can swap rom CD's by returning to the base romlist directory    |
   |    (the one which lists "CDROM", "Memory Card 1" etc), switching CD's  |
   |    and then selecting "CDROM" again. Each time "CDROM" is selected     |
   |    from the base romlist directory, the CD listing is refreshed.       |
   |                                                                        |
   |    Swapping USB Flash Drives                                           |
   |    =================                                                   |
   |    You can swap rom USB by returning to the base romlist directory     |
   |    (the one which lists "CDROM", "Memory Card 1" etc), switching USB   |
   |    and then selecting "MASS" again. Each time "MASS" is selected       |
   |    from the base romlist directory, the USB listing is refreshed.      |
   |                                                                        |
   | Options menu:                                                          |
   | -------------                                                          |
   | The options menu is used to configure PGEN. If a memory card is        |
   | present in slot 1, the options will be saved to the memory card and    |
   | restored next time you start PGEN. Below is a description of each      |
   | setting available in the options menu:                                 |
   |                                                                        |
   | Region: This is the region that will be emulated. If set to AUTO, PGEN |
   |         will auto-detect the correct region for each rom when it loads.|
   |         If set to USA, Europe or Japan, the selected region will be    |
   |         forced.                                                        |
   |                                                                        |
   | Default Region: Some ROM's are designed for all regions.               |
   |                 If this situation arises, PGEN will emulate the        |
   |                 default region.                                        |
   |                                                                        |
   | Renderer: Use this option to select the rendering engine which will be |
   |           used to render the genesis display. The CELL engine is much  |
   |           faster than the LINE renderer, but does not render some games|
   |           properly. The LINE renderer is slower, but much more         |
   |           accurate.                                                    |
   |                                                                        |
   | Sound: Switch sound emulation on or off.                               |
   |                                                                        |
   | MultiMode TV: If this option is enabled, the PS2 will switch the TV    |
   |               video mode to the emulated video mode when you start     |
   |               emulation of each rom. For example, if you are using a   |
   |               PAL PS2 and you run a NTSC rom, the video mode of your   |
   |               TV will be changed from PAL to NTSC. This is useful,     |
   |               because it gets the roms running at the correct speed.   |
   |               This option should ONLY be enabled if your TV supports   |
   |               both the PAL and NTSC signal.                            |
   |                                                                        |
   | Reposition screen: Select this to enter screen repositioning mode.     |
   |                                                                        |
   | Frame counter: If this option is enabled, a framecounter will be       |
   |                displayed below the genesis display during emulation.   |
   |                                                                        |
   | Video mode: Switching between PAL or NTSC video modes                  |
   |                                                                        |
   | Save Device: If a properly formatted HDD is installed in your PS2 then |
   |              by default PGEN will use the HDD to store saved states    |
   |              and emulation settings. You can override this by setting  |
   |              save device to memory card instead of HDD.                |
   |                                                                        |
   | In-game menu:                                                          |
   | -------------                                                          |
   | During emulation, press SELECT to bring up the ingame menu. From here  |
   | you can save/load the game state and change the rendering settings.    |
   | Below is a description of each setting available in the ingame menu:   |
   |                                                                        |
   | Quick Save State:  Quickly saves the game state to RAM (not to the     |
   |                    memory card).                                       |
   | Quick Load State:  Quickly loads the game state from RAM.              |
   | MC/HDD Save State: Compresses the game state and saves to the memory   |
   |                    card.                                               |
   | MC/HDD Load State: Loads the game state for the game currently being   |
   |                    emulated from the memory card.                      |
   | Reposition Screen: Enter screen repositioning mode.                    |
   | Renderer:          Changes the rendering engine ingame.                |
   | Region:            Changes the region ingame. However, this will only  |
   |                    come into effect after a soft reset.                |
   | Soft Reset: Resets emulation of the current game.                      |
   |                                                                        |
   | Save manager:                                                          |
   | -------------                                                          |
   | The save manager is used to manipulate PGEN save data on the current   |
   | save device (one of MC or HDD, selectable from the options menu). From |
   | the save manager dialog, you can see how much space is free on the     |
   | save device, see the total amount of space taken up by PGEN saves, see |
   | how much space each individual save takes up, and most importantly     |
   | delete saves.                                                          |
   |                                                                        |
   | Navigate through the list of saves using the up/down arrows on the     |
   | d-pad. To delete a save, make your selection and press X. You will then|
   | be asked for confirmation to delete. Select "yes" and press X again.   |
   |                                                                        |
   | Note: Save names prefixed with "(SR)" are SRAM saves. All others are   |
   | saved states.                                                          |
   |                                                                        |
   | Gui music:                                                             |
   | -------------                                                          |
   | The music used was                                                     |
   | http://modarchive.org/index.php?request=view_by_moduleid&query=58827   |
   | credit due to who wrote this fantatic 8bit masterpieces.               |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                               FAQ                                  |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   |                                                                        |
   | 1) How do I get this to run on my Playstation 2?                       |
   |                                                                        |
   |        You need to be able to do one of the following:                 |
   |          * Boot CD-R's. This requires either a modchip or the use of   |
   |            a "swap trick". Please see http://www.ps2newz.net for more  |
   |            information about modchips and the swap trick.              |
   |          * A way to get PGEN and roms onto your memory card, for use   |
   |            with Free MC Boot. For more information please see          |
   |            http://psx-scene.com/forums/official-free-mc-boot-forums/   |
   |          * A HDD installed in your PS2 and a way to boot appplications |
   |            from the HDD. to my mind the best way to do it is to        |
   |            install Free MC Boot on your HDD.                           |
   |                                                                        |
   | 2) Ok, I downloaded all the files off the website. How do I make a cd? |
   |                                                                        |
   |        You can take a CD example from                                  |
   |        http://depositfiles.com/files/gnw56fthk       |
   |                                                                        |
   | 3) Sound is too slow/fast!                                             |
   |                                                                        |
   |        If possible, make sure that the emulated video mode is the      |
   |        same as your PS2 video mode. If the emulated video mode does    |
   |        not match the video mode of your PS2, the emulated game will    |
   |        not run at the correct speed. Eg: If a PAL video mode is being  |
   |        being emulated (region = Europe) but your PS2 it using a NTSC   |
   |        video mode, than the game will run 20% faster than it should,   |
   |        which means that the music will run 20% faster too.             |
   |                                                                        |
   | 4) PGEN saving into memory card takes up to 5 minutes.                 |
   |                                                                        |
   |        Yep this is PGEN related bugs if you are using bad memory or    |
   |        memory card with a little bit broken structure. Unfortunately   |
   |        for now there is no way to fix this. You can try to press POWER |
   |        button on your PS2 to interrupt the process and try again.      |
   |                                                                        |
   | 5) Sometimes PGEN was frozen. What can i do if it is happened?         |
   |                                                                        |
   |        You can try to press POWER button.                              |
   |                                                                        |
   | 6) POWER button ddoesn't switch off the console.                       |
   |                                                                        |
   |        Thu can try to press POWER button.                              |
   |                                                                        |
   | 7) I cant load my saved states from earlier PGEN releases, what gives? |
   |                                                                        |
   |        The saved state format has changed with v1.1, and any old saves |
   |        are incompatible. You can still use the old saves with the old  |
   |        PGEN releases, however.                                         |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                    Outdated History                                |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   |                                                                        |
   |   v1.5.1s - Changes include:                                           |
   |                                                                        |   
   | This is Pgen 1.51.s with a new look and modified to work on a ps2      |
   | compatible ps3. The new ps3 look was done by Samson from ps3hax. The   |
   | modifications for the ps3 were done by aries2k. This version doesn´t   |
   | support hdd saving. I had to deactivate the hdd modules for the elf to |
   | load. All references to naplink were removed. The SifIopReset sequence |
   | was removed and replaced with the one from uLaunch.elf by E.P and      |
   | dlanor. All necessary .Irx modules have been embedded in the elf.The   |
   | Irx module are all 100% homebrew from the ps2 homebrew sdk. :)         |
   | No copyrighted Sony ps2 bios modules are used.                         |
   |                                                                        |
   |   18th March, 2007: v1.5.1 - Changes include:                          |
   |                                                                        |   
   | - Now using older ps2hdd related IRX modules instead of latest         |
   |   PS2SDK ones because console wasn't turning off!                      |
   | - Fixed minor credits screen scroll bug.                               |
   |                                                                        |
   |   10th Janurary, 2007: v1.5 - Changes include:                         |
   |                                                                        |
   | - CPU68K ABCD instruction fix by bootsector (thanks Barry!)            |
   | - Added  gslib 0.51 library files                                      |
   | - Changed background image (thanks to luckess ;)                       |
   |                                                                        |
   |   28th July, 2006: v1.4 - Changes include:                             |
   |                                                                        |
   | - Updated to recent ps2sdk                                             |
   | - Added usb mass storage support                                       |
   |                                                                        |
   |   30th Janurary, 2004: v1.2 - Changes include:                         |
   |                                                                        |
   | - Fixed zipped rom support                                             |
   | - Fixed loading PGEN via AR2 or other swap disc                        |
   |                                                                        |
   |   16th November, 2003: v1.1 - Changes include:                         |
   |                                                                        |
   | - Just about all PS2 specific code re-written                          |
   | - Started using gsLib for graphics                                     |
   | - Added scroll bar to romlist and Save Manager                         |
   | - Added support for loading ROM's from the memory cards                |
   | - Added HDD support. Now possible to save/load states and options to   |
   |   the HDD, and load ROM's from the HDD                                 |
   | - CD refresh obsolete, removed                                         |
   | - Added support for zipped roms                                        |
   | - Filtering now available for rendering (hardware linear filtering)    |
   | - Saved states now use zlib for compression. Old saves are             |
   |   incompatible.                                                        |
   | - Killed support for OLD JAP consoles (sorry :P)                       |
   | - Fixed bug with FIFO empty (VDP control port) emulation. Fixes games  |
   |   such as Wonderboy 5 and Splatterhouse 2.                             |
   | - Added multitap support for use in 4 player games                     |
   | - No longer load modules from cdrom, making it possible to boot from a |
   |   memory card via the PS2 Independence exploit                         |
   | - Many other misc changes                                              |
   |                                                                        |
   |   16th April, 2003: v1.0 BETA - Changes include:                       |
   |                                                                        |
   | - Lots of bugfixes                                                     |
   | - Disclaimer only showed on first use (if you have a memory card)      |
   | - Added support for swapping ROM cd's                                  |
   | - Added support for customising the music played in the menu screen    |
   | - Added old Japanese console compatibility                             |
   | - Removed crypto protection code                                       |
   | - Speedups                                                             |
   |                                                                        |
   |   2nd August, 2002: RC2 beta - Changes include:                        |
   |                                                                        |
   | - Minor speed increase                                                 |
   | - Fixed bug in pag code (you can now use an old PS1 digital controller)|
   | - Fixed "red screen of death" bug (added checksum fixer)               |
   | - Added proper region emulation, improved region detection code        |
   | - Made selector loop in ingame menu and romlist                        |
   | - Fixed FPS counter position with NTSC video mode                      |
   | - Additions to the ingame menu: Soft reset, Region selection, Screen   |
   |   repositioning                                                        |
   | - Now include screen position in saved options                         |
   | - Spin down CD when not in use                                         |
   | - Added emulation of 6-button genesis controller                       |
   | - Added support for subdirs inside the base rom directory              |
   | - Added shortcuts in romlist: L1 to move to top of list, L2 to move to |
   |   bottom                                                               |
   |                                                                        |
   |   21st July, 2002:  RC1 beta - First official release                  |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                             Contact                                |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | PGEN Homepage: https://bitbucket.org/AKuHAK/pgen                       |
   |                                                                        |
   | AKuHAK: new pgen developer                                             |
   | - e-mail: akuhak@gmail.com                                             |
   |                                                                        |
   |                                                                        |
   | Nick Van Veen (aka Sjeep) - PGEN Author (stop working on emulator)     |
   | - e-mail: sjeep@gamebase.ca                                            |
   | - IRC: Sjeep, on EFNet                                                 |
   |                                                                        |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                        Credits/Greetz etc                          |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   |                              CREDITS                                   |
   |                              -------                                   |
   |                                                                        |
   | PS2 specific code, as well as many additions to the emulation core by  |
   |                    Nicholas Van Veen (aka Sjeep)                       |
   |                                                                        |
   |     Generator, the emulator which PGEN is based on, was written by     |
   |                            James Ponder                                |
   |                                                                        |
   |   PSG and FM emulation code by Stephane, author of the Gens emulator   |
   |                                                                        |
   |                  SjPCM sound output library by Sjeep                   |
   |                                                                        |
   |                   libMtap multitap library by Sjeep                    |
   |                                                                        |
   |                  libhdd HDD utility library by Sjeep                   |
   |                                                                        |
   |            HDD drivers by Sjeep, mrbrown, RCE and [RO]man              |
   |                                                                        |
   |         libCDVD PS2 CDVD library written by Sjeep and Hiryu            |
   |                                                                        |
   |   PS2Lib PS2 kernel library by Sjeep, Gustavo Scotti, Hiryu, mrbrown,  |
   |                        Oobles, Pukko and others                        |
   |                                                                        |
   |                  gsLib PS2 graphics library by Hiryu                   |
   |                                                                        |
   |                  AmigaMod mod file player by Vzzrzzn                   |
   |                                                                        |
   |             The memory card icon was created by Nikorasu               |
   |                                                                        |
   |                              THANKS TO:                                |
   |                              ----------                                |
   |                                                                        |
   |  Special thanks goes to the DMS team. They have chosen to sponsor me   |
   |  and my projects, and it is because of them that PGEN has been revived |
   |                   and will continue to be developed                    |
   |                                                                        |
   |    Hiryu for gsLib and libCDVD, for his continual help and support     |
   |                                                                        |
   |        The Unknown artists who composed the excellent MOD file         |
   |                            used in the menu                            |
   |                                                                        |
   | [vEX], Bgnome and MoRpHiUs for creating tutorials, documents, websites |
   |                           etc related to PGEN                          |
   |                                                                        |
   | Fat Mike, for lending me his spare HDD to assist in adding HDD support |
   |                                                                        |
   |    The BETA testers: Drakonite, emukid, Emulord, Hiryu, Jide, Jimmi,   |
   |                             Mark, Rob6021                              |
   |                                                                        |
   |                               GREETZ                                   |
   |                               ------                                   |
   |                                                                        |
   |  adk, adresd, blackdroid, drakonite, dreamtime, duke, guichi, herben,  |
   |     hiryu, jenova, jules, karmix, longchair, loser, mrbrown, nagra,    |
   |  nikorasu, norecess, oobles, oopo, pukko, rce, [ro]man, runtime, sg2,  |
   |  tyranid, warren, vzzrzzn and anybody else that I forgot to mention :) |
   +-+                                                                    +-+
   +++--------------------------------------------------------------------+++
   +++--------------------------------------------------------------------+++
   ++|                            Legalese                                |++
   +++--------------------------------------------------------------------+++
   +-+                                                                    +-+
   | This software is not endorsed by Sony Computer Entertainment Inc. or   |
   | SEGA Inc. in any way. This software is produced without the use of any |
   | copyrighted material which belongs to Sony Computer Entertainment Inc, |
   | or SEGA Inc, or any other party.                                       |
   |                                                                        |
   | Companies and all products pertaining to that company are trademarks of|
   | that company. Please contact the appropriate company for trademark and |
   | copyright information.                                                 |
   |                                                                        |
   | This software should only be used to play games which the user         |
   | legally owns.                                                          |
   |                                                                        |
   | This software must never be distributed with any copyrighted games or  |
   | other material. Any breach of these terms is out of the authors control|
   | and is not at the authors consent.                                     |
   |                                                                        |
   | PGEN is FREE software. If you bought this, you have been ripped off.   |
   +-+                                                                    +-+
   ++----------------------------------------------------------------------++
   ++                                                   Layout by MoRpHiUs ++
   +++    +----+    +----+          ____          +----+    +----+        +++
   +-|---------|---------|---------(0  o)--------------|---------|--------|-+
     +----+    +----+    +-----oo0--(__)--0oo-----+    +----+    +--------+
