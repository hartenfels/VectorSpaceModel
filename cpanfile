requires perl => '5.016';

requires 'Inline::Java';
requires 'Lingua::Stem::Snowball';
requires 'Term::ReadLine::Gnu';

on test => sub
{
    requires 'Test::Most';
};
