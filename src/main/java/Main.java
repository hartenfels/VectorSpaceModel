public class Main {
    public static void main(String[] args) {
        InvertedIndex invertedIndex = new InvertedIndex();

        invertedIndex.addToken(1, "apple");
        invertedIndex.addToken(1, "apple");
        invertedIndex.addToken(1, "pear");
        invertedIndex.addToken(1, "pear");
        invertedIndex.addToken(1, "pear");

        invertedIndex.addToken(2, "banana");
        invertedIndex.addToken(2, "cherry");
        invertedIndex.addToken(2, "grape");
        invertedIndex.addToken(2, "mango");
        invertedIndex.addToken(2, "mango");
        invertedIndex.addToken(2, "mango");

        invertedIndex.addToken(3, "apple");
        invertedIndex.addToken(3, "cherry");
        invertedIndex.addToken(3, "cherry");
        invertedIndex.addToken(3, "mango");

        invertedIndex.addToken(4, "apple");
        invertedIndex.addToken(4, "apple");
        invertedIndex.addToken(4, "grape");

        invertedIndex.addToken(5, "apple");
        invertedIndex.addToken(5, "mango");
        invertedIndex.addToken(5, "mango");
        invertedIndex.addToken(5, "pear");
        invertedIndex.addToken(5, "pear");
        invertedIndex.addToken(5, "pear");

        System.out.println(invertedIndex.occurrences);
        System.out.println(invertedIndex.documentFrequency("apple"));
        System.out.println(invertedIndex.fetch(new String[]{"apple", "pear"}));
    }
}
