#!/usr/bin/env perl
use strict;
use File::Temp 'tempfile';
use File::Basename;

use Test::Simple tests => 1;

my $dir = dirname $0;
$dir .= "/..";
my $bin = "$dir/bin";

my ($file) = @ARGV
    or die "no args";

ok `gosh $file` eq `$bin/lisp $file`, $file

