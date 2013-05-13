PebbleSimulator
===============

Pebble Simulator and support libraries for Mac.

Vision
======

My vision is to be able to simulate the Pebble OS, without having to write an ARM emulator.
The idea is to provide a binary compatible version of "libpebble", called "libpebblesim", which shipped with "pebble-sdk-release-001".

This compatibility library can be used to compile a binary for OS X containing your app code.
Your binary can then be loaded into the Simulator, which will provide your app with the necessary "hardware" handles.
The most interesting bit is that you aren't bound to Pebble's limitations. TAKE ALL THE MEMORY!

I hope this Simulator will give developers the tools they need to rapidly iterate on Pebble Apps and Watch Faces.

Portability
===========

Portability is low on my priority list at the moment.
My original plan was to leverage the portability of libraries such as Qt and Cairo.
I'll take a closer look at it after the initial release of Pebble Simulator, which will be Mac only for now.

TODO
====

- Implement custom graphics backend, because Core Graphics doesn't support various filter modes supplied by Pebble.
- Implement Animations.
- Implement various other features, e.g. resolve remaining ~100 "figure it out." annotated functions.

You will find each of the pebble_os.c functions annotated with one of the following

1. "figure it out.": Not yet implemented.
2. "verify.": Implemented, but may need to be updated, tested or verified.
3. "malloc/init.": Same as "figure it out.", specially marked as initializer.
4. "free.": Free as in release, not as in beer, same as "figure it out.", specially marked as deinitializer.
5. None of the above: No clue, I'll have to review that.
