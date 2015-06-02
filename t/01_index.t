use Test::Most;
use autodie;
use InvertedIndex;


ok my $index = InvertedIndex->new, 'constructing InvertedIndex';

open my $fh, '<', 't/cutlery.txt';
$index->index($., $_) while <$fh>;
close $fh;


sub fetch_is
{
    my ($query, $want) = @_;
    my $got = $index->query($query);
    is_deeply [map { $_->{id} } @$got], $want, "query for `$query`";
}

fetch_is 'mug'     => [];
fetch_is 'water'   => [5];
fetch_is 'coffee'  => [4, 1, 3];
fetch_is 'cup jar' => [4, 3, 2, 5];


done_testing
