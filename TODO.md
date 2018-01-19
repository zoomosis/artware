- timEd: #pragma pack(__push, 1) for structures requiring it
  (partly done)

- Update README.md with latest news and build instructions

- timEd: Investigate timezone bug. Windows & OS/2 version use UTC time when
  posting new messages. Stranger still, the DOS version uses a time five hours
  in advance! Workaround in both cases is to set TZ=0.

- timEd & NetMgr: Double-check Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Fix OS/2 startup crash where screen is wider than 127 columns

- timEd: Merge any recent patches or bug fixes from the UNIX version

- timEd: Fix: Text cursor won't turn off in Windows NT version

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key (eg. Ctrl, Alt, Shift) is pressed (very minor bug!)

- WIMM: Rebuild

- JAMINFO: Rebuild (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit
