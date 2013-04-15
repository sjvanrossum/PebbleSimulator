PebbleSimulator
===============

Pebble Simulator and support libraries for Mac.

Structure
=========

Ha! You wish it were structured...
Seriously though, I'm trying my best to structure files, but I can't guarantee any structure at this point.

Vision
======

My vision is to be able to simulate the Pebble OS, without having to write an ARM emulator.
The idea is to provide a binary compatible version of "libpebble", called "libpebblesim", which shipped with "pebble-sdk-release-001".

This compatibility library can be used to compile a binary for OS X containing your app code.
Your binary can then be loaded into the Simulator, which will provide your app with the necessary "hardware" handles.
The most interesting bit is that you aren't bound to Pebble's limitations. TAKE ALL THE MEMORY!

I hope this Simulator will give developers the tools they need to rapidly iterate on Pebble Apps and Watch Faces.

TODO
====

You will find each of the pebble_os.c functions annotated with one of the following

1. "figure it out.": Not yet implemented.
2. "verify.": Implemented, but may need to be updated, tested or verified.
3. "malloc/init.": Same as "figure it out.", specially marked as initializer.
4. "free.": Free as in release, not as in beer, same as "figure it out.", specially marked as deinitializer.
5. None of the above: No clue, I'll have to review that.
