# This is a fork of the simple XP-Pen DECO 02 hack made by @vale981

This simple program listens for the eraser events and runs shell
commands in reaction to those events. I've made some modifications in order
to have a better user experience and mostly to suit my needs.

## Side notes

This version of the `hack` is meant to work with my graphic tablet
the XP-Pen G640, the only thing that really changes for the purpose of this
program is the event code being dispatched by the event device when pressing
the pen side button.

### The event codes:
 - `331` XP-Pen G640
 - `321` XP-Pen DECO 02

## Install
 - `make install` and you're good

You have to run this with root permissions... Also make sure to have installed
`xdotool` in your machine otherwise this program won't work, since its what it
uses to send keyboard interrupts to the os.
