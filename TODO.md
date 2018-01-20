- timEd: #pragma pack(__push, 1) for structures requiring it
  (partly done)

- Update README.md with latest news and build instructions

- timEd: Add more %vars to reply.c (%mmon already added)

- timEd & NetMgr: Double-check Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Merge any recent patches or bug fixes from the UNIX version

- timEd: Fix: Text cursor won't turn off in Windows NT version

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key (eg. Ctrl, Alt, Shift) is pressed (very minor bug!)

- WIMM: Rebuild

- JAMINFO: Rebuild (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit
