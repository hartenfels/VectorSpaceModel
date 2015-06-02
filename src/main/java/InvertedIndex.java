import java.io.*;
import java.util.*;

import static java.lang.String.format;


public class InvertedIndex implements Serializable {

    public Map<String, Map<Integer, Integer>> occurrences; // token -> (docID -> #occurrences)
    public Map<Integer, Double> documents; // docID -> length of document


    public InvertedIndex() {
        occurrences = new HashMap<>();
        documents = new HashMap<>();
    }

    public void addToken(int id, String token) {
        occurrences.putIfAbsent(token, new HashMap<>());
        Map<Integer, Integer> a = occurrences.get(token);
        a.put(id, a.getOrDefault(id, 0) + 1);
        documents.put(id, Math.sqrt(Math.pow(documents.getOrDefault(id, 0D), 2) + Math.pow(weightDfIdf(token, id), 2)));
    }

    public QueryResult[] fetch(String[] query) {
        Map<Integer, Double> rankings = new HashMap<>();

        for (String token : query) {
            if (occurrences.containsKey(token)) {
                double globalWeight = inverseDocumentFrequency(token);
                double queryWeight = globalWeight;

                for (Map.Entry<Integer, Integer> e : occurrences.get(token).entrySet()) {
                    double ranking = rankings.getOrDefault(e.getKey(), 0D) + globalWeight * queryWeight * e.getValue();
                    rankings.put(e.getKey(), ranking);
                }
            }
        }

        List<QueryResult> queryResults = new ArrayList<QueryResult>();
        for (Map.Entry<Integer, Double> e : rankings.entrySet()) {
            queryResults.add(new QueryResult(e.getKey(), e.getValue()));
        }

        QueryResult[] sorted = queryResults.toArray(new QueryResult[0]);
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
