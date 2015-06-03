use Test::Most;
use autodie;
use InvertedIndex;


ok my $index = InvertedIndex->new, 'constructing InvertedIndex';

is_deeply $index->dump_index, {}, 'initial index is empty';

while (<DATA>)
{   $index->add_token($., $_) for split ' ' }

my %expected = (
    coffee => [[1, 2], [3, 1], [4, 3]        ],
    cup    => [[2, 1], [3, 2], [4, 3]        ],
    jar    => [[2, 2], [3, 1], [4, 3], [5, 2]],
    tea    => [[2, 2], [4, 1]                ],
    water  => [[5, 2]                        ],
);
is_deeply $index->dump_index, \%expected, 'index is filled correctly';


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
