#compiler command line invokation file maker
#!./perl
open(try, '>library.inv') || (die "Can't open 'library.inv'.");
foreach $i (@ARGV){
    print try "$i ";
}
close try;

    

