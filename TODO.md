- timEd: #pragma pack(__push, 1) for structures requiring it
  (partly done)

- timEd, NetMgr: Fix missing prototypes & warnings

- NetMgr: Remaining Y2K patches

- Update README.md with latest news and build instructions

- timEd & NetMgr: Double-check Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Fix OS/2 startup crash where screen is wider than 127 columns

- timEd: Merge any recent patches or bug fixes from the UNIX version

- timEd: Fix: Text cursor won't turn off

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key is pressed (very minor bug)

- Use relative paths in makefiles (where possible)

- Rebuild WIMM

- Rebuild JAMINFO (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit

