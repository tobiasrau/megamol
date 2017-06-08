push @INC, "3rd/rev2ver";
require "rev2ver.inc";

my $path = shift;
my $inFile = shift;
my $outFile = shift;

my %filter = (
    default => "ignore",
    consider => [ '^[^/]+$', '^include/.*$', '^src/.*$' ],
    #consider => [ '^src/.*$', '^[^/]+$', '^include/.*$' ],
    ignore => [ ]
);
my $vislib = getRevisionInfo($path, \%filter);
#my $vislib = getRevisionInfo($path);

my %hash;

$hash{'$VISLIB_VERSION$'} = $vislib->rev;
$hash{'$VISLIB_HAS_MODIFICATIONS$'} = $vislib->dirty;

processFile($outFile, $inFile, \%hash);