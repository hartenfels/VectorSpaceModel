use Test::Most;
use InvertedIndex;


my $index = InvertedIndex->new;

while (<DATA>)
{   $index->add_document($., [split ' ']) }
$index->calculate_lengths;


sub fetch_is
{
    my ($query, $want) = @_;
    my $got = $index->fetch([split ' ', $query]);
    is_deeply [map { $_->[0] } @$got], $want, "query for `$query`";
}

fetch_is 'mug'     => [];
fetch_is 'water'   => [5];
fetch_is 'coffee'  => [1, 4, 3];
fetch_is 'cup jar' => [3, 4, 2, 5];


done_testing
__DATA__
coffee coffee
cup jar jar tea tea
coffee cup cup jar
tea coffee coffee coffee cup cup cup jar jar jar
water water jar jar
