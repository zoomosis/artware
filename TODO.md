- Update README.md with latest news and build instructions

- timEd: add DEBUG option to makefiles

- timEd: See SVN log of UNIX for other previously-added features and merge
  into this branch

- timEd & NetMgr: Verify Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key (eg. Ctrl, Alt, Shift) is pressed (very minor bug!)

- WIMM: Rebuild

- JAMINFO: Rebuild (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit

- Check for Y2038 bugs.

- timEd: Cosmetic fix: Adjust the size of the arealist window so it uses the
  width of the entire screen (currently fixed to 80 columns). (requested by Dallas
  Hinton)

- timEd: Cosmetic fix: Make the use of initials in quoted messages
  configurable. (requested by Dallas Hinton)
  (see UNIX port where this was inadvertantly made the default in 2008, but
  later reverted)

- timEd: Cosmetic fix: Use ISO dates in the user interface. Alternatively,
  allow the user to specify the date format in strftime() form in TIMED.CFG.

- timEd: Bug: When hitting right arrow in an empty message base, timEd
  doesn't necessarily move to the first area with new mail.
  
- timEd: Document how to use timEd in Windows 10, using Legacy Mode in the
  shortcut properties.

- timEd: Convert the documentation to Markdown format.

- timEd: Write a TIMCOLOR.EXE replacement.
