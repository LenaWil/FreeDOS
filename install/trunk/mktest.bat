@echo off

echo MKTEST
echo Generates *.DIR directories and *empty* *.ZIP files, to test the
echo new Install program. To undo, delete the *.DIR directories and
echo contents. NOTE THAT EXISTING *.ZIP FILES MAY BE OVERWRITTEN!

choice Make test directories now?
if ERRORLEVEL 2 goto :done

echo Working ...

for %d in (base boot devel edit gui net sound util) do mkdir %d.dir

cd base.dir
for %f in (append assign attrib chkdsk choice command comp country cpidos) do echo %f > %f.zip
cd ..

cd boot.dir
for %f in (blackout bootmgr bootspl grub4dos smbtmgr xosl) do echo %f > %f.zip
cd ..

cd devel.dir
for %f in (arrow basec bin2c bwbasic cc386 clib dflat dmake fasm freebas) do echo %f > %f.zip
cd ..

cd edit.dir
for %f in (blocek fed freemacs mined ospedit setedit tde vim) do echo %f > %f.zip
cd ..

cd gui.dir
echo opengem > opengem.zip
cd ..

cd net.dir
for %f in (arachne code ertos fdsmtpop freexp leetirc lsicq lsppp lynx mime64) do echo %f > %f.zip
cd ..

cd sound.dir
for %f in (abcmidi drtrack id3tools lame midiplay mpxplay normaliz opencub) do echo %f > %f.zip
cd ..

cd util.dir
for %f in (4dos 7-zip arj bzip2 cabextr cal cdrcache c-lock cmos compinfo) do echo %f > %f.zip
cd ..

echo Done!
:done
