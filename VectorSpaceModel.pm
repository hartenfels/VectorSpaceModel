use strict;
use warnings;
use feature qw(fc);
use Lingua::Stem::Snowball;
use Inline CPP => config => ccflags => '-std=c++11';
use Inline CPP => './VectorSpaceModel.cpp';


package InvertedIndex;

our $stemmer   = Lingua::Stem::Snowball->new(lang => 'en', encoding => 'UTF-8');
our %stopwords = map { $_ => undef } split ' ', <<HERE;
a able about across after all almost also am among an and any are as at be
because been but by can cannot could dear did do does either else ever every
for from get got had has have he her hers him his how however i if in into is
it its just least let like likely may me might most must my neither no nor not
of off often on only or other our own rather said say says she should since so
some than that the their them then there these they this tis to too twas us
wants was we were what when where which while who whom why will with would yet
you your . , ' " - : ;
HERE


sub index
{
    my ($self, $id, $document) = @_;
    my @words = grep { not exists $stopwords{$_} } split ' ', fc $document;
    $stemmer->stem_in_place(\@words);
    $self->add_document($id, \@words);
}


sub query
{
    my ($self, $query) = @_;
    my @words = split ' ', fc $query;
    $stemmer->stem_in_place(\@words);
    return (\@words, $self->fetch(\@words));
}


1
__END__

=head1 VectorSpaceModel

The best vector space model in the entire galaxy.

Interfaces with F<VectorSpaceModel.cpp>.

=head2 InvertedIndex

This extends the C++ C<InvertedIndex> class by a few methods so that we can
treat our documents as tokens of bytes instead of having to handle UTF-8
strings.

=head3 $stemmer

Our very own Snowball Porter stemmer.

=head3 %stopwords

A set of stop words. The keys of this hash are the list of stop words, the
values are unused. To check for a stopword, use
C<< exists $stopwords->{$word} >>.

The stop word list is taken from
L<http://www.textfixer.com/resources/common-english-words.txt>, which is linked
from the
L<Wikipedia article on stopwords|http://en.wikipedia.org/wiki/Stop_words>.

=head3 new

    $self = InvertedIndex->new

Create a new InvertedIndex. Then you must choose your adventure:

=over

=item

Either L</index> some documents, L</calculate_lengths> when you're done and
then maybe L</stash> what you have to a file, or

=item

L</unstash> a previously L</stash>ed index.

=back

Afterwards you can run a L</query> or two on it.

=head3 index

    $self->index($id, $document)

Index a document under the given ID. The ID has to be larger than the previous
ID, but it need not be sequential.

The given document will be case-folded, split by whitespace, stripped of stop
words, stemmed with the L</$stemmer> and finally placed inside the index.

=head3 calculate_lengths

    $self->calculate_lengths

Once you're done L</index>ing, call this to calculate the lengths for all
documents to enable length normalization.

=head3 query

    $self->query($query)

Run a query against the index.

=head3 stash

    $self->stash($path)

Store the index into a file at the given path. Returns wether that succeeded.

=head3 unstash

    $self = InvertedIndex->new
    $self->unstash($path)

Reconstitute a previously L</stash>ed index from the file at the given path.
Returns wether the unstashing succeeded. If it did not, the index is guaranteed
to still be empty and can still be filled manually.

=head3 dump

    $hashref = $self->dump

Dump this index into something Perl can understand. You shouldn't use this on
very large indexes or your memory will be eaten.

The dump will be something like:

    {
        # map of token to a map of document ID to token frequency
        index   => {
            coffee => {(1, 2),         (3, 1), (4, 3)        },
            cup    => {        (2, 1), (3, 2), (4, 3)        },
            jar    => {        (2, 2), (3, 1), (4, 3), (5, 2)},
            tea    => {        (2, 2),         (4, 1)        },
            water  => {                                (5, 2)},
        },
        # map of document ID to document vector length
        lengths => {
            1      => 0.44,
            2      => 0.85,
            3      => 0.51,
            4      => 1.06,
            5      => 1.41,
        },
    }

=cut
