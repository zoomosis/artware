- Update README.md with latest news and build instructions

- timEd: Add more %vars to reply.c (%mmon already added)

- timEd: add DEBUG option to makefiles

- timEd: See SVN log of UNIX for other previously-added features and merge
  into this branch

- timEd & NetMgr: Verify Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Fix: Text cursor won't turn off in Windows NT version

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key (eg. Ctrl, Alt, Shift) is pressed (very minor bug!)

- WIMM: Rebuild

- JAMINFO: Rebuild (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit

- Check for Y2038 bugs.

- timEd: Adjust the size of the arealist window so it uses the width of
  the entire screen (currently fixed to 80 columns).
