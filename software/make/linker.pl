#compiler command line invokation file maker
#!./perl
open(try, '>linker.inv') || (die "Can't open 'linker.inv'.");
foreach $i (@ARGV){
    print try "$i\n";
}
close try;

    

