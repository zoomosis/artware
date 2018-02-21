- timEd: #pragma pack(__push, 1) for structures requiring it
  (partly done)

- Update README.md with latest news and build instructions

- timEd: Add more %vars to reply.c (%mmon already added)

- timEd: add DEBUG option to makefiles

- timEd: Add FidoNodelist code (raw nodelist lookup) code from UNIX version

- timEd: Add ArealistWraparound code from UNIX version

- timEd: See SVN log of UNIX for other previously-added features and merge
  into this branch

- timEd & NetMgr: Verify Y2K patches and test where possible

- timEd: Test Hudson code. Probably need to use old Y2K-patched timEd to
  verify data integrity. (Can do this on SDM, Squish & JAM, too.)

- timEd: Fix: Text cursor won't turn off in Windows NT version

- timEd: Fix: Popup windows in NT build erroneously close when a keyboard
  modifier key (eg. Ctrl, Alt, Shift) is pressed (very minor bug!)

- timEd: Fix: Erroneous error messages in OS/2 version when scanning Hudson
  bases. Presumably a struct alignment issue?

- WIMM: Rebuild

- JAMINFO: Rebuild (requires Borland C++ for DOS & OS/2)

- When code is declared "stable", run GNU Indent over it all and clean up
  things a bit

- Check for Y2038 bugs.
