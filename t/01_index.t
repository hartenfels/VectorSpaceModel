use Test::Most;
use List::Util qw(sum);
use InvertedIndex;


my $index = InvertedIndex->new;
is_deeply $index->dump, {}, 'initial index is empty';


while (<DATA>)
{   $index->add_document($., [split ' ']) }


is_deeply $index->dump, {
    coffee => [[1, 2],         [3, 1], [4, 3]        ],
    cup    => [        [2, 1], [3, 2], [4, 3]        ],
    jar    => [        [2, 2], [3, 1], [4, 3], [5, 2]],
    tea    => [        [2, 2],         [4, 1]        ],
    water  => [                                [5, 2]],
}, 'index is filled correctly';


done_testing
__DATA__
coffee coffee
cup jar jar tea tea
coffee cup cup jar
tea coffee coffee coffee cup cup cup jar jar jar
water water jar jar
