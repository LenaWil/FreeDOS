#!/bin/perl

use strict;

# base name for NLS and binary files, giving us e.g.: MEM.EXE, MEM.EN
my($progname) = 'MEM';

# the available languages
my(@langs) = qw(DE EN ES IT NL);

# path separator
my($pathsep) = '\\';

# path to directory containing translation files; relative to source directory
my($nlsdir) = '..' . $pathsep . '..' . $pathsep . 'NLS';

# output directories; under these we have a directory per language
my($dirprefix) = 'NLS';
my($basesrcdir) = $dirprefix . 'SRC';
my($baseobjdir) = $dirprefix . 'OBJ';
my($basebindir) = $dirprefix . 'BIN';

my($objsuffix) = '.OBJ';
my($binsuffix) = '.EXE';

# output filenames in the per-language directories
my($srcfilename) = 'MUSCHI.C';
my($srcbasename) = 'MUSCHI';
my($objfilename) = $progname . $objsuffix;
my($binfilename) = $progname . $binsuffix;

my($lang);
my($srcall) = '';
my($objall) = '';
my($binall) = '';
my($srcdir, $objdir, $bindir);
my($src, $muschiarg, $obj, $bin);
for $lang (@langs) {
    $srcdir = $basesrcdir . $pathsep . $lang;
    $objdir = $baseobjdir . $pathsep . $lang;
    $bindir = $basebindir . $pathsep . $lang;

    $src = $srcdir . $pathsep . $srcfilename;
    $muschiarg = $srcdir . $pathsep . $srcbasename;
    $obj = $objdir . $pathsep . $objfilename;
    $bin = $bindir . $pathsep . $binfilename;

    print(<<__END__);
$srcdir: $basesrcdir
    md $srcdir

$objdir: $baseobjdir
    md $objdir

$bindir: $basebindir
    md $bindir

#FIXME: -fe= is a Watcom-specific flag; -e is the TCC version
$bin: $bindir $obj prf.obj kitten.obj \$(MEMSUPT)
    \$(CC) \$(LFLAGS) $obj prf.obj kitten.obj \$(MEMSUPT) -fe=\$*${binsuffix}

#FIXME: -fo= is a Watcom-specific flag; -o is the TCC version
$obj: $objdir mem.c $src
# pass -I so we pick up muschi.c from the right directory
    \$(CC) \$(CFLAGS) -DMUSCHI -I$srcdir mem.c -fo=\$*${objsuffix}

$src: $nlsdir$pathsep${progname}.$lang $srcdir
    perl muschi.pl $muschiarg < $nlsdir$pathsep${progname}.$lang 

__END__

    $srcall .= ' ' . $src;
    $objall .= ' ' . $obj;
    $binall .= ' ' . $bin;
}

print(<<__END__);
${basesrcdir}ALL: $srcall
${baseobjdir}ALL: $objall
${basebindir}ALL: $binall

# make the base directories:
$basesrcdir:
    md $basesrcdir

$baseobjdir:
    md $baseobjdir

$basebindir:
    md $basebindir

__END__
