#! /usr/bin/perl

$| = 1;

my %FILES;
opendir(DIR, ".");
while (defined($file=readdir(DIR))) {
    if (-f $file) {
	$FILES{$file} = 1;
    }
}
closedir(DIR);

delete($FILES{"hello.c"});
delete($FILES{"configure.in"});
delete($FILES{"Makefile.am"});
delete($FILES{"boot"});
delete($FILES{"reconf"});
delete($FILES{"acconfig.h"});

foreach $file (keys %FILES) {
    unlink($file);
}

sub sys {
    my $cmd = shift;
    print "*** $cmd\n";
    return system($cmd);
}

#sys("./reconf");
#sys("./configure");

