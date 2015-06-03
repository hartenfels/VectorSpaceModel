use Test::Most;
use InvertedIndex;


my $index = InvertedIndex->new;

while (<DATA>)
{   $index->add_document($., split ' ') }


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
__DATA__
coffee coffee
cup jar jar tea tea
coffee cup cup jar
tea coffee coffee coffee cup cup cup jar jar jar
water water jar jar
