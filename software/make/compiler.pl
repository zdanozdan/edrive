#compiler command line invokation file maker
#!./perl
open(try, '>compiler.inv') || (die "Can't open 'compiler.inv'.");
foreach $i (@ARGV){
    print try "$i\n";
}
close try;

    

