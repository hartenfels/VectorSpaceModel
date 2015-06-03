use Test::Most;
use List::Util qw(sum);
use InvertedIndex;


my $index = InvertedIndex->new;
is_deeply $index->dump, {
    index     => {},
    documents => {},
}, 'initial index is empty';


while (<DATA>)
{   $index->add_document($., [split ' ']) }


sub veclen { sqrt(sum map { $_ ** 2 } @_) }

is_deeply $index->dump, {
    index     => {
        coffee => [[1, 2],         [3, 1], [4, 3]        ],
        cup    => [        [2, 1], [3, 2], [4, 3]        ],
        jar    => [        [2, 2], [3, 1], [4, 3], [5, 2]],
        tea    => [        [2, 2],         [4, 1]        ],
        water  => [                                [5, 2]],
    },
    documents => {
        1      => veclen(2, 0, 0 ,0, 0),
        2      => veclen(0, 1, 2, 2, 0),
        3      => veclen(1, 2, 1, 0, 0),
        4      => veclen(3, 3, 3, 1, 0),
        5      => veclen(0, 0, 2, 0, 2),
    },
}, 'document lengths are filled correctly';


done_testing
__DATA__
coffee coffee
cup jar jar tea tea
coffee cup cup jar
tea coffee coffee coffee cup cup cup jar jar jar
water water jar jar
