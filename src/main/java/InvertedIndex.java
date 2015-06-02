import java.io.*;
import java.util.*;

import static java.lang.String.format;


public class InvertedIndex implements Serializable {

    public Map<String, Map<Integer, Integer>> occurrences; // token -> (docID -> #occurrences)
    public Map<Integer, Integer> documents; // docID -> length of document


    public InvertedIndex() {
        occurrences = new HashMap<>();
        documents = new HashMap<>();
    }

    public void addToken(int id, String token) {
        occurrences.putIfAbsent(token, new HashMap<>());
        Map<Integer, Integer> a = occurrences.get(token);
        a.put(id, a.getOrDefault(id, 0) + 1);
        documents.put(id, documents.getOrDefault(id, 0) + 1);
    }

    public Result[] fetch(String[] query) {
        Map<Integer, Double> rankings = new HashMap<>();

        for (String token : query) {
            double globalWeight = inverseDocumentFrequency(token);
            double queryWeight = globalWeight;

            for (Map.Entry<Integer, Integer> e : occurrences.get(token).entrySet()) {
                rankings.put(e.getKey(), rankings.getOrDefault(e.getKey(), 0D) + globalWeight * queryWeight * e.getValue());
            }
        }

        List<Result> results = new ArrayList<Result>();
        for (Map.Entry<Integer, Double> e : rankings.entrySet()) {
            results.add(new Result(e.getKey(), e.getValue()));
        }

        Result[] sorted = results.toArray(new Result[0]);
        Arrays.sort(sorted);
        return sorted;
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
