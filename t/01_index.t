use Test::Most;
use File::Temp qw(tmpnam);
use VectorSpaceModel;


my $index = InvertedIndex->new;
is_deeply $index->dump, {index => {}, lengths => {}}, 'initial index is empty';


while (<DATA>)
{   $index->add_document($., [split ' ']) }
$index->calculate_lengths;


is_deeply $index->dump, {
    index   => {
        coffee => {(1, 2),         (3, 1), (4, 3)        },
        cup    => {        (2, 1), (3, 2), (4, 3)        },
        jar    => {        (2, 2), (3, 1), (4, 3), (5, 2)},
        tea    => {        (2, 2),         (4, 1)        },
        water  => {                                (5, 2)},
    },
    lengths => {
        1      => 0.44,
        2      => 0.85,
        3      => 0.51,
        4      => 1.06,
        5      => 1.41,
    },
}, 'index is filled correctly';


my $path = tmpnam;
ok $index->stash($path), 'stashing the index to a file';

my $clone = InvertedIndex->new;
ok $clone->unstash($path), 'reconstituting index from stash';
is_deeply $clone->dump, $index->dump, 'unstashing gives the stashed index';


done_testing
__DATA__
coffee coffee
cup jar jar tea tea
coffee cup cup jar
tea coffee coffee coffee cup cup cup jar jar jar
water water jar jar
