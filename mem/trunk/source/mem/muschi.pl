#!/bin/perl -w

use strict;

use FileHandle;

# Modify this if you want to generate different output.  For example, you may
# wish to remove the generation of the .h file, you may wish to put the
# strings into an array instead of separate globals, or perhaps generate the
# output for a different (programming) language.

# This defines the format strings used for generating the output into each
# file.  The 'string' format is used each time we read a string with its set
# and message numbers from the input file; $1 refers to the set number, $2
# refers to the message number, and $3 refers to the text of the string.  The
# 'comment' format is used when we read a comment (a line starting with a
# hash) from the input file; $1 refers to the text of the comment without the
# leading hash.  The 'blank' format is used for each blank line (consisting
# only of zero or more whitespace characters) that we read; there are no
# variables you can substitute here.

my($output_formats) = {'.h' => {'string' =>  'extern char *muschi_$1_$2;\n',
                'comment' => '/*$1 */\n',
                'blank' =>   '\n'},
               '.c' => {'string' =>  'char *muschi_$1_$2 = \"$3\";\n',
                'comment' => '/*               $1 */\n',
                'blank' =>   '\n'},
           };

my($basename) = pop(@ARGV);
if (!defined($basename) || $basename =~ /^(--?|\/)h(elp)?$/) {
    print(<<'__END__');
Usage: muschi.pl <basename>
where the output file extensions defined in $output_formats are appended to
the basename to generate the output filenames.  Supply an NLS file on
standard input.
__END__
    exit(1);
}

my(%output_files);

# Open up the output file(s)
my($extension);
foreach $extension (keys(%$output_formats)) {
    $output_files{$extension} = new FileHandle;
    my($filename) = $basename . $extension;
    $output_files{$extension}->open(">$filename")
        || die("Failed to open $filename for output");
}

my($format);
while (<>) {
    # Work out which format to use and set up the $1, $2, ...
    if (/^\s*#(.*)$/) {
    $format = 'comment';
    } elsif (/^(\d+)\.(\d+):(.*)$/) {
    $format = 'string';
    } elsif (/^\s*$/) {
    $format = 'blank';
    } else {
    die("Could not interpret input line: $_");
    }

    # Write the output to the output file(s)
    foreach $extension (keys(%$output_formats)) {
    eval('$output_files{$extension}->print("' . $output_formats->{$extension}->{$format} . '")');
    }
}

# Close the output file(s)
foreach $extension (keys(%$output_formats)) {
    $output_files{$extension}->close() || die("Error closing file");
}
