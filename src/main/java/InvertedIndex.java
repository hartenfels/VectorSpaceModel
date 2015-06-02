import java.io.*;
import java.util.HashMap;
import java.util.Map;

import static java.lang.String.format;


public class InvertedIndex implements Serializable {

    public Map<String, Map<Integer, Integer>> occurrences;
    public Map<Integer, Integer> documents; // docID -> length of document

    public InvertedIndex() {
        occurrences = new HashMap<>();
    }

    public void addToken(int id, String token) {
        occurrences.putIfAbsent(token, new HashMap<>());
        Map<Integer, Integer> a = occurrences.get(token);
        a.put(id, a.getOrDefault(id, 0) + 1);
    }

    public void addDocumentLength(int id, int length) {
        documents.put(id, length);
    }


    public void stash(String path) {
        try {
            FileOutputStream file = new FileOutputStream(path);
            ObjectOutputStream objs = new ObjectOutputStream(file);
            objs.writeObject(this);
            objs.close();
            file.close();
        } catch (IOException e) {
            System.err.println(format("Error stashing `%s`: %s", path, e.getMessage()));
        }
    }


    public static InvertedIndex unstash(String path) {
        InvertedIndex index = null;

        try {
            FileInputStream file = new FileInputStream(path);
            ObjectInputStream objs = new ObjectInputStream(file);
            index = (InvertedIndex) objs.readObject();
            objs.close();
            file.close();
        } catch (IOException | ClassNotFoundException e) {
            System.err.println(format("Error unstashing `%s`: %s", path, e.getMessage()));
        }

        return index;
    }

    public int documentFrequency(String token) {
        return occurrences.get(token).size();
    }

    public double inverseDocumentFrequency(String token) {
        return Math.log10(((double) documents.size()) / documentFrequency(token));
    }

    public int termFrequency(String token, int documentId) {
        return occurrences.get(token).get(documentId);
    }

    public double weightDfIdf(String token, int documentId) {
        return termFrequency(token, documentId) * inverseDocumentFrequency(token);
    }
}
