use Test::Most;
use autodie;
use InvertedIndex;


ok my $index = InvertedIndex->new, 'constructing InvertedIndex';

open my $fh, '<', 't/cutlery.txt';
$index->index($., $_) while <$fh>;
close $fh;

my $cupjars = $index->fetch([qw(cup jar)]);
is_deeply [map { $_->{id} } @$cupjars], [4, 3, 2, 5], 'query for `cup jar`';

done_testing
