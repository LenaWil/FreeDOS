#!perl

# Parses <IN> as EBNF HTML into HTML

$file_cmd = "CMDLIST";	# List of commands to be automatically linked

sub parseEBNF {
	my($ref_na) = @_;		# refernce name of this object

	while(<IN>) {
		chomp;
		my($own_line) = 1;

		if(/^CMD:/) {
			$_ = "\n<CMD>$'</CMD><BR>";
		} elsif(/^(SYNOPSIS):/ || /^(OPTIONS):/ || /^(EXAMPLES):/
		 || /^(ARGUMENTS):/) {
			$_ = &htmlSection($1);
		} elsif(/^STD_OPTIONS:/) {
			$_ = &htmlSection('Options') . $htmlHowToFormatOptions;
		} elsif(/^EXAMPLE:\s*/) {
			$_ = &htmlSubSection("Example: $'");
		} elsif(/^DESCRIPTION:/) {
			$_ = "<P>\n";
		}

		# Process the EBNF tag
		if(/^EBNF(\!)?:/) {
			my($line) = $';
			$own_line = !$1;

			$_ = '';
			my $delim = '';
			while($line =~ /^\s*('.*?'|<<.*?>>)/
			 || $line =~ /^\s*(\S+)/) {
				my($token) = $1;
				$line = $delim . $';
				$delim = '';
				my($tag) = '';

				if($token =~ /^[A-Z']/) {		# terminal characters
					$tag = 'CMD';
				} elsif($token =~ /^[a-z]/) {	# placeholder
					$tag = 'I';
				} elsif(index(',./[]{}():;', substr($token, 0, 1)) >= 0) {
					$tag = 'B';
				}
				if($token =~ /^<<(.*?)>>$/) {
					$token = '&laquo;' . &htmlXLat($1) . '&raquo;';
				} else {
					$token = &htmlXLat($token);
				}
				if($token) {
					$_ .= ' ';
					$_ .= "<$tag>" if $tag;
					$_ .= $token;
					$_ .= "</$tag>" if $tag;
					$delim = ' ';
				}
			}

			$_ .= "<BR>\n" if $own_line;
		} else {
			eval $cmdExpr;	# perform the linkaging of the commands
		}

		# Basic special tags
		s:<OPT>:<TT><B>:ig;
		s:</OPT>:</B></TT>:ig;
		s:<ARG>:<TT><I>:ig;
		s:</ARG>:</I></TT>:ig;
		s:<CMD>:<TT>:ig;
		s:</CMD>:</TT>:ig;
		s:<HELL>:<FONT COLOR="#CC0000">:ig;
		s:</HELL>:</FONT>:ig;
		s:<DIMM>:<FONT COLOR="#999999">:ig;
		s:</DIMM>:</FONT>:ig;

		# Modify references
		s:<A\s+NAME="([a-zA-Z].+?)"\s*>:<A NAME="${ref_na}_\1">:ig;
		my $line = '';
		while(m!<A\s+HREF=\"(.*?):(.*?)\"\s*>!i) {
			$line .= $`;
			my $post = $';
			my $all = $&;
			my $obj = lc($1);
			my $label = lc($2);
			if($obj =~ /^ftp|http|about$/) {
				# This is an absolute URL
				$line .= $all;
			} else {
				$label = "_" . $label if $label;
				$obj = $ref_na unless $obj;
				if($obj eq '!') {	## command appendix	
					$line .= &htmlAppendixRef($label);
				} else {
					$line .= "<A HREF=\"#$obj$label\">";
				}
			}
			$_ = $post;
		}
		$line .= $_;


		print HTML $line;
		print HTML "\n" if $own_line;
	}
}

2;

sub htmlXLat {
	my($line) = @_;

	$line =~ s/\&/\&amp;/g;
	$line =~ s/\</\&lt;/g;
	$line =~ s/\>/\&gt;/g;

	return $line;
}

sub readCmdList {
	if(open(CMDLIST, $file_cmd)) {
		# Read them
		$cmdExpr = '';
		while(<CMDLIST>) {
			chomp;
			$cmdlist{$_} = 1;
			$cmdExpr .=  ", s/\\b$_\\b/"
			 . quotemeta("<CMD>" . &htmlCommandRef($_) . "</CMD>")
			 . "/g";
		}
		close CMDLIST;
		$cmdExpr = substr($cmdExpr, 2);		# Kill the leading ", "
	}
}
