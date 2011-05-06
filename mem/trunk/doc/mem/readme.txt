This is FreeDOS MEM version 1.8 alpha 1 from David O'Shea.  I have included
the only changed source file (MEM.C), you can get the other source files from
MEM17BET.ZIP.  MEM.EXE is built with OpenWatcom 1.3 with full debugging
symbols and isn't UPX'd which is why it is so large.

This is an alpha version for testing only.  There is still some work to be
done on this version:
- NLS files have not been updated, so you will probably get some outdated
  text which doesn't line up if you have LC_ALL or LANG set
- some lists show "Conventional memory details" but also include UMB
  information, i.e. the heading implies two separate lists would be used but
  they are not (yet)
- /U shows more information than it did previously; the code needs to be
  enhanced to filter out the unwanted stuff although I am not totally clear
  on what /U is meant to show and the fact that /U is short for "UPPER" seems
  confusing as it is showing conventional memory details

Please report any issues on freedos-devel@lists.soureforge.net and please go
easy on me as this is my first contribution to FreeDOS :)
